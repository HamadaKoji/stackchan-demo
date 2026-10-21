# stackchan-demo

DevelopersIO 2026 Tokyo ミニセッション「AIをリアル世界に解き放つ　〜M5Stack×AIであなたのアイディアを手の平に〜」（2026-10-24）のライブデモ用リポジトリ。

M5Stack StackChanを、Claude CodeでVibe Codingして光らせ・喋らせ・動かす。

## ハードウェア

- M5Stack StackChan（K151）: CoreS3（ESP32-S3）、フィードバックサーボ2軸、RGB LED 12個、スピーカー、デュアルマイク、2.0インチ画面
- 公式ドキュメント: https://docs.m5stack.com/en/StackChan

## ビルド環境

- arduino-cli 1.5.1（Homebrew）
- ボードパッケージ: `m5stack:esp32` 3.3.10（追加URL: https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json）
- ボード: M5CoreS3（FQBN `m5stack:esp32:m5stack_cores3`）
- ライブラリ: M5StackChan 1.1.1（依存: M5Unified、IRremoteESP8266、M5Unit-NFC）

```bash
brew install arduino-cli
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json
arduino-cli core update-index && arduino-cli core install m5stack:esp32
arduino-cli lib install M5StackChan
```

ビルド・書き込みコマンドは `CLAUDE.md` を参照。

## 注意

- 自作ファームを書き込むと工場ファームは上書きされる。戻すときはM5Burnerで公式StackChanファームを書き戻す
- APIキー等の秘密情報はコミットしない（`secrets.h` / `.env` はgitignore済み）

## リハーサル

試走のたびに起点へ戻す（作業ツリーの変更は破棄される）:

```bash
tools/reset_demo.sh
```

起点はタグ `demo-start`。起点ファームを更新したらタグを打ち直す（`git tag -f demo-start`）。
