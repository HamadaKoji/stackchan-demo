// デモの起点ファーム: 顔を出す・首を動かす・喋る
// 頭のタッチセンサーに触れると、うなずいて挨拶する
#include <Arduino.h>
#include <M5StackChan.h>
#include <Avatar.h>

#include "voice_welcome.h"
#include "voice_hmm.h"
#include "voice_answer.h"
#include "voice_n1.h"
#include "voice_n2.h"
#include "voice_n3.h"
#include "voice_n4.h"
#include "voice_n5.h"
#include "voice_n6.h"
#include "voice_n7.h"
#include "voice_n8.h"
#include "voice_n9.h"
#include "voice_n10.h"
#include "voice_aho3.h"
#include "voice_aho6.h"
#include "voice_aho9.h"

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

    speak(voice_welcome, voice_welcome_len, "DevelopersIOへようこそ！");

    // 左右を見回す
    M5StackChan.Motion.moveYaw(300, 400);
    delay(700);
    M5StackChan.Motion.moveYaw(-300, 400);
    delay(900);
    M5StackChan.Motion.moveYaw(0, 400);
    delay(500);

}

void setLeds(uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < 12; i++) M5StackChan.setRgbColor(i, r, g, b);
    M5StackChan.refreshRgb();
}

// HSVの色相を回して虹色にする
void rainbowLeds(uint8_t shift)
{
    for (int i = 0; i < 12; i++) {
        float h = fmodf((i * 30 + shift) , 360.0f) / 60.0f;
        int k = (int)h;
        float f = h - k;
        uint8_t v = 168, q = v * (1 - f), t = v * f;
        uint8_t r[] = {v, q, 0, 0, t, v}, g[] = {t, v, v, q, 0, 0}, b[] = {0, 0, t, v, v, q};
        M5StackChan.setRgbColor(i, r[k % 6], g[k % 6], b[k % 6]);
    }
    M5StackChan.refreshRgb();
}

// 頭を撫でたとき: 首を傾げて考え、笑顔で答える
void pat()
{
    avatar.setExpression(Expression::Doubt);
    setLeds(0, 0, 168);
    M5StackChan.Motion.moveYaw(0, 300);
    M5StackChan.Motion.movePitch(kPitchCenter + 120, 300);
    speak(voice_hmm, voice_hmm_len, "うーん…");
    delay(500);

    avatar.setExpression(Expression::Happy);
    M5StackChan.Motion.movePitch(kPitchCenter, 400);
    uint32_t start = millis();
    M5.Speaker.playWav(voice_answer, voice_answer_len);
    avatar.setSpeechText("わかった！今日は最高の一日になるよ");
    while (M5.Speaker.isPlaying()) {
        rainbowLeds((millis() - start) / 3);
        avatar.setMouthOpenRatio((millis() / 120) % 2 ? 0.8f : 0.1f);
        delay(30);
    }
    avatar.setMouthOpenRatio(0);
    avatar.setSpeechText("");
    setLeds(0, 0, 0);
}

struct Voice {
    const uint8_t* wav;
    size_t len;
    const char* text;
};

// 1〜10。3の倍数はアホ用のセリフ
const Voice kCount[10] = {
    {voice_n1, voice_n1_len, "1"},
    {voice_n2, voice_n2_len, "2"},
    {voice_aho3, voice_aho3_len, "さ〜ん〜！あひゃひゃ"},
    {voice_n4, voice_n4_len, "4"},
    {voice_n5, voice_n5_len, "5"},
    {voice_aho6, voice_aho6_len, "ろ〜く〜！うひょ〜"},
    {voice_n7, voice_n7_len, "7"},
    {voice_n8, voice_n8_len, "8"},
    {voice_aho9, voice_aho9_len, "きゅ〜う〜！でへへ"},
    {voice_n10, voice_n10_len, "10"},
};

// 1から10まで数える。3の倍数ではアホになる
void countUp()
{
    for (int i = 1; i <= 10; i++) {
        const Voice& v = kCount[i - 1];
        if (i % 3 == 0) {
            avatar.setExpression(Expression::Doubt);
            setLeds(168, 0, 168);
            M5StackChan.Motion.movePitch(kPitchCenter + 150, 300);
            M5StackChan.Motion.moveYaw(i % 2 ? 250 : -250, 300);
            speak(v.wav, v.len, v.text);
            M5StackChan.Motion.moveYaw(-(i % 2 ? 250 : -250), 200);
            delay(300);
        } else {
            avatar.setExpression(Expression::Happy);
            setLeds(0, 168, 0);
            M5StackChan.Motion.move(0, kPitchCenter, 300);
            speak(v.wav, v.len, v.text);
            delay(200);
        }
    }
    M5StackChan.Motion.move(0, kPitchCenter, 400);
    avatar.setExpression(Expression::Happy);
    setLeds(0, 0, 0);
}

void setup()
{
    M5StackChan.begin();
    M5.Speaker.setVolume(200);

    avatar.setSpeechFont(&fonts::efontJA_16);
    avatar.init();
    avatar.setExpression(Expression::Happy);

    M5StackChan.Motion.move(0, kPitchCenter, 400);
}

void loop()
{
    M5StackChan.update();
    auto& ts = M5StackChan.TouchSensor;
    if (ts.wasSwiped()) {
        countUp();
    } else if (ts.wasClicked()) {
        greet();
    }
    delay(10);
}
