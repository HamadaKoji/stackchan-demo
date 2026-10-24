#!/bin/bash
# リハーサル用: デモの起点（タグ demo-start）に戻して、起点ファームを書き込み直す
# 注意: 作業ツリーの変更はすべて破棄される
set -euo pipefail
cd "$(dirname "$0")/.."
PORT="${1:-/dev/cu.usbmodem101}"
# 作業中のブランチ（trial/...）を巻き戻さないよう、必ず main に切り替えてから戻す
git switch -f main
git reset --hard demo-start
git clean -fd sketches
arduino-cli compile --fqbn m5stack:esp32:m5stack_cores3 sketches/stackchan_base
arduino-cli upload -p "$PORT" --fqbn m5stack:esp32:m5stack_cores3 sketches/stackchan_base
echo "起点に戻しました"
