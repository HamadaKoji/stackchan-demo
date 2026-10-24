#!/bin/bash
# スケッチをビルドして StackChan に書き込む
# 使い方: tools/flash.sh <スケッチ名> [ポート]
#   例: tools/flash.sh stackchan_stars
#   スケッチ名: stackchan_base（起点）／ stackchan_web（curl版）／ stackchan_stars（スター版）
#   ポートを省略すると /dev/cu.usbmodem* を自動で探す
set -euo pipefail
cd "$(dirname "$0")/.."
SKETCH="${1:?スケッチ名を指定してください（例: tools/flash.sh stackchan_stars）}"
DIR="sketches/$SKETCH"
FQBN="m5stack:esp32:m5stack_cores3"
[ -d "$DIR" ] || { echo "スケッチが見つかりません: $DIR" >&2; exit 1; }
if [ "$SKETCH" != "stackchan_base" ] && [ -f "$DIR/secrets.h.example" ] && [ ! -f "$DIR/secrets.h" ]; then
  echo "$DIR/secrets.h がありません。secrets.h.example をコピーしてWi-Fiの情報を書いてください" >&2
  exit 1
fi
PORT="${2:-}"
if [ -z "$PORT" ]; then
  PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1 || true)
  [ -n "$PORT" ] || { echo "StackChanが見つかりません。ケーブル（データ通信対応）と電源を確認してください" >&2; exit 1; }
fi
echo "ビルド: $SKETCH → 書き込み先: $PORT"
arduino-cli compile --fqbn "$FQBN" "$DIR"
arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$DIR"
echo "書き込みました: $SKETCH"
