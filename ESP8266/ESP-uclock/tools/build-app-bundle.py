#!/usr/bin/env python3
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
APP_DIR = ROOT / "data" / "app"
OUT_FILE = ROOT / "data" / "app-bundle.txt"


def iter_files():
    for path in sorted(APP_DIR.rglob("*")):
        if path.is_file():
            yield path


def main():
    lines = ["WCAPPBUNDLE 1"]

    for path in iter_files():
        rel = path.relative_to(ROOT / "data").as_posix()
        lines.append(f"FILE {rel}")
        content = path.read_text(encoding="utf-8")
        lines.extend(content.splitlines())
        lines.append("END_FILE")

    lines.append("END_BUNDLE")
    OUT_FILE.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {OUT_FILE}")


if __name__ == "__main__":
    main()
