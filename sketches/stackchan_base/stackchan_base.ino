// デモの起点ファーム: 顔を出す・首を動かす・喋る
// 頭のタッチセンサーに触れると、うなずいて挨拶する
#include <Arduino.h>
#include <M5StackChan.h>
#include <Avatar.h>

#include "voice_hello.h"
#include "voice_welcome.h"

using namespace m5avatar;

Avatar avatar;

// 角度の単位は 10 = 1度。Yaw: -1280〜1280、Pitch: 50〜850（5〜85度）の範囲で使う
constexpr int kPitchCenter = 300;

// 音声再生中は口をパクパクさせる
void speak(const uint8_t* wav, size_t len, const char* text)
{
    avatar.setSpeechText(text);
    M5.Speaker.playWav(wav, len);
    while (M5.Speaker.isPlaying()) {
        avatar.setMouthOpenRatio((millis() / 120) % 2 ? 0.8f : 0.1f);
        delay(30);
    }
    avatar.setMouthOpenRatio(0);
    avatar.setSpeechText("");
}

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

// テンション高めの挨拶: 虹色に光りながら弾んで、早口で「ようこそ！！」
void greet()
{
    avatar.setExpression(Expression::Happy);
    uint32_t start = millis();

    // 弾むように2回うなずく
    for (int i = 0; i < 2; i++) {
        M5StackChan.Motion.movePitch(kPitchCenter + 250, 450);
        for (uint32_t t = millis() + 450; millis() < t; delay(20)) rainbowLeds((millis() - start) / 2);
        M5StackChan.Motion.movePitch(kPitchCenter - 50, 450);
        for (uint32_t t = millis() + 450; millis() < t; delay(20)) rainbowLeds((millis() - start) / 2);
    }
    M5StackChan.Motion.movePitch(kPitchCenter + 150, 400);

    // 虹色に光りながら、口パクで喋る
    avatar.setSpeechText("みなさん、DevelopersIOへようこそ！！");
    M5.Speaker.playWav(voice_welcome, voice_welcome_len);
    while (M5.Speaker.isPlaying()) {
        rainbowLeds((millis() - start) / 2);
        avatar.setMouthOpenRatio((millis() / 100) % 2 ? 0.9f : 0.1f);
        delay(30);
    }
    avatar.setMouthOpenRatio(0);

    // 喜んで首を左右に振る
    for (int i = 0; i < 2; i++) {
        M5StackChan.Motion.moveYaw(300, 450);
        for (uint32_t t = millis() + 600; millis() < t; delay(20)) rainbowLeds((millis() - start) / 2);
        M5StackChan.Motion.moveYaw(-300, 450);
        for (uint32_t t = millis() + 600; millis() < t; delay(20)) rainbowLeds((millis() - start) / 2);
    }
    M5StackChan.Motion.move(0, kPitchCenter, 400);
    delay(700);
    avatar.setSpeechText("");
    setLeds(0, 0, 0);
}

void setup()
{
    M5StackChan.begin();
    M5.Speaker.setVolume(200);

    avatar.setSpeechFont(&fonts::efontJA_16);
    avatar.init();

    M5StackChan.Motion.move(0, kPitchCenter, 400);
}

void loop()
{
    M5StackChan.update();
    if (M5StackChan.TouchSensor.wasPressed()) {
        greet();
    }
    delay(10);
}
