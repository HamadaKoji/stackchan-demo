# Claude向けガイド

このリポジトリはM5Stack StackChanのライブデモ用。登壇中に会場の前でVibe Codingする前提で振る舞うこと。

## デバイス

- M5Stack StackChan（K151）。コントローラはCoreS3（ESP32-S3、16MB Flash、8MB PSRAM）
- サーボ: フィードバックサーボ2軸。水平は360°回転、**垂直は5〜85°の範囲に収める**（範囲外は機構に負担）
- RGB LED 12個（2列）、1Wスピーカー、デュアルマイク、2.0インチ 320×240タッチ画面、3ゾーンのタッチパネル
- 公式ドキュメント: https://docs.m5stack.com/en/StackChan

## 進め方

- 変更したら、**ビルドと書き込みまで自分で実行**して終える（ユーザーに手作業を残さない）
- 書き込みに失敗したら、RSTボタン約2秒長押し（緑LED点灯）でダウンロードモードに入るようユーザーに伝える
- 会場で投影されるので、返答は短く。何を変えたかを1〜2文で言う
- 秘密情報（APIキー等）は `secrets.h` に置き、コミットしない
- **ライブデモで変更するのは `sketches/stackchan_base` だけ**。`stackchan_web`（curl版）・`stackchan_stars`（スター版）は活用パート用に完成済みなので触らない

## ビルド・書き込みコマンド

```bash
# ビルド（初回は約40秒、2回目以降は約10秒）
arduino-cli compile --fqbn m5stack:esp32:m5stack_cores3 sketches/<スケッチ名>

# 書き込み（約9秒）。ポートは arduino-cli board list で確認（通常 /dev/cu.usbmodem101）
arduino-cli upload -p /dev/cu.usbmodem101 --fqbn m5stack:esp32:m5stack_cores3 sketches/<スケッチ名>
```

- ビルドと書き込みを1回でやるなら `tools/flash.sh <スケッチ名>`（ポートは自動で探す）
- スケッチは `sketches/<スケッチ名>/<スケッチ名>.ino` の形で置く（Arduinoの規約でフォルダ名とファイル名を揃える）
- ライブラリ `M5StackChan` の公式サンプル: `~/Documents/Arduino/libraries/M5StackChan/examples/`（RGB_LED、Servo/BasicMovement・Dance・HomeCalibration、TouchSensor、IR、NFC、INA226）

## 起点ファームと音声

- デモの起点は `sketches/stackchan_base`（顔: M5Stack_Avatarライブラリ、首: `M5StackChan.Motion`、声: 埋め込みWAV）
- 首の角度は 10 = 1度。Yaw -1280〜1280、Pitch は 50〜850 に収める
- 表情は `Expression::Happy / Angry / Sad / Doubt / Sleepy / Neutral`
- セリフを変える・増やすときは、音声ヘッダを生成してから `#include` して `speak()` で再生する:

```bash
python3 tools/make_voice.py sketches/stackchan_base voice_xxx "セリフ"
```
