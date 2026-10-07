#!/usr/bin/env python3
"""Check button timing, conversion boundaries, and protected example layouts."""
import csv
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="passport-host-") as tmp:
    binary = str(Path(tmp) / "test_logic")
    subprocess.run([os.environ.get("CXX", "c++"), "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    "-I" + str(root / "src"), str(root / "tests/test_logic.cpp"), "-o", binary], check=True)
    subprocess.run([binary], check=True)
for sketch in sorted((root / "examples").iterdir()):
    assert (sketch / (sketch.name + ".ino")).is_file(), sketch
    partition = sketch / "partitions.csv"
    assert partition.read_bytes() == (root / "extras/partitions.csv").read_bytes(), partition
    lines = [line for line in partition.read_text().splitlines() if line.strip() and not line.startswith("#")]
    rows = list(csv.reader(lines))
    layout = {row[0]: (int(row[3], 0), int(row[4], 0)) for row in rows}
    assert layout["cardid"] == (0x356000, 0x4000), partition
    assert layout["factory"] == (0x10000, 0x300000), partition
    spans = sorted((start, start + size, name) for name, (start, size) in layout.items())
    for a, b in zip(spans, spans[1:]):
        assert a[1] <= b[0], (partition, a, b)
    assert spans[-1][1] <= 8 * 1024 * 1024, partition
print("PASS: button boundaries, debounce, long press, timer rollover, battery conversion, and example partition layouts")
