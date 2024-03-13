import subprocess
import traceback
import sys
import os
import json
import yaml

MARGIN = '  '

class Paths:
    DEFAULT_TEMP = 'temp'
    DEFAULT_SUPPORT = 'support'

    def __init__(self, base_dir) -> None:
        self.base_dir = os.path.normpath(base_dir)
        self.root_dir = "{}/..".format(base_dir)
        self.temp_dir = "{}/{}".format(base_dir, Paths.DEFAULT_TEMP)
        self.support_dir = "{}/{}".format(base_dir, Paths.DEFAULT_SUPPORT)

    def setTemp(self, temp):
        self.temp_dir = temp

    def base(self, path = None):
        return self.normalize(self.base_dir, path)

    def root(self, path = None):
        return self.normalize(self.root_dir, path)

    def support(self, path = None):
        return self.normalize(self.support_dir, path)

    def temp(self, path = None):
        return self.normalize(self.temp_dir, path)

    def normalize(self, base, path = None):
        if (path is None) or ('' == path) or ('.' == path):
            full = base
        else:
            full = "{}/{}".format(base, path)
        return os.path.abspath(os.path.normpath(full))


class File:

    def __init__(self, path) -> None:
        self.path = path

    def read(self):
        with open(self.path, 'r') as f:
            return f.read()

    def write(self, data):
        with open(self.path, 'w') as f:
            f.write(data)

class BinaryFile(File):

    def read(self):
        if self.path is None: return bytearray()
        with open(self.path, 'rb') as f:
            return bytearray(f.read())

    def write(self, x):
        if x is None:
            data = bytearray()
        elif isinstance(x, bytearray):
            data = x
        else:
            data = str(x).encode('utf-8')
        with open(self.path, 'wb') as f:
            f.write(data)

class JsonFile(File):

    def read(self):
        with open(self.path, 'r') as f:
            return json.loads(f.read())

    def write(self, data):
        with open(self.path, 'w') as f:
            json.dump(data, f, indent=4)

class YamlFile(File):

    def read(self):
        with open(self.path, 'r') as f:
            return yaml.safe_load(f)

    def write(self, data):
        with open(self.path, 'w') as f:
            yaml.dump(data, f)


def execute(args, output = False, check = True, env = None):
    args = [ str(x) for x in args ]
    cmd = ' '.join(args)
    print("{}> {}\n".format(MARGIN, cmd))
    if env is None:
        env = os.environ.copy()

    if output:
        try:
            return subprocess.check_output(cmd, shell=True)
        except BaseException as err:
            if check:
                fail(err)
            return None
    else:
        complete = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT, shell=True, env=env)
        if check and (0 != complete.returncode):
            fail("Command failed with code {}".format(complete.returncode))
        return complete.returncode


def fail(message, paths = None):
    if paths is not None:
        print()
        prefix = paths.base() + '/'
        trace = traceback.extract_stack()
        for t in trace[:-1]:
            filename = t.filename.removeprefix(prefix)
            location = "{}:{}".format(filename, t.lineno)
            print("{}{:32}\t{}".format(MARGIN, location, t.line))
        print()
    print("(!) {}\n".format(message))
    exit(1)


def warn(message):
    print("(!) {}\n".format(message))

def roundNearest(n, multiple):
    if n % multiple:
        n = n + (multiple - n % multiple)
    return n
