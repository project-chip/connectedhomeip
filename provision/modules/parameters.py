from enum import Enum
from abc import ABC, abstractmethod
import modules.util as _util
import os
import yaml

def parseInt(i):
    return int(i, 0)

def parseHex(x):
    return bytearray.fromhex(x.removeprefix('0x'))

class Types(Enum):
    BINARY  = 0x00
    INT8U   = 0x01
    INT16U  = 0x02
    INT32U  = 0x03
    INT64U  = 0x04
    FLOAT   = 0x05
    STRING  = 0x08
    INT8S   = 0x09
    INT16S  = 0x0a
    INT32S  = 0x0b
    INT64S  = 0x0c
    FLOATS  = 0x0d

    @staticmethod
    def string(t):
        if Types.BINARY == t: return 'binary'
        if Types.STRING == t: return 'string'
        if Types.INT8U == t:  return 'int8u'
        if Types.INT8S == t:  return 'int8s'
        if Types.INT16U == t: return 'int16u'
        if Types.INT16S == t: return 'int16s'
        if Types.INT32U == t: return 'int32u'
        if Types.INT32S == t: return 'int32s'
        if Types.INT64U == t: return 'int64u'
        if Types.INT64S == t: return 'int64s'
        if Types.INT64U == t: return 'float(u)'
        if Types.INT64S == t: return 'float(s)'
        return '?'

class Formats(Enum):
    NONE = 0
    DECIMAL = 1
    BOOL = 2
    HEX = 3
    STRING = 4
    PATH = 5

    @staticmethod
    def string(t):
        if Formats.NONE == t: return None
        if Formats.DECIMAL == t: return 'dec'
        if Formats.BOOL == t: return 'bool'
        if Formats.HEX == t:  return 'hex'
        if Formats.STRING == t:  return 'string'
        if Formats.PATH == t: return 'path'
        return '?'

class ID:
    kNone = 0
    # Internal
    kFlashAddress       = 0x0101
    kFlashSize          = 0x0102
    kFlashPageSize      = 0x0103
    kBaseAddress        = 0x0104
    kCsrFile            = 0x0105
    # Options
    kVersion            = 0x0111
    kAction             = 0x0112
    kExtra              = 0x0113
    kStop               = 0x0114
    kParamsPath         = 0x0121
    kInputsPath         = 0x0122
    kOutputPath         = 0x0123
    kTemporaryDir       = 0x0124
    kDevice             = 0x0131
    kChannel            = 0x0132
    kGenerateCreds      = 0x0133
    kCsrMode            = 0x0134
    kGeneratorFW        = 0x0135
    kProductionFW       = 0x0136
    kCertToolPath       = 0x0137
    kPylinkLib          = 0x013a
    # Instance Info
    kSerialNumber       = 0x0141
    kVendorId           = 0x0142
    kVendorName         = 0x0143
    kProductId          = 0x0144
    kProductName        = 0x0145
    kProductLabel       = 0x0146
    kProductUrl         = 0x0147
    kPartNumber         = 0x0148
    kHwVersion          = 0x0151
    kHwVersionStr       = 0x0152
    kManufacturingDate  = 0x0153
    kUniqueId           = 0x0154
    # Commissionable Data
    kDiscriminator      = 0x0161
    kSpake2pPasscode    = 0x0162
    kSpake2pIterations  = 0x0163
    kSpake2pSalt        = 0x0164
    kSpake2pVerifier    = 0x0165
    kSetupPayload       = 0x0166
    kCommissioningFlow  = 0x0167
    kRendezvousFlags    = 0x0168
    # Attestation Credentials
    kFirmwareInfo       = 0x0181
    kCertification      = 0x0182
    kCdCert             = 0x0183
    kCdKey              = 0x0184
    kPaaCert            = 0x0191
    kPaaKey             = 0x0192
    kPaiCert            = 0x0193
    kPaiKey             = 0x0194
    kDacCert            = 0x0195
    kDacKey             = 0x0196
    kKeyId              = 0x0197
    kKeyPass            = 0x0198
    kPKCS12             = 0x0199
    kCommonName         = 0x01a1
    kOtaKey             = 0x01a2


class Parameter:
    kKnownFlag     = 0x1000

    def __init__(self, y) -> None:
        self.id = None
        self.desc = None
        self.name = None
        self.long = None
        self.short = None
        self.check = True
        self.hidden = False
        self.fixed = False
        self.feedback = None
        self.default = None
        self.min = None
        self.max = None
        self.invalid = None
        self.type = None
        self.format = None

    def __str__(self) -> str:
        ids = (self.id is not None) and "0x{:04}".format(self.id) or '!'
        name = self.name and "\"{}\"".format(self.name) or '?'
        rng = self.max and " [{}, {}]".format(self.min, self.max) or ''
        short = self.short and "-{}".format(self.short) or ''
        long = "-{}".format(self.long)
        return "{} {:20} {:4} {:20} '{}'{}".format(ids, name, short, long, self.desc, rng)

    def parse(self, y):
        if 'id' in y:       self.id = y['id']
        if 'desc' in y:     self.desc = y['desc']
        if 'name' in y:     self.name = self.long = y['name']
        if 'long' in y:     self.long = y['long']
        if 'short' in y:    self.short = y['short']
        if 'check' in y:    self.check = y['check']
        if 'hidden' in y:   self.hidden = y['hidden']
        if 'fixed' in y:    self.fixed = y['fixed']
        if 'feedback' in y:    self.feedback = y['feedback']
        if 'default' in y:  self.default = y['default']
        if 'invalid' in y:  self.invalid = y['invalid']
        typ = ('type' in y) and y['type'] or None
        fmt = ('format' in y) and y['format'] or None
        default_max = None
        if 'int8u' == typ:
            self.type = Types.INT8U
            self.format = ('hex' == fmt) and Formats.HEX or Formats.DECIMAL
            self.parser = parseInt
            default_max = 0xff
        elif 'int16u' == typ:
            self.type = Types.INT16U
            self.format = ('hex' == fmt) and Formats.HEX or Formats.DECIMAL
            self.parser = parseInt
            default_max = 0xffff
        elif 'int32u' == typ:
            self.type = Types.INT32U
            self.format = ('hex' == fmt) and Formats.HEX or Formats.DECIMAL
            self.parser = parseInt
            default_max = 0xffffffff
        elif 'bool' == typ:
            self.type = Types.INT8U
            self.format = Formats.BOOL
            self.parser = bool
        elif 'flag' == typ:
            self.type = Types.INT8U
            self.format = Formats.BOOL
            self.parser = None
        elif 'string' == typ:
            self.type = Types.BINARY
            self.format = Formats.STRING
            self.parser = str
        elif 'date' == typ:
            self.type = Types.BINARY
            self.format = Formats.STRING
            self.parser = str
            default_max = 10
        elif 'path' == typ:
            self.type = Types.BINARY
            self.format = Formats.PATH
            self.parser = str
        elif 'binary' == typ:
            self.type = Types.BINARY
            self.format = Formats.HEX
            self.parser = parseHex
        if 'min' in y:
            self.min = y['min']
        if 'max' in y:
            self.max = y['max']
        elif self.max is None:
            self.max = default_max
        if self.type is None: raise ValueError("Invalid \"{}\" type: \"{}\"".format(self.name, typ))
        return self

    def copy(self, p):
        if p.id is not None:        self.id = p.id
        if p.desc is not None:      self.desc = p.desc
        if p.name is not None:      self.name = p.name
        if p.long is not None:      self.long = p.long
        if p.short is not None:     self.short = p.short
        if p.check is not None:     self.check = p.check
        if p.hidden is not None:    self.hidden = p.hidden
        if p.fixed is not None:     self.fixed = p.fixed
        if p.default is not None:   self.default = p.default
        if p.min is not None:       self.min = p.min
        if p.max is not None:       self.max = p.max
        if p.invalid is not None:   self.invalid = p.invalid
        if p.type is not None:      self.type = p.type
        if p.format is not None:    self.format = p.format
        if p.feedback is not None:  self.feedback = p.feedback

    @staticmethod
    def tag(self, y, x, default_value = None):
        if x in y: return y[x]
        return default_value

    def range(self):
        min = (self.min is not None) and self.min or 0
        return (self.max is not None) and range(min, self.max + 1) or None

    @staticmethod
    def validate(p, x):
        if x is None: return None
        if not p.check: return x
        h = (x is not None) and (x.__hash__) or None
        if (p.invalid is not None) and (h is not None) and (x in p.invalid):
            raise ValueError("Invalid \"{}\" value: {}".format(p.name, x))
        elif Types.INT8U == p.type:
            if (Formats.BOOL == p.format):
                return Parameter.validateBool(p, x)
            else:
                return Parameter.validateInt(p, x)
        elif (Types.INT16U == p.type) or (Types.INT32U == p.type):
            return Parameter.validateInt(p, x)
        elif (Types.BINARY == p.type):
            if (Formats.STRING == p.format):
                return Parameter.validateString(p, x)
            elif (Formats.PATH == p.format):
                return Parameter.validatePath(p, x)
            else:
                return Parameter.validateBinary(p, x)
        return x

    @staticmethod
    def validateInt(p, x):
        try:
            i = isinstance(x, str) and int(x, 0) or int(x)
        except Exception as e:
            raise ValueError("Invalid \"{}\" integer value: {}; {}".format(p.name, x, e))
        r = p.range()
        if r is not None:
            if i not in r:
                raise ValueError("Integer \"{}\" out of range: {} [{}, {}]".format(p.name, x, p.min, p.max))
        return i

    @staticmethod
    def validateBool(p, x):
        try:
            return bool(x)
        except Exception as e:
            raise ValueError("Invalid \"{}\" boolean value: {}; {}".format(p.name, x, e))

    @staticmethod
    def validateString(p, x):
        s = None
        if isinstance(x, str):
            s = x
        elif isinstance(x, bytearray):
            try:
                s = x.decode()
            except:
                raise ValueError("Invalid \"{}\" string value: {}".format(p.name, x))
        else:
            s = str(x)
        r = p.range()
        if r is not None:
            l = (s is not None) and len(s) or 0
            if l not in r:
                raise ValueError("String \"{}\" size out of range: {} [{}, {}]".format(p.name, l, p.min, p.max))
        return s

    @staticmethod
    def validatePath(p, x):
        s = str(x)
        if not os.path.exists(x):
            raise ValueError("Invalid \"{}\" path: {}".format(p.name, s))
        return s

    @staticmethod
    def validateBinary(p, x):
        if isinstance(x, str):
            x = bytearray.fromhex(x.removeprefix('0x'))
        if not isinstance(x, bytearray):
            raise ValueError("Invalid \"{}\" binary value: {}".format(p.name, x))
        r = p.range()
        if r is not None:
            sz = len(x)
            if sz not in r:
                raise ValueError("Binary \"{}\" size out of range: {} [{}, {}]".format(p.name, sz, p.min, p.max))
        return x



class ParameterList:

    PARAMS_FILENAME = 'parameters.yaml'

    def __init__(self, paths, custom_path = None) -> None:
        self.paths = paths
        self.names = {}
        self.ids = {}
        self.groups = {}
        self.custom = {}
        # Default parameters
        self.load(paths.base('modules/' + ParameterList.PARAMS_FILENAME))
        # Custom parameters
        self.load(custom_path, True)

    def load(self, filename, custom = False):
        if (filename is None) and custom:
            if os.path.exists(ParameterList.PARAMS_FILENAME):
                # Use default custom parameters
                filename = ParameterList.PARAMS_FILENAME
            else:
                # No custom file available
                return
        elif not os.path.exists(filename):
            raise ValueError("Invalid inputs path: '{}'".format(filename))
        # Read parameters file
        y = _util.YamlFile(filename).read()
        # Parse YAML
        for g, gy in y.items():
            group = (g in self.groups) and self.groups[g] or {}
            for py in gy:
                if not 'id' in py: raise ValueError("Missing parameter ID in \"{}\"".format(filename))
                p = self.add(py['id'], py)
                group[p.id] = p
            self.groups[g] = group
            if custom:
                self.custom[g] = group

    def add(self, id, y):
        if id in self.ids:
            p = self.ids[id]
        else:
            p = self.create(y)
            self.ids[id] = p
        p.parse(y)
        self.names[p.name] = p
        return p

    def create(self, y):
        return Parameter(y)

    def get(self, k):
        if k in self.ids:
            return self.ids[k]
        raise ValueError("Unknown parameter 0x{:02x}".format(k))

    def find(self, name):
        if name in self.names:
            return self.names[name]
        raise ValueError("Unknown parameter \"{}\"".format(name))

    def findList(self, names):
        name_list = [x.strip() for x in names.split(',')]
        args = {}
        for n in name_list:
            a = self.find(n)
            args[a.id] = a
        return args

    def help(self):
        for n, g in self.groups.items():
            if 'internal' == n: continue
            print("* {}:".format(n))
            for i, p in g.items():
                if p.hidden: continue
                short = "-{}".format(p.short)
                long = "--{}".format(p.long)
                param = "{}{:3} {}".format(_util.MARGIN, short, long)
                fmt = Formats.string(p.format)
                typ = "[{}]".format(fmt or Types.string(p.type))
                print("{:28} {:10}{}".format(param, typ, p.desc))
            print()
