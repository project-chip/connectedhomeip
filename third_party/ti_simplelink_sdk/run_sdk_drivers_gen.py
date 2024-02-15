# Copyright 2020 Texas Instruments Incorporated

"""A wrapper to run the SDK makefiles for Platform driver generation
Args:
1. [TI Simplelink SDK Root]
2. [Matter repository Root]
3. [Path to built drivers in repo]
"""

import argparse
import os
import shutil
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument('--sdk', help="TI SDK root")
parser.add_argument('--chip-root', help="CHIP Root")
parser.add_argument('--src-path', help="the path where the built drivers exist")
parser.add_argument('--dest-path', help="path where drivers will be copied to")

args = parser.parse_args()

ret = False

if os.getenv('_PW_ACTUAL_ENVIRONMENT_ROOT'):
    CHIP_ENV_ROOT = os.getenv('_PW_ACTUAL_ENVIRONMENT_ROOT')
else:
    CHIP_ENV_ROOT = os.path.join(args.chip_root, ".environment")

GCC_ARMCOMPILER_PATH = os.path.join(CHIP_ENV_ROOT, "cipd", "packages", "arm")

if not os.path.isdir(GCC_ARMCOMPILER_PATH):
    print("Compiler Path is invalid: " + GCC_ARMCOMPILER_PATH)
    sys.exit(2)

source_file = args.sdk + args.src_path
dest_path = args.dest_path

make_command = ["make", "-C", args.sdk, "CMAKE=cmake", "GCC_ARMCOMPILER=" +
                GCC_ARMCOMPILER_PATH, "IAR_ARMCOMPILER=", "TICLANG_ARMCOMPILER=", "GENERATOR=Ninja"]

pid = os.fork()
if pid:
    status = os.wait()
    if os.path.exists(source_file):
        shutil.copy(source_file, dest_path)
    else:
        print("Driver does not exist or path is incorrect.")
        sys.exit(2)

else:
    make_command = ["make", "-C", args.sdk, "CMAKE=cmake", "GCC_ARMCOMPILER=" +
                    GCC_ARMCOMPILER_PATH, "IAR_ARMCOMPILER=", "TICLANG_ARMCOMPILER=", "GENERATOR=Ninja"]
    res = subprocess.run(make_command, capture_output=True, encoding="utf8")
    if res.returncode != 0:
        print("!!!!!!!!!!!! EXEC FAILED !!!!!!!!!!!!!!!!")
        print("!!!!!!!!!!!!!!! STDOUT !!!!!!!!!!!!!!!!!!")
        print("%s" % res.stdout)
        print("!!!!!!!!!!!!!!! STDERR !!!!!!!!!!!!!!!!!!")
        print("%s" % res.stderr)
        print("res.returncode: %d", res.returncode)
        sys.exit(1)
