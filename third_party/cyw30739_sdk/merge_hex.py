#!/usr/bin/env python

import argparse
import pathlib
import sys

from intelhex import IntelHex


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=pathlib.Path)
    parser.add_argument("--ds_hex", required=True, type=pathlib.Path)
    parser.add_argument("--xs_hex", required=True, type=pathlib.Path)

    option = parser.parse_args()

    ds_hex = IntelHex(str(option.ds_hex))
    xs_hex = IntelHex(str(option.xs_hex))
    ds_hex.merge(xs_hex)
    ds_hex.write_hex_file(option.output)


if __name__ == "__main__":
    sys.exit(main())
