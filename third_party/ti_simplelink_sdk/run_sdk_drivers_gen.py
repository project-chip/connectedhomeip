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
parser.add_argument('--src-path-drivers', help="the path where the built drivers exist")
parser.add_argument('--src-path-driverlib', help="the path where the built driverlib exist")
parser.add_argument('--dest-path', help="path where drivers will be copied to")
parser.add_argument('--src-path-hsm', help="path where the HSM Library will be copied to")
parser.add_argument('--secure-src-file', help="path to the secure source file")
parser.add_argument('--secure-dest-path', help="destination path for the secure source file")

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

source_file_drivers = args.sdk + args.src_path_drivers
source_file_driverlib = args.sdk + args.src_path_driverlib
dest_path = args.dest_path

if args.secure_src_file is not None:
    secure_src_file = args.sdk + args.secure_src_file
else:
    secure_src_file = None

if args.src_path_hsm is not None:
    source_file_hsm = args.sdk + args.src_path_hsm
else:
    source_file_hsm = None

secure_dest_path = args.secure_dest_path

make_command = ["make", "-C", args.sdk, "CMAKE=cmake", "GCC_ARMCOMPILER=" +
                GCC_ARMCOMPILER_PATH, "IAR_ARMCOMPILER=", "TICLANG_ARMCOMPILER=", "GENERATOR=Ninja"]


pid = os.fork()
if pid:
    status = os.wait()
    if os.path.exists(source_file_drivers):
        shutil.copy(source_file_drivers, dest_path)
    else:
        print("Driver does not exist or path is incorrect.")
        sys.exit(2)

    if os.path.exists(source_file_driverlib):
        shutil.copy(source_file_driverlib, dest_path)
    else:
        print("Driverlib does not exist or path is incorrect.")
        sys.exit(2)


    if source_file_hsm is not None:
        if os.path.exists(source_file_hsm):
            shutil.copy(source_file_hsm, dest_path)
        else:
            print("HSM Library does not exist or path is incorrect.")
            sys.exit(2)

    if secure_src_file is not None and secure_dest_path is not None:
        # Copy the secure source file to the destination path
        if os.path.exists(secure_src_file):
            shutil.copy(secure_src_file, secure_dest_path)
        else:
            print("Secure source file does not exist or path is incorrect.")
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
