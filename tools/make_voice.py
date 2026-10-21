#!/usr/bin/env python3
"""macOSの say でセリフを音声化し、スケッチに埋め込むヘッダを生成する。

使い方:
    python3 tools/make_voice.py <スケッチフォルダ> <変数名> "<セリフ>" [--voice Kyoko]

例:
    python3 tools/make_voice.py sketches/stackchan_base voice_hello "こんにちは、スタックチャンです"
    → sketches/stackchan_base/voice_hello.h（const uint8_t voice_hello[] を定義）
"""
import argparse
import pathlib
import subprocess
import tempfile


def main():
    p = argparse.ArgumentParser()
    p.add_argument("sketch_dir")
    p.add_argument("name")
    p.add_argument("text")
    p.add_argument("--voice", default="Kyoko")
    a = p.parse_args()

    out = pathlib.Path(a.sketch_dir) / f"{a.name}.h"
    with tempfile.TemporaryDirectory() as tmp:
        aiff = pathlib.Path(tmp) / "v.aiff"
        wav = pathlib.Path(tmp) / "v.wav"
        subprocess.run(["say", "-v", a.voice, "-o", str(aiff), a.text], check=True)
        # 16kHz・16bit・モノラルのWAVに変換（M5.Speaker.playWavで再生できる形式）
        subprocess.run(["afconvert", "-f", "WAVE", "-d", "LEI16@16000", "-c", "1", str(aiff), str(wav)], check=True)
        data = wav.read_bytes()

    lines = [", ".join(f"0x{b:02x}" for b in data[i:i + 16]) for i in range(0, len(data), 16)]
    body = ",\n  ".join(lines)
    out.write_text(
        f"// 自動生成: tools/make_voice.py（セリフ: {a.text}）\n"
        "#pragma once\n#include <Arduino.h>\n\n"
        f"const uint8_t {a.name}[] = {{\n  {body}\n}};\n"
        f"const size_t {a.name}_len = sizeof({a.name});\n",
        encoding="utf-8",
    )
    print(f"{out} ({len(data)} bytes)")


if __name__ == "__main__":
    main()
