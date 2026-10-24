# stackchan-demo

DevelopersIO 2026 Tokyo ミニセッション「AIをリアル世界に解き放つ　〜M5Stack×AIであなたのアイディアを手の平に〜」（2026-10-24）のライブデモ用リポジトリ。

M5Stack StackChanを、Claude CodeでVibe Codingして光らせ・喋らせ・動かす。C++は1行も手で書かず、AIに頼むだけで「依頼 → ビルド → 書き込み → 動く」を回す。

## 必要なもの

- **M5Stack StackChan（K151）**: CoreS3（ESP32-S3）、フィードバックサーボ2軸、RGB LED 12個、スピーカー、デュアルマイク、2.0インチ画面
  - 公式ドキュメント: https://docs.m5stack.com/en/StackChan
- **Mac**（音声の生成に macOS の `say` と `afconvert` を使う）
- **データ通信できるUSB-Cケーブル**（充電専用のケーブルだと認識されない）
- **Claude Code**（Vibe Codingする場合）
- 活用デモを試す場合は **2.4GHzのWi-Fi**

> 自作ファームを書き込むと、StackChanの工場ファーム（AIエージェントなど）は上書きされる。戻すときはM5Burnerで公式のStackChanファームを書き戻す。

## クイックスタート

### 1. ビルド環境を入れる

```bash
brew install arduino-cli
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json
arduino-cli core update-index && arduino-cli core install m5stack:esp32
arduino-cli lib install M5StackChan M5Stack_Avatar
```

動作を確認したバージョン: arduino-cli 1.5.1 ／ ボードパッケージ `m5stack:esp32` 3.3.10 ／ M5StackChan 1.1.1 ／ M5Stack_Avatar 0.10.0

### 2. StackChanにデモの起点ファームを書き込む

StackChanをUSBでつないで（土台側のUSB-C）:

```bash
tools/flash.sh stackchan_base
```

頭をタップすると、うなずいて「こんにちは、スタックチャンです。よろしくね」と挨拶する。

- ポートは自動で探す（`/dev/cu.usbmodem*`）。指定するなら `tools/flash.sh stackchan_base /dev/cu.usbmodem1401`
- 書き込めないときは、RSTボタンを2〜3秒長押し（緑のLEDが点灯）してダウンロードモードに入る

### 3. Claude Codeに頼んでみる

このリポジトリでClaude Codeを起動して、日本語で頼む。

```
挨拶のセリフを「DevelopersIOへようこそ！」に変えて、表情はずっと笑顔にして
```

```
頭を撫でたら、首を傾げて困った顔で「うーん…」と考えてから、笑顔で答えるようにして。LEDも考え中は青、答えるときは虹色に
```

Claude Codeは `CLAUDE.md`（デバイスの仕様・ビルドと書き込みのコマンド・セリフの作り方）を読んで、音声の生成からビルド・書き込みまで自分で実行する。

## スケッチ

`sketches/` に4つある。ブランチは切り替えず、書き込むスケッチを選ぶだけ。

| スケッチ | 中身 | 書き込み |
|---|---|---|
| `stackchan_base` | デモの起点。頭をタップすると挨拶する。ライブデモではこれを変更する | `tools/flash.sh stackchan_base` |
| `stackchan_web` | 活用デモ: curlで動かす（Wi-Fi） | `tools/flash.sh stackchan_web` |
| `stackchan_stars` | 活用デモ: GitHubのスターで喜ぶ（Wi-Fi） | `tools/flash.sh stackchan_stars` |
| `RGB_LED` | M5Stack公式のLEDサンプル（環境の動作確認用） | `tools/flash.sh RGB_LED` |

### 活用デモ: curlで動かす（`stackchan_web`）

Wi-Fiにつないで小さなHTTPサーバーを立てる。Webhookや監視アラートの「物理的な出口」のイメージ。

1. `sketches/stackchan_web/secrets.h.example` を `secrets.h` にコピーして、Wi-Fi（2.4GHz）のSSIDとパスワードを書く
2. `tools/flash.sh stackchan_web` で書き込む
3. 起動するとWi-Fiにつなぎ、IPアドレスを吹き出しに表示する

```bash
curl -4 http://stackchan.local/alert                # 首を振って赤く光り「アラートだよ！」
curl -4 http://stackchan.local/ok                   # 虹色に光って「解決したよ」
curl -4 "http://stackchan.local/say?text=こんにちは"  # 吹き出しに好きな文字を表示
```

- Macの `curl` は `stackchan.local` に対してIPv6を先に試して待たされることがあるので、`-4` を付ける
- `stackchan.local` で届かないネットワークでは、吹き出しに出たIPアドレスを使う
- 会場Wi-Fiなどで端末同士が通信できない場合は、スマホのテザリングにMacとStackChanの両方をつなぐ

### 活用デモ: GitHubのスターで喜ぶ（`stackchan_stars`）

GitHub APIでこのリポジトリのスター数を定期的に確認し、増えたら虹色に光って踊り「スターありがとう！」と喋る。

1. `sketches/stackchan_stars/secrets.h.example` を `secrets.h` にコピーして、Wi-Fi（2.4GHz）の情報を書く
   - 任意で `GITHUB_TOKEN`（`GITHUB_PAT` でも可）を入れる。権限なしのfine-grained tokenで十分
   - 未認証だとGitHub APIは1時間60回までなので75秒ごと、トークンありなら15秒ごとに確認する
2. `tools/flash.sh stackchan_stars` で書き込む
3. 起動するとスター数を吹き出しに表示する。頭をスワイプすると今すぐ確認する

- GitHub APIの応答は最大60秒ほどキャッシュされるので、スターを付けてから反応までは数十秒〜1分強かかる
- 証明書の検証は省略している（公開情報を読むだけのデモのため）

## ツール

| ツール | 使い方 | 中身 |
|---|---|---|
| `tools/flash.sh` | `tools/flash.sh <スケッチ名> [ポート]` | ビルドして書き込む。ポートは自動で探す |
| `tools/make_voice.py` | `python3 tools/make_voice.py <スケッチフォルダ> <変数名> "<セリフ>" [--voice Kyoko] [--rate 230] [--pitch 70]` | macOSの `say` でセリフを音声にし、スケッチに埋め込むヘッダ（`voice_xxx.h`）を作る。`--rate` で早口、`--pitch` で高めの声に |
| `tools/reset_demo.sh` | `tools/reset_demo.sh [ポート]` | リハーサル用。`main` に切り替えて起点（タグ `demo-start`）に戻し、起点ファームを書き込み直す。**作業ツリーの変更はすべて破棄される** |

ビルドと書き込みのコマンドを直接使う場合:

```bash
arduino-cli compile --fqbn m5stack:esp32:m5stack_cores3 sketches/<スケッチ名>
arduino-cli upload -p /dev/cu.usbmodem1401 --fqbn m5stack:esp32:m5stack_cores3 sketches/<スケッチ名>
```

ビルドは初回約40秒、2回目以降は約10秒。書き込みは約9秒。

起点ファームを更新したら、タグを打ち直す（`git tag -f demo-start`）。

## しくみのメモ

- **顔**: M5Stack_Avatarライブラリ（表情は Happy／Angry／Sad／Doubt／Sleepy／Neutral の6種類）
- **首・LED・タッチ**: M5Stack公式の M5StackChan ライブラリ。首の角度は 10 ＝ 1度で、上下は5〜85度の範囲で使う
- **声**: クラウドもAPIキーも使わない。ビルドの前にMacの音声合成でWAVを作り、C++の配列としてファームに埋め込む（2〜3秒のセリフで約100KB）

## リポジトリの構成

```
stackchan-demo/
├── CLAUDE.md          … Claude Codeへの取扱説明書（デバイスの仕様・コマンド・セリフの作り方）
├── sketches/
│   ├── stackchan_base/   … デモの起点
│   ├── stackchan_web/    … 活用デモ（curl）
│   ├── stackchan_stars/  … 活用デモ（GitHubのスター）
│   └── RGB_LED/          … 公式サンプル
└── tools/
    ├── flash.sh
    ├── make_voice.py
    └── reset_demo.sh
```

- 秘密情報（Wi-Fiのパスワード、トークン）は各スケッチの `secrets.h` に置く。`.gitignore` 済みでコミットされない
- `trial/` で始まるブランチは、リハーサルで実際にClaude Codeに頼んで作った結果の記録

## ライセンス

[MIT License](LICENSE)

- `sketches/RGB_LED/` はM5Stack公式のサンプル（M5StackChanライブラリ同梱、MIT License）をそのまま使っている
- 依存ライブラリ（M5StackChan、M5Stack_Avatar など）はそれぞれのライセンスに従う
