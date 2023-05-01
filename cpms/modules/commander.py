import os
from .util import *


# SoC configuration
socs = {
  'mg12': { 'ram_addr': 0x20000000, 'stacksize': 0x1000, 'flash_addr': 0x0000000 },
  'mg24': { 'ram_addr': 0x20000000, 'stacksize': 0x1000, 'flash_addr': 0x8000000 },
}


class DeviceInfo:
    def __init__(self, text):
        lines = text.decode('utf-8').splitlines()
        d = dict(map(str.strip, x.split(':')) for x in lines[0:len(lines)-1])
        self.part = d['Part Number'].lower()
        self.uid = d['Unique ID']
        self.revision = d['Die Revision']
        self.version = d['Production Ver']
        self.flash_size = self.parseSize(d['Flash Size'])
        self.family = self.part[0:9].lower()

        # Only MG12 and MG24 are supported in matter currently
        if "efr32mg12" in self.part:
            soc = socs['mg12']
        elif "efr32mg24" in self.part:
            soc = socs['mg24']
        else:
            raise Exception('Invalid MCU')
        self.ram_addr = soc['ram_addr']
        self.stacksize = soc['stacksize']
        self.flash_addr = soc['flash_addr']


    def parseSize(self, text):
        parts = text.split()
        value = int(parts[0])
        multiplier = 1
        if 'kb' == parts[1].lower():
            multiplier = 1024
        return value * multiplier

    def __str__(self):
        text =  "  ∙ part: '{}'\n".format(self.part)
        text += "  ∙ family: '{}'\n".format(self.family)
        text += "  ∙ version: '{}'\n".format(self.version)
        text += "  ∙ revision: '{}'\n".format(self.revision)
        text += "  ∙ flash_addr: 0x{:08x}\n".format(self.flash_addr)
        text += "  ∙ flash_size: 0x{:08x}\n".format(self.flash_size)
        return text

class Commander:

    def __init__(self, conn):
        self.conn = conn

    def execute(self, args, output = True, check = False):
        args.insert(0, 'commander')
        if self.conn.serial_num:
            args.extend(["--serialno", self.conn.serial_num])
        elif self.conn.ip_addr:
            if self.conn.port:
                args.extend(["--ip", "{}:{}".format(self.conn.ip_addr, self.conn.port)])
            else:
                args.extend(["--ip", self.conn.ip_addr])
        cmd = ' '.join(args)
        return execute(args, output, check)

    def info(self):
        res = self.execute(['device', 'info'], True, True)
        return DeviceInfo(res)

    def flash(self, image_path):
        return self.execute(['flash' , image_path], False, True)
