# Copyright 2020 Texas Instruments Incorporated

"""A wrapper to run the SDK makefiles for Platform driver generation
Args:
1. [TI Simplelink SDK Root]
2. [Matter repository Root]
"""

import os
import subprocess
import sys

SDK_ROOT = sys.argv[1]
CHIP_ENV_ROOT = sys.argv[2]
ret = False

if os.getenv('_PW_ACTUAL_ENVIRONMENT_ROOT'):
    CHIP_ENV_ROOT = os.getenv('_PW_ACTUAL_ENVIRONMENT_ROOT')

GCC_ARMCOMPILER_PATH = CHIP_ENV_ROOT + "/cipd/packages/arm/"

if os.path.isdir(GCC_ARMCOMPILER_PATH):
    make_command = ["make", "-C", SDK_ROOT, "CMAKE=cmake", "GCC_ARMCOMPILER=" +
                    GCC_ARMCOMPILER_PATH, "IAR_ARMCOMPILER=", "TICLANG_ARMCOMPILER=", "GENERATOR=Ninja"]

    ret = subprocess.call(make_command)
else:
    print("Compiler Path is invalid: " + GCC_ARMCOMPILER_PATH)

sys.exit(ret)
