// 活用アイディアのデモ: HTTPで呼ぶとStackChanが反応する
//   curl http://stackchan.local/alert        → 首を振って赤く光り「アラートだよ！」
//   curl http://stackchan.local/ok           → 虹色に光って「解決したよ」
//   curl "http://stackchan.local/say?text=…" → 吹き出しに好きな文字を出す
// 頭をタップすると挨拶する（stackchan_base と同じ）
#include <Arduino.h>
#include <M5StackChan.h>
#include <Avatar.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "secrets.h"  // WIFI_SSID / WIFI_PASSWORD（secrets.h.example をコピーして作る。コミットしない）
#include "voice_hello.h"
#include "voice_alert.h"
#include "voice_ok.h"

using namespace m5avatar;

Avatar avatar;
WebServer server(80);

// 角度の単位は 10 = 1度。Yaw: -1280〜1280、Pitch: 50〜850（5〜85度）の範囲で使う
constexpr int kPitchCenter = 300;

// HTTPで受けた動作は、レスポンスを返したあとに loop() で実行する
enum class Action { None, Alert, Ok, Say };
Action pending = Action::None;
String sayText;

void setLeds(uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < 12; i++) M5StackChan.setRgbColor(i, r, g, b);
    M5StackChan.refreshRgb();
}

// HSVの色相を回して虹色にする
void rainbowLeds(uint16_t shift)
{
    for (int i = 0; i < 12; i++) {
        float h = fmodf(i * 30 + shift, 360.0f) / 60.0f;
        int k = (int)h;
        float f = h - k;
        uint8_t v = 168, q = v * (1 - f), t = v * f;
        uint8_t r[] = {v, q, 0, 0, t, v}, g[] = {t, v, v, q, 0, 0}, b[] = {0, 0, t, v, v, q};
        M5StackChan.setRgbColor(i, r[k % 6], g[k % 6], b[k % 6]);
    }
    M5StackChan.refreshRgb();
}

// 音声再生中は口をパクパクさせる。ledFn があれば再生中に呼び続ける
void speak(const uint8_t* wav, size_t len, const char* text, void (*ledFn)(uint32_t) = nullptr)
{
    avatar.setSpeechText(text);
    uint32_t start = millis();
    M5.Speaker.playWav(wav, len);
    while (M5.Speaker.isPlaying()) {
        if (ledFn) ledFn(millis() - start);
        avatar.setMouthOpenRatio((millis() / 120) % 2 ? 0.8f : 0.1f);
        delay(30);
    }
    avatar.setMouthOpenRatio(0);
    avatar.setSpeechText("");
}

void greet()
{
    avatar.setExpression(Expression::Happy);
    M5StackChan.Motion.movePitch(kPitchCenter + 200, 600);
    delay(400);
    M5StackChan.Motion.movePitch(kPitchCenter - 100, 600);
    delay(400);
    M5StackChan.Motion.movePitch(kPitchCenter, 600);
    speak(voice_hello, voice_hello_len, "こんにちは！");
}

// アラート: 赤く点滅しながら首を左右に振る
void alert()
{
    avatar.setExpression(Expression::Angry);
    for (int i = 0; i < 3; i++) {
        setLeds(200, 0, 0);
        M5StackChan.Motion.moveYaw(350, 800);
        delay(300);
        setLeds(0, 0, 0);
        M5StackChan.Motion.moveYaw(-350, 800);
        delay(300);
    }
    M5StackChan.Motion.moveYaw(0, 600);
    setLeds(200, 0, 0);
    speak(voice_alert, voice_alert_len, "アラートだよ！確認して！");
    setLeds(0, 0, 0);
    avatar.setExpression(Expression::Neutral);
}

// 解決: 虹色に光って喜ぶ
void ok()
{
    avatar.setExpression(Expression::Happy);
    M5StackChan.Motion.movePitch(kPitchCenter + 200, 500);
    speak(voice_ok, voice_ok_len, "解決したよ！おつかれさま！", [](uint32_t t) { rainbowLeds(t / 3); });
    M5StackChan.Motion.movePitch(kPitchCenter, 500);
    setLeds(0, 0, 0);
}

// 好きな文字を吹き出しに出す（声は事前生成なので、ここは文字だけ）
void say(const String& text)
{
    avatar.setExpression(Expression::Happy);
    M5StackChan.Motion.moveYaw(150, 400);
    delay(300);
    M5StackChan.Motion.moveYaw(0, 400);
    avatar.setSpeechText(text.c_str());
    delay(5000);
    avatar.setSpeechText("");
}

void setupServer()
{
    server.on("/", [] {
        server.send(200, "text/plain; charset=utf-8",
                    "StackChan is here!\n  /alert  アラート\n  /ok     解決\n  /say?text=...  吹き出しに表示\n");
    });
    server.on("/alert", [] {
        pending = Action::Alert;
        server.send(200, "text/plain; charset=utf-8", "alert: OK\n");
    });
    server.on("/ok", [] {
        pending = Action::Ok;
        server.send(200, "text/plain; charset=utf-8", "ok: OK\n");
    });
    server.on("/say", [] {
        sayText = server.arg("text");
        if (sayText.isEmpty()) {
            server.send(400, "text/plain; charset=utf-8", "text を指定してください\n");
            return;
        }
        pending = Action::Say;
        server.send(200, "text/plain; charset=utf-8", "say: " + sayText + "\n");
    });
    server.begin();
}

void connectWifi()
{
    avatar.setSpeechText("Wi-Fiにつないでいます…");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(250);
    }
    if (WiFi.status() != WL_CONNECTED) {
        avatar.setExpression(Expression::Sad);
        avatar.setSpeechText("Wi-Fiにつながりません");
        return;
    }
    MDNS.begin("stackchan");  // http://stackchan.local でアクセスできる
    setupServer();
    // mDNSが使えないネットワーク向けに、IPアドレスを吹き出しで知らせる
    String ip = WiFi.localIP().toString();
    avatar.setSpeechText(ip.c_str());
    Serial.printf("StackChan: http://stackchan.local/ (%s)\n", ip.c_str());
    delay(4000);
    avatar.setSpeechText("");
}

void setup()
{
    M5StackChan.begin();
    M5.Speaker.setVolume(255);  // 最大（0〜255）

    avatar.setSpeechFont(&fonts::efontJA_16);
    avatar.init();
    avatar.setExpression(Expression::Happy);
    M5StackChan.Motion.move(0, kPitchCenter, 400);

    connectWifi();
}

void loop()
{
    M5StackChan.update();
    if (WiFi.status() == WL_CONNECTED) {
        server.handleClient();
    }

    Action a = pending;
    pending = Action::None;
    switch (a) {
        case Action::Alert: alert(); break;
        case Action::Ok: ok(); break;
        case Action::Say: say(sayText); break;
        default: break;
    }

    if (M5StackChan.TouchSensor.wasClicked()) {
        greet();
    }
    delay(10);
}
