# Copyright 2020 Texas Instruments Incorporated

"""A wrapper to run SysConfig.

This script provides a basic Python wrapper for GN integration.

Run with:
    python run_sysconfig.py <path to SysConfig> <SysConfig args>

This script will attempt to run sh or cmd to run the wrapper scripts for the
SysConfig cmd line.
"""

import sys
import subprocess

# Select OS specific helper script
if sys.platform.startswith('linux'):
    proc_call = [sys.argv[1] + '/sysconfig_cli.sh']

elif sys.platform.startswith('win'):
    proc_call = [sys.argv[1] + '/sysconfig_cli.bat']

else:
    # guess sh?
    proc_call = [sys.argv[1] + '/sysconfig_cli.sh']

# Append cmd options
proc_call += sys.argv[2:]

# exec
subprocess.call(proc_call)
