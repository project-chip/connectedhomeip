# Copyright 2020 Texas Instruments Incorporated

"""A wrapper to run the oad tool.

This script provides a basic Python wrapper for GN to call the TI OAD image
tool. This will result in a downloadable binary file. Currently this is only
used for CC13XX_CC26XX devices.

Run with:
    python run_oad_tool.py <path to TI SimpleLinkSdk> <root out dir> <exe base name> <pem file> <bim hexfile>
"""

import os
import subprocess
import sys

import intelhex

# The extension is probably not necessary
if sys.platform.startswith('win'):
    proc_call = [sys.argv[1] + '/tools/common/oad/oad_image_tool.exe']

else:
    proc_call = [sys.argv[1] + '/tools/common/oad/oad_image_tool']

root_out_dir = sys.argv[2]
project_name = sys.argv[3]
pem_file = sys.argv[4]
hex_file = root_out_dir + '/' + project_name + '.hex'
oad_bin_file = root_out_dir + '/' + project_name + '.bin'
combined_hex = root_out_dir + '/' + project_name + '-bim.hex'
bim_hex_file = sys.argv[5]

proc_call += [
    '--verbose',
    'ccs',
    root_out_dir,
    '7',
    '-hex1', os.path.relpath(hex_file, root_out_dir),
    '-k', pem_file,
    '-o', sys.argv[3]
]

# run oad_tool to fill in the header
subprocess.check_call(proc_call)

# merge binary executable with bim hex file
ota_image = intelhex.IntelHex()
ota_image.fromfile(oad_bin_file, format='bin')

bim_hex = intelhex.IntelHex()
bim_hex.fromfile(bim_hex_file, format='hex')

ota_image.merge(bim_hex)

ota_image.tofile(combined_hex, format='hex')
