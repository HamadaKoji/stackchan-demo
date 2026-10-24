// 活用アイディアのデモ: GitHubのスターが増えるとStackChanが喜ぶ
// GitHub APIで stackchan-demo リポジトリのスター数を定期的に確認し、増えたら踊ってお礼を言う
//   頭をタップ   → 挨拶（stackchan_base と同じ）
//   頭をスワイプ → 今すぐスター数を確認する
#include <Arduino.h>
#include <M5StackChan.h>
#include <Avatar.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>

#include "secrets.h"  // WIFI_SSID / WIFI_PASSWORD / GITHUB_TOKEN（secrets.h.example をコピーして作る。コミットしない）
#include "voice_hello.h"
#include "voice_thanks.h"

using namespace m5avatar;

Avatar avatar;

constexpr const char* kRepoApi = "https://api.github.com/repos/HamadaKoji/stackchan-demo";

// 未認証のGitHub APIは1時間60回まで（IPアドレスごと）。トークンがあれば1時間5000回
#if !defined(GITHUB_TOKEN) && defined(GITHUB_PAT)
#define GITHUB_TOKEN GITHUB_PAT
#endif
#ifndef GITHUB_TOKEN
#define GITHUB_TOKEN ""
#endif
const uint32_t kPollMs = strlen(GITHUB_TOKEN) > 0 ? 15000 : 75000;

// 角度の単位は 10 = 1度。Yaw: -1280〜1280、Pitch: 50〜850（5〜85度）の範囲で使う
constexpr int kPitchCenter = 300;

int stars = -1;       // 最後に確認したスター数（-1 は未取得）
String etag;          // 変化がないときは 304 が返る
uint32_t lastPoll = 0;

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

void speak(const uint8_t* wav, size_t len, const char* text)
{
    avatar.setSpeechText(text);
    M5.Speaker.playWav(wav, len);
    while (M5.Speaker.isPlaying()) {
        avatar.setMouthOpenRatio((millis() / 120) % 2 ? 0.8f : 0.1f);
        delay(30);
    }
    avatar.setMouthOpenRatio(0);
}

void showStars()
{
    String s = "★ " + String(stars);
    avatar.setSpeechText(s.c_str());
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
    showStars();
}

// スターが増えた: 虹色に光って踊り、お礼を言う
void celebrate(int added)
{
    avatar.setExpression(Expression::Happy);
    String s = "★ " + String(stars) + "（+" + String(added) + "）";
    avatar.setSpeechText(s.c_str());
    uint32_t start = millis();
    for (int i = 0; i < 4; i++) {
        M5StackChan.Motion.move(i % 2 ? -300 : 300, kPitchCenter + 200, 700);
        uint32_t until = millis() + 450;
        while (millis() < until) {
            rainbowLeds((millis() - start) / 3);
            delay(20);
        }
    }
    M5StackChan.Motion.move(0, kPitchCenter, 500);
    speak(voice_thanks, voice_thanks_len, s.c_str());
    setLeds(0, 0, 0);
    showStars();
}

// GitHub APIでスター数を取る。取れなければ -1、変化なし（304）なら現在値を返す
int fetchStars()
{
    NetworkClientSecure client;
    client.setInsecure();  // 公開情報を読むだけなので証明書の検証は省略（デモ用）
    HTTPClient http;
    if (!http.begin(client, kRepoApi)) return -1;
    http.setUserAgent("stackchan-demo");  // GitHub APIはUser-Agentが必須
    http.addHeader("Accept", "application/vnd.github+json");
    if (strlen(GITHUB_TOKEN) > 0) http.addHeader("Authorization", String("Bearer ") + GITHUB_TOKEN);
    if (etag.length() > 0) http.addHeader("If-None-Match", etag);
    const char* keys[] = {"ETag", "X-RateLimit-Remaining"};
    http.collectHeaders(keys, 2);

    int code = http.GET();
    int result = -1;
    if (code == 200) {
        etag = http.header("ETag");
        String body = http.getString();
        int i = body.indexOf("\"stargazers_count\":");
        if (i >= 0) result = body.substring(i + 19).toInt();
    } else if (code == 304) {
        result = stars;
    }
    Serial.printf("GitHub API: %d stars=%d remaining=%s\n", code, result, http.header("X-RateLimit-Remaining").c_str());
    http.end();
    return result;
}

void poll()
{
    lastPoll = millis();
    int n = fetchStars();
    if (n < 0) return;
    if (stars >= 0 && n > stars) {
        int added = n - stars;
        stars = n;
        celebrate(added);
    } else {
        stars = n;
        showStars();
    }
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
    avatar.setSpeechText("スターを数えています…");
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
    if (WiFi.status() == WL_CONNECTED) poll();
}

void loop()
{
    M5StackChan.update();
    auto& ts = M5StackChan.TouchSensor;
    if (ts.wasSwiped()) {
        avatar.setSpeechText("確認中…");
        poll();
    } else if (ts.wasClicked()) {
        greet();
    }

    if (WiFi.status() == WL_CONNECTED && millis() - lastPoll > kPollMs) {
        poll();
    }
    delay(10);
}
