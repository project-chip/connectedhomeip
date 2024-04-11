import os
import yaml
import modules.util as _util
from modules.parameters import ID


class Device:
    CONFIG_FILE = 'devices.yaml'

    def __init__(self, paths, args, part_num, flash_size) -> None:
        self.label = None
        self.ram_addr = None
        self.flash_addr = None
        self.flash_size = flash_size
        self.stack_size = None
        self.firmware = args.str(ID.kGeneratorFW)
        self.load(paths, part_num, args.str(ID.kVersion))

    def __str__(self) -> str:
        return "({}) ram:0x{:08x}, flash:0x{:08x}|0x{:08x}, stack:0x{:04x}, image:{}".format(self.label, self.ram_addr, self.flash_addr, self.flash_size, self.stack_size, self.firmware)

    def load(self, paths, part_num, version):
        filename = paths.base(Device.CONFIG_FILE)
        if part_num is None:
            _util.fail("Missing target part number")
        if not os.path.exists(filename):
            _util.fail("Missing device configuration ({})".format(Device.CONFIG_FILE))

        # Load the device configuration
        info = None
        pn = part_num.lower()
        y = _util.YamlFile(filename).read()
        for x in y:
            if self.match(pn, x, y):
                self.label = x
                info = y[x]
                break
        if info is None:
            _util.fail("Invalid part number \"{}\"".format(part_num))

        self.ram_addr = info['ram_addr']
        self.flash_addr = int(info['flash_addr'])
        self.stack_size = int(info['stack_size'])

        # Search for a firmware for the given version, if needed
        if self.firmware is None:
            image = None
            version_len = len(version)
            for y in info['firmware']:
                v = y['version']
                prefix = v[:version_len]
                if prefix > version:
                    break
                if version == prefix:
                    image = y['file']
            if image is None:
                _util.fail("Missing firmware for \"{}\" in version \"{}\"".format(part_num, version))

            self.firmware = paths.base("images/{}".format(image))

    def match(self, pn, id, y):
        if pn.startswith(id.lower()):
            return True
        if ('alias' in y):
            alias = y['alias']
            if alias:
                for a in alias:
                    if pn.startswith(a.lower()):
                        return True
        return False