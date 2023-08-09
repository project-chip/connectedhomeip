#! /usr/bin/python3
import sys
import argparse
from modules.util import *

GSDK_DIR = 'third_party/silabs/gecko_sdk'

def make(base_dir, part):
    print("▪︎ Make")
    part_dir = "{}/support/{}".format(base_dir, part)
    image_src = "{}/build/debug/generator.s37".format(part_dir)
    image_dest = "{}/images/{}.s37".format(base_dir, part)
    print("  ∙ dir:  {}".format(part_dir))
    print("  ∙ from:  {}".format(image_src))
    print("  ∙ to:  {}".format(image_dest))
    execute([ 'make', '-C', part_dir, '-f', 'generator.Makefile' ])
    execute([ 'cp', image_src, image_dest ])

def main(argv):
    base_dir = os.path.normpath(os.path.dirname(__file__))

    # Parse arguments
    parser = argparse.ArgumentParser(description='Provisioner Support')
    parser.add_argument('parts')
    args = parser.parse_args()

    # Build image(s)
    for part in args.parts.split(','):
        part_name = part.lower()
        print("\n◆ {}".format(part))
        make(base_dir, part_name)


main(sys.argv[1:])