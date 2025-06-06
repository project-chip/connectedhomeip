import sys
from pathlib import Path

TARGET_DIR = Path("src/python_testing/")


def is_exception_line(line):
    return ("except ChipStackError" in line) and ("# chipstack-ok" not in line)


failures = []

for py_file in TARGET_DIR.rglob("*.py"):
    with open(py_file, encoding="utf-8") as f:
        lines = f.readlines()
        for i, line in enumerate(lines):
            if is_exception_line(line):
                failures.append(f"{py_file}: {i+1}: 'except ChipStackError' without # chipstack-ok -> {line.strip()}")

if failures:
    print("\n".join(failures))
    sys.exit(1)
else:
    print("No 'except ChipStackError' found in testing Python files")
