#!/usr/bin/env python3
"""Verify the partition reservation and 8 MB header in actual Arduino artifacts."""
import struct
from pathlib import Path
import sys

build = Path(sys.argv[1])
name = sys.argv[2]
partition_file = build / (name + ".ino.partitions.bin")
image_file = build / (name + ".ino.bin")
raw = partition_file.read_bytes()
layout = {}
for offset in range(0, len(raw), 32):
    magic = struct.unpack_from("<H", raw, offset)[0]
    if magic in (0xFFFF, 0xEBEB):
        break
    if magic != 0x50AA:
        raise RuntimeError("Invalid partition entry magic")
    _, kind, subtype, start, size, label, _ = struct.unpack_from("<HBBII16sI", raw, offset)
    label = label.split(b"\x00", 1)[0].decode("ascii")
    if label in layout:
        raise RuntimeError("Duplicate partition label")
    layout[label] = (kind, subtype, start, size)
if layout.get("cardid") != (1, 2, 0x356000, 0x4000):
    raise RuntimeError("Built partition table does not reserve cardid at the expected address")
if layout.get("factory") != (0, 0, 0x10000, 0x300000):
    raise RuntimeError("Built application partition is not the expected 3 MB factory partition")
spans = sorted((start, start + size) for _, _, start, size in layout.values())
if any(a[1] > b[0] for a, b in zip(spans, spans[1:])):
    raise RuntimeError("Overlapping built partitions")
image = image_file.read_bytes()
if len(image) < 24 or image[0] != 0xE9 or (image[3] >> 4) != 3:
    raise RuntimeError("Built ESP image does not declare 8 MB flash")
if struct.unpack_from("<H", image, 12)[0] != 5:
    raise RuntimeError("Built image is not for ESP32-C3")
if len(image) > layout["factory"][3]:
    raise RuntimeError("Built image exceeds the factory partition")
print(f"PASS: {name} built ESP32-C3/8 MB image and protected partition table")
