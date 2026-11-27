"""Usage: ~/.platformio/penv/bin/python scripts/screenshot.py PORT OUT.png (debug build)."""

import struct
import sys
import time
import zlib

import serial

BEGIN = "=====SCREEN BEGIN"
END = "=====SCREEN END====="


def write_png(path: str, w: int, h: int, rows: list) -> None:
    def chunk(kind: bytes, data: bytes) -> bytes:
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body))

    # 1-bit grayscale PNG treats 1 as white, the dump uses 1 as black.
    raw = b"".join(b"\x00" + bytes(b ^ 0xFF for b in row) for row in rows)
    header = struct.pack(">IIBBBBB", w, h, 1, 0, 0, 0, 0)
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", header))
        f.write(chunk(b"IDAT", zlib.compress(raw, 9)))
        f.write(chunk(b"IEND", b""))


def main() -> int:
    port, out_path = sys.argv[1], sys.argv[2]
    with serial.Serial(port, 115200, timeout=1) as link:
        link.reset_input_buffer()
        link.write(b"S\n")
        deadline = time.time() + 60
        rows, size = [], None
        while time.time() < deadline:
            line = link.readline().decode(errors="replace").strip()
            if not line:
                continue
            if line.startswith(BEGIN):
                w, h = (int(v) for v in line[len(BEGIN):].strip("= ").split())
                size, rows = (w, h), []
            elif line == END and size:
                break
            elif size is not None:
                rows.append(bytes.fromhex(line))
            else:
                print(line)
        else:
            print("timeout waiting for screen dump", file=sys.stderr)
            return 1

    w, h = size
    if len(rows) != h:
        print(f"expected {h} rows, got {len(rows)}", file=sys.stderr)
        return 1
    write_png(out_path, w, h, rows)
    print(out_path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
