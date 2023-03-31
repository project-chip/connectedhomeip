import subprocess
import datetime
import sys
import os

def execute(args, output = False, check = True):
    args = [ str(x) for x in args ]
    cmd = ' '.join(args)
    print("  ‣ {}".format(cmd))

    if output:
        try:
            return subprocess.check_output(cmd, shell=True)
        except BaseException as err:
            fail("Command failed", err)
    else:
        complete = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT, shell=True)
        if check and (0 != complete.returncode):
            fail("Command failed with code {}".format(complete.returncode), complete.returncode)
        return complete.returncode


def fail(message, code = 1):
    print("⛔️ {}".format(message))
    exit(code)


def generateSerial():
    base_time = datetime.datetime(2000, 1, 1)
    delta = datetime.datetime.now() - base_time
    return delta.seconds
