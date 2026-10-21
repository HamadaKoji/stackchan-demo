# stackchan-demo

DevelopersIO 2026 Tokyo ミニセッション「AIをリアル世界に解き放つ　〜M5Stack×AIであなたのアイディアを手の平に〜」（2026-10-24）のライブデモ用リポジトリ。

M5Stack StackChanを、Claude CodeでVibe Codingして光らせ・喋らせ・動かす。

## ハードウェア

- M5Stack StackChan（K151）: CoreS3（ESP32-S3）、フィードバックサーボ2軸、RGB LED 12個、スピーカー、デュアルマイク、2.0インチ画面
- 公式ドキュメント: https://docs.m5stack.com/en/StackChan

## ビルド環境

（Step 2で確定する。公式手順はArduino IDE: ボード M5CoreS3、ライブラリ M5StackChan）

## 注意

- 自作ファームを書き込むと工場ファームは上書きされる。戻すときはM5Burnerで公式StackChanファームを書き戻す
- APIキー等の秘密情報はコミットしない（`secrets.h` / `.env` はgitignore済み）
