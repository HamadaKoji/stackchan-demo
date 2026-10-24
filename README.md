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

## スケッチの切り替え

`sketches/` に3つのスケッチがある。ブランチは切り替えず、書き込むスケッチを選ぶだけ。

| スケッチ | 中身 |
|---|---|
| `stackchan_base` | デモの起点（頭をタップすると挨拶）。ライブデモではこれを変更する |
| `stackchan_web` | 活用デモ: curlで動かす（Wi-Fi） |
| `stackchan_stars` | 活用デモ: GitHubのスターで喜ぶ（Wi-Fi） |

```bash
tools/flash.sh stackchan_web     # ビルドして書き込む（ポートは自動で探す。指定するなら第2引数）
```

## リハーサル

試走のたびに起点へ戻す（作業ツリーの変更は破棄される）:

```bash
tools/reset_demo.sh
```

起点はタグ `demo-start`。起点ファームを更新したらタグを打ち直す（`git tag -f demo-start`）。

## ライセンス

[MIT License](LICENSE)

- `sketches/RGB_LED/` はM5Stack公式のサンプル（M5StackChanライブラリ同梱、MIT License）をそのまま使っている
- 依存ライブラリ（M5StackChan、M5Stack_Avatar など）はそれぞれのライセンスに従う

## 活用デモ: HTTPでStackChanを動かす（`sketches/stackchan_web`）

Wi-Fiにつないで小さなHTTPサーバーを立てる。Webhookや監視アラートの「物理的な出口」のイメージ。

1. `sketches/stackchan_web/secrets.h.example` を `secrets.h` にコピーして、Wi-Fi（2.4GHz）のSSIDとパスワードを書く
2. ビルドして書き込む（コマンドは `CLAUDE.md` と同じで、スケッチ名を `stackchan_web` にする）
3. 起動するとWi-Fiにつなぎ、IPアドレスを吹き出しに表示する

```bash
curl http://stackchan.local/alert                # 首を振って赤く光り「アラートだよ！」
curl http://stackchan.local/ok                   # 虹色に光って「解決したよ」
curl "http://stackchan.local/say?text=こんにちは"  # 吹き出しに好きな文字を表示
```

- Macの `curl` で `stackchan.local` が応答しないときは `-4` を付ける（IPv6を先に試して待たされるため）: `curl -4 http://stackchan.local/alert`
- `stackchan.local` で届かないネットワークでは、吹き出しに出たIPアドレスを使う
- 会場Wi-Fiなどで端末同士が通信できない場合は、スマホのテザリングにMacとStackChanの両方をつなぐ

## 活用デモ: GitHubのスターで喜ぶ（`sketches/stackchan_stars`）

GitHub APIでこのリポジトリのスター数を定期的に確認し、増えたら虹色に光って踊り「スターありがとう！」と喋る。

1. `sketches/stackchan_stars/secrets.h.example` を `secrets.h` にコピーして、Wi-Fi（2.4GHz）の情報を書く
   - 任意で `GITHUB_TOKEN`（`GITHUB_PAT` でも可） を入れる（権限なしのfine-grained tokenで十分）。未認証だと1時間60回までなので75秒ごと、トークンありなら15秒ごとに確認する
2. ビルドして書き込む（スケッチ名を `stackchan_stars` にする）
3. 起動するとスター数を吹き出しに表示する。頭をスワイプすると今すぐ確認する

- GitHub APIの応答は最大60秒ほどキャッシュされるので、スターを付けてから反応までは数十秒〜1分強かかる
- 証明書の検証は省略している（公開情報を読むだけのデモのため）
