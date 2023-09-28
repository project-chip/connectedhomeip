# Copyright 2020 Texas Instruments Incorporated

"""A wrapper to run the SDK makefiles for Platform driver generation
Args:
1. [TI Simplelink SDK Root]
2. [Matter repository Root]
"""

import subprocess
import sys

SDK_ROOT=sys.argv[1]
CHIP_ROOT=sys.argv[2]

GCC_ARMCOMPILER_PATH= CHIP_ROOT + "/.environment/cipd/packages/arm/"
make_command = ["make", "-C" ,SDK_ROOT,"CMAKE=cmake","GCC_ARMCOMPILER=" + GCC_ARMCOMPILER_PATH,"IAR_ARMCOMPILER=","TICLANG_ARMCOMPILER=","GENERATOR=Ninja"]

ret = subprocess.call(make_command)
sys.exit(ret)
