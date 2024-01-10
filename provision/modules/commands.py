from .util import *
from .encoding import *
from .util import *

class Command(object):
    NONE = 0
    INIT = 1
    CSR = 2
    IMPORT = 3
    SETUP = 4
    SPAKE2p = 10
    STRINGS = [ 'None', 'Init', 'CSR', 'Import', 'Setup' ]

    def __init__(self, cid):
        self.id = cid
        self.name = Command.STRINGS[cid]
        # self.error = 0

    def __str__(self):
        return Encoder.hex(self.serialize())

    def serialize(self):
        enc = Encoder()
        enc.addUint8(self.id)
        self.encode(enc)
        cmd = enc.serialize()
        print("▵ {} send({})".format(self.name, len(cmd)))
        return cmd

    def encode(self, enc):
        return bytearray()

    def decode(self, enc):
        return 0

    def execute(self, serial):
        # Send
        serial.start()
        serial.send(self.serialize())
        # Receive
        resp = serial.receive()
        serial.stop()
        print("▴ {} response({})".format(self.name, len(resp)))
        # Decode
        try:
            enc = Encoder(resp)
            rid = enc.getUint8()
            err = enc.getInt32()
        except:
            fail("Invalid header({}): {}".format(len(resp), bytearray(resp).hex()))
        if rid != self.id:
            fail("Unexpected response {} to command {}".format(rid, self.id))
        if err:
            fail("Command {} failed with error {}".format(self.name, hex(err)))
        try:
            return self.decode(enc, err)
        except:
            fail("Invalid response")

class InitCommand(Command):

    def __init__(self, info):
        super().__init__(Command.INIT)
        self.flash_addr = info.flash_addr
        self.flash_size = info.flash_size

    def encode(self, enc):
        enc.addUint32(self.flash_addr)
        enc.addUint32(self.flash_size)

    def decode(self, enc, err):
        addr = enc.getUint32()
        page = enc.getUint32()
        print("Init:\n  ∙ addr:{:08x}, page:{}".format(addr, page))


class CsrCommand(Command):

    def __init__(self, cn, vid, pid, kid):
        super().__init__(Command.CSR)
        self.common_name = cn # bytearray(cn, 'utf-8')
        self.vendor_id = int(vid)
        self.product_id = int(pid)
        self.key_id = int(kid)

    def encode(self, enc):
        enc.addString(self.common_name)
        enc.addUint16(self.vendor_id)
        enc.addUint16(self.product_id)
        enc.addUint32(self.key_id)

    def decode(self, enc, err):
        kid = enc.getUint32()
        csr = enc.getString()
        print("CSR, key:{}, len:{}\n{}".format(kid, len(csr), csr))
        return (kid, csr)


class ImportCommand(Command):
    KEY = 1
    DAC = 2
    PAI = 3
    CD = 4
    STRINGS = [ '?', 'KEY', 'DAC', 'PAI', 'CD' ]

    def __init__(self, fid, kid, path, do_flash = False):
        super().__init__(Command.IMPORT)
        self.file_id = fid
        self.key_id = kid
        self.do_flash = do_flash
        self.data = self.read(path)
        self.name = ImportCommand.STRINGS[fid]

    def read(self, path):
        contents = bytearray()
        with open(path, 'rb') as f:
            contents = bytearray(f.read())
        return contents

    def encode(self, enc):
        enc.addUint8(self.file_id)
        enc.addUint32(self.key_id)
        enc.addUint8(self.do_flash)
        enc.addArray(self.data)

    def decode(self, enc, err):
        kid = enc.getUint32()
        off = enc.getUint32()
        size = enc.getUint32()
        print("Import({}):\n  ∙ key:{}, off:{}, size:{}".format(self.name, kid, hex(off), size))
        return (kid, off, size)


class SetupCommand(Command):

    def __init__(self, args):
        super().__init__(Command.SETUP)

        # Vendor
        self.vendor_id = args.vendor_id
        self.vendor_name = args.vendor_name
        # Product
        self.product_id = args.product_id
        self.product_name = args.product_name
        self.product_label = args.product_label
        self.product_url = args.product_url
        # Hardware
        self.part_number = args.part_number
        self.hw_version = args.hw_version
        self.hw_version_str = args.hw_version_str
        self.manufacturing_date = args.manufacturing_date
        # Commissioning
        self.unique_id = args.unique_id
        self.commissioning_flow = args.commissioning_flow
        self.rendezvous_flags = args.rendezvous_flags
        self.discriminator = args.discriminator
        # SPAKE2+
        self.spake2p_verifier = args.spake2p.verifier
        self.spake2p_iterations = args.spake2p.iterations
        self.spake2p_salt = args.spake2p.salt
        self.passcode = args.spake2p.passcode

    def encode(self, enc):
        enc.addUint32(self.vendor_id)
        enc.addString(self.vendor_name)
        enc.addUint32(self.product_id)
        enc.addString(self.product_name)
        enc.addString(self.product_label)
        enc.addString(self.product_url)
        enc.addString(self.part_number)
        enc.addUint16(self.hw_version)
        enc.addString(self.hw_version_str)
        enc.addString(self.manufacturing_date)
        enc.addArray(self.unique_id)
        enc.addUint32(self.commissioning_flow)
        enc.addUint32(self.rendezvous_flags)
        enc.addUint16(self.discriminator)
        enc.addString(self.spake2p_verifier)
        enc.addUint32(self.spake2p_iterations)
        enc.addString(self.spake2p_salt)
        enc.addUint32(self.passcode)

    def decode(self, enc, err):
        passc = enc.getUint32()
        disc = enc.getUint16()
        unique = enc.getArray()
        payload = enc.getArray()
        # hash = enc.getArray()
        print("Setup:\n  ∙ passcode:  {}\n  ∙ discriminator: {}\n  ∙ uid: {}\n  ∙ payload: {}\n".format(hex(passc), hex(disc), unique.hex(), payload.hex()))
        assert(0 == err)
        return payload
