import os
from .util import *

class DeviceInfo:
    def __init__(self, text):
        lines = text.decode('utf-8').splitlines()
        d = dict(map(str.strip, x.split(':')) for x in lines[0:len(lines)-1])
        self.part = d['Part Number']
        self.uid = d['Unique ID']
        self.revision = d['Die Revision']
        self.version = d['Production Ver']

    def __str__(self):
        return "  ∙ part: '{}'\n  ∙ UID: {}\n  ∙ version: {}\n  ∙ revision: {}".format(self.part, self.uid, self.version, self.revision)


class Commander:

    def __init__(self, jtag_serial = None):
        self.jtag = jtag_serial

    def execute(self, args, serial = False, output = True, check = False):
        args.insert(0, 'commander')
        if serial and (self.jtag is not None):
            args.extend(["--serialno", self.jtag])
        return execute(args, output, check)

    def info(self):
        output = self.execute(['device', 'info'], True)
        return DeviceInfo(output)

    def flash(self, image_path):
        return self.execute(['flash' , image_path], True)