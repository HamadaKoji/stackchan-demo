#!/bin/bash
# リハーサル用: デモの起点（タグ demo-start）に戻して、起点ファームを書き込み直す
# 注意: 作業ツリーの変更はすべて破棄される
set -euo pipefail
cd "$(dirname "$0")/.."
PORT="${1:-}"  # 省略すると flash.sh が自動で探す
# 作業中のブランチ（trial/...）を巻き戻さないよう、必ず main に切り替えてから戻す
git switch -f main
git reset --hard demo-start
git clean -fd sketches
tools/flash.sh stackchan_base $PORT
echo "起点に戻しました"
