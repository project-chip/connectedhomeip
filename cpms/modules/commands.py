from .util import *
from .encoding import *

class Command(object):
    NONE = 0
    INIT = 1
    CSR = 2
    IMPORT = 3
    SETUP = 4
    SPAKE2p = 10
    STRINGS = [ 'None', 'INIT', 'CSR', 'IMPORT', 'SETUP' ]

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
        # print("◦ {}".format(cmd.hex()))
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
        # Decode
        enc = Encoder(resp)
        rid = enc.getUint8()
        err = enc.getInt32()
        # print("✧ cmd:{}/{}, err:{}".format(self.id, rid, err))
        if rid != self.id:
            fail("Unexpected response {} to command {}".format(rid, self.id))
        if err:
            fail("Command {} failed with error {}".format(self.name, hex(err)))
        return self.decode(enc, err)

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
        file = open(path, "rb")
        contents = bytearray(file.read())
        file.close()
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
        if args.vendor_name:
            self.vendor_name = bytes(args.vendor_name, 'utf-8')
        else:
            self.vendor_name = None

        # Product
        self.product_id = args.product_id
        if args.product_name is None:
            self.product_name = None
        else:
            self.product_name = bytes(args.product_name, 'utf-8')

        if args.product_label is None:
            self.product_label = None
        else:
            self.product_label = bytes(args.product_label, 'utf-8')

        if args.product_url is None:
            self.product_url = None
        else:
            self.product_url = bytes(args.product_url, 'utf-8')

        if args.part_number is None:
            self.part_number = None
        else:
            self.part_number = bytes(args.part_number, 'utf-8')

        if args.hw_version is None:
            self.hw_version = None
        else:
            self.hw_version = args.hw_version.to_bytes(2, "little")

        if args.hw_version_str is None:
            self.hw_version_str = None
        else:
            self.hw_version_str = bytes(args.hw_version_str, 'utf-8')

        if args.manufacturing_date is None:
            self.manufacturing_date = None
        else:
            self.manufacturing_date = bytes(args.manufacturing_date, 'utf-8')

        # Commissioning
        if args.unique_id is None:
            self.unique_id = None
        else:
            self.unique_id = bytearray.fromhex(args.unique_id)
        self.commissioning_flow = args.commissioning_flow
        self.rendezvous_flags = args.rendezvous_flags
        self.discriminator = args.discriminator or 0

        # SPAKE2+
        self.spake2p_verifier = bytes(args.spake2p.verifier, 'utf-8')
        self.spake2p_iterations = args.spake2p.iterations.to_bytes(4, 'little')
        self.spake2p_salt = bytes(args.spake2p.salt, 'utf-8')
        self.passcode = args.spake2p.passcode

    def encode(self, enc):
        enc.addUint32(self.vendor_id)
        enc.addArray(self.vendor_name)
        enc.addUint32(self.product_id)
        enc.addArray(self.product_name)
        enc.addArray(self.product_label)
        enc.addArray(self.product_url)
        enc.addArray(self.part_number)
        enc.addArray(self.hw_version)
        enc.addArray(self.hw_version_str)
        enc.addArray(self.manufacturing_date)
        enc.addArray(self.unique_id)
        enc.addUint32(self.commissioning_flow)
        enc.addUint32(self.rendezvous_flags)
        enc.addUint16(self.discriminator)
        enc.addArray(self.spake2p_verifier)
        enc.addArray(self.spake2p_iterations)
        enc.addArray(self.spake2p_salt)
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
