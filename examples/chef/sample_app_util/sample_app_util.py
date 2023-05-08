#!/usr/bin/env python3

"""Utility to work with sample app device files.

This utility helps with the following:
- Parsing sample app device files.
- Producing metadata files from sample app device files.
- Generating names for sample app device files.

Usage:
  python sample_app_util.py zap <ZAP_FILE> [options]

  python sample_app_util.py zap test_files/sample_zap_file.zap --generate-name
  python sample_app_util.py zap test_files/sample_zap_file.zap --generate-metadata
"""

import argparse
import os
import shutil

import zap_file_parser


def zap_cmd_handler(args: argparse.Namespace) -> None:
    """Handles args for zap_cmd_parser."""
    zap_file_path = args.zap_file
    if args.generate_name:
        print(zap_file_parser.generate_name(zap_file_path))
    elif args.rename_file:
        name = zap_file_parser.generate_name(zap_file_path)
        dirpath = os.path.dirname(zap_file_path)
        output_path = os.path.join(dirpath, f"{name}.zap")
        shutil.move(zap_file_path, output_path)
        print(f"Renamed from: {zap_file_path} to {output_path}")
    elif args.generate_metadata:
        created_file = zap_file_parser.generate_metadata_file(zap_file_path)
        print(f"Created {created_file}")


parser = argparse.ArgumentParser()
subparsers = parser.add_subparsers(dest="command")
subparsers.required = True

zap_cmd_parser = subparsers.add_parser(
    "zap", help="Command to operate on zap files.")
zap_cmd_parser.add_argument(
    "zap_file", metavar="ZAP_FILE", type=str, help="Zap file to generate name for.")

zap_cmd_group = zap_cmd_parser.add_mutually_exclusive_group()

zap_cmd_group.add_argument(
    "--generate-name", action="store_true",
    help="Print the name file name according to the name convention"
)

zap_cmd_parser.add_argument(
    "--generate-metadata", action="store_true",
    help=(
        "Generate the metadata file which provides summary information about the app.")
)

zap_cmd_group.add_argument(
    "--rename-file", action='store_true',
    help="Rename the target zap file according to name convention."
)

zap_cmd_parser.set_defaults(func=zap_cmd_handler)


if __name__ == "__main__":
    args = parser.parse_args()
    args.func(args)
