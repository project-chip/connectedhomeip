'''
A Python wrapper to sign the CC35xx Matter image 

The example's out/ directory, the Simplelink Wi-Fi Toolbox path and the
example's sysconfig generated files path are passed in the shell script
is executed
'''

import subprocess
import sys

#pass in path to the shell file
script_path = "./" +sys.argv[1] + "sign_cc35xx_img.sh"

print(script_path)

#pass in paths for the toolbox commands
script_args = sys.argv[2:]

# exec
ret = subprocess.run([script_path] + script_args)


