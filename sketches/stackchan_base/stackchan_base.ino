// デモの起点ファーム: 顔を出す・首を動かす・喋る
// 頭のタッチセンサーに触れると、うなずいて挨拶する
#include <Arduino.h>
#include <M5StackChan.h>
#include <Avatar.h>

#include "voice_hello.h"

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

void greet()
{
    avatar.setExpression(Expression::Happy);

    // うなずく
    M5StackChan.Motion.movePitch(kPitchCenter + 200, 600);
    delay(400);
    M5StackChan.Motion.movePitch(kPitchCenter - 100, 600);
    delay(400);
    M5StackChan.Motion.movePitch(kPitchCenter, 600);

    speak(voice_hello, voice_hello_len, "こんにちは！");

    // 左右を見回す
    M5StackChan.Motion.moveYaw(300, 400);
    delay(700);
    M5StackChan.Motion.moveYaw(-300, 400);
    delay(900);
    M5StackChan.Motion.moveYaw(0, 400);
    delay(500);

    avatar.setExpression(Expression::Neutral);
}

void setup()
{
    M5StackChan.begin();
    M5.Speaker.setVolume(255);  // 最大（0〜255）

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
