#!/usr/bin/env python

import argparse
import shutil
import sys


def main():

    parser = argparse.ArgumentParser(description="Copy a tree of files from a source directory to a destination one.")

    parser.add_argument('--src-dir', required=True)
    parser.add_argument('--dest-dir', required=True)

    args = parser.parse_args()

    shutil.copytree(args.src_dir, args.dest_dir, dirs_exist_ok=True)


if __name__ == '__main__':
    sys.exit(main())
