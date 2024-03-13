import os
import modules.util as _util
from abc import ABC, abstractmethod
from modules.parameters import Types, Formats


class Formatter:

    def __init__(self, args, is_user_input = False) -> None:
        self.args = args
        self.is_user_input = is_user_input

    @staticmethod
    def read(filename):
        if filename is None: return {}
        if not os.path.exists(filename):
            raise ValueError("Missing inputs file \"{}\"".format(filename))
        return _util.JsonFile(filename).read()

    @staticmethod
    def write(filename, data):
        return _util.JsonFile(filename).write(data)

    def parseVersion(self, filename):
        json = Formatter.read(filename)
        return json['version']

    def parseAll(self, filename):
        json = Formatter.read(filename)
        # Parse main parameters
        self.parse(json)
        # Parse custom parameters
        self.parseCustom(json)

    def exportAll(self, filename):
        # Parse main parameters
        data = self.format()
        # Parse custom parameters
        data = self.formatCustom(data)
        Formatter.write(filename, data)

    def parse(self, main):
        # Version
        self.extract(main, 'version')
        self.extract(main, 'action')
        self.extract(main, 'extra')
        # Options
        options = ('options' in main) and main['options'] or None
        self.extract(options, 'stop')
        self.extract(options, 'parameters', 'params')
        self.extract(options, 'output')
        self.extract(options, 'temp')
        self.extract(options, 'device')
        self.extract(options, 'channel')
        self.extract(options, 'generate')
        self.extract(options, 'csr')
        self.extract(options, 'gen_fw')
        self.extract(options, 'prod_fw')
        self.extract(options, 'cert_tool')
        self.extract(options, 'pylink_lib')

    def format(self, main = {}):
        # Version
        self.insert(main, 'version')
        # Options
        options = {}
        self.insert(options, 'stop')
        self.insert(options, 'params', 'parameters')
        self.insert(options, 'inputs')
        self.insert(options, 'output')
        self.insert(options, 'temp')
        self.insert(options, 'device')
        self.insert(options, 'channel')
        self.insert(options, 'generate')
        self.insert(options, 'csr')
        self.insert(options, 'gen_fw')
        self.insert(options, 'prod_fw')
        self.insert(options, 'cert_tool')
        self.insert(options, 'pylink_lib')
        if len(options) > 0: main['options'] = options
        # Matter
        main['matter'] = matter = {}
        return main

    def parseCustom(self, main):
        for g, group in self.args.custom.items():
            if g in main:
                gj = main[g]
                for i, p in group.items():
                    self.extract(gj, p.name)

    def formatCustom(self, main):
        for g, group in self.args.custom.items():
            data = {}
            for i, p in group.items():
                self.insert(data, p.name)
            if len(data) > 0:
                main[g] = data
        return main

    def extract(self, data, tag, tag2 = None):
        if data is None: return
        arg = self.args.find(tag2 or tag)
        if tag in data:
            arg.set(data[tag])
            arg.is_user_input = self.is_user_input

    def insert(self, data, tag, tag2 = None):
        if data is None: return
        arg = self.args.find(tag)
        if (arg.value is not None) and (not arg.hidden):
            if isinstance(arg.value, bytearray):
                value = arg.str()
            else:
                value = arg.value
            data[tag2 or tag] = value
