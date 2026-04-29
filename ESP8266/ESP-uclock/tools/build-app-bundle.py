#!/usr/bin/env python3
import base64
import gzip
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
APP_DIR = ROOT / "data" / "app"
OUT_FILE = ROOT / "data" / "app-bundle.txt"

GZIP_EXTENSIONS = {".html", ".css", ".js", ".json", ".webmanifest", ".svg"}


def iter_files():
    for path in sorted(APP_DIR.rglob("*")):
        if path.is_file():
            yield path


def main():
    lines = ["WCAPPBUNDLE 1"]

    for path in iter_files():
        rel = path.relative_to(ROOT / "data").as_posix()
        raw = path.read_bytes()

        if path.suffix.lower() in GZIP_EXTENSIONS:
            compressed = gzip.compress(raw, compresslevel=9)
            lines.append(f"FILE_GZ {rel}")
            lines.append(base64.b64encode(compressed).decode("ascii"))
        else:
            lines.append(f"FILE {rel}")
            lines.extend(raw.decode("utf-8").splitlines())

        lines.append("END_FILE")

    lines.append("END_BUNDLE")
    OUT_FILE.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {OUT_FILE}")


if __name__ == "__main__":
    main()
