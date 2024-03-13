import os
import modules.util as _util
import modules.manager as _base
import modules.credentials as _creds
import modules.signing_server as _pki
from modules.parameters import Types, Formats, ID
import modules.v1_0.encoding as _enc
from .exporter import *


class Protocol(_base.ProvisionProtocol):

    def __init__(self) -> None:
        super().__init__()

    def execute(self, paths, args, chan):
        chan.open()
        action = args.str(ID.kAction)
        if 'binary' == action:
            e = Exporter(paths, args)
            return e.export()
        # Init
        if 'auto' == action:
            # Auto
            self.executeAuto(paths, args, chan)
        else:
            raise ValueError("Action not supported: \"{}\"".format(action))
        chan.close()

    def executeAuto(self, paths, args, chan):
        # Initialize
        init = InitCommand(paths, args)
        init.execute(chan)
        # WARNING: is up to the storage implementation to assign the key ID,
        #          so custom key ID is currently NOT supported
        key_id = args.int(ID.kKeyId) or 0

        if args.bool(ID.kCsrMode):
            # CSR
            csr = CsrCommand(paths, args)
            csr.execute(chan)
        else:
            # DAC key
            dac_key = args.str(ID.kDacKey)
            if dac_key is None: _util.fail("Missing: DAC key")
            imp = ImportCommand(paths, args, ImportCommand.KEY, key_id, dac_key)
            imp.execute(chan)
        # DAC certificate
        dac_cert = args.str(ID.kDacCert)
        if dac_cert is None: _util.fail("Missing: DAC cert")
        imp = ImportCommand(paths, args, ImportCommand.DAC, key_id, dac_cert)
        imp.execute(chan)
        # PAI certificate
        pai_cert = args.str(ID.kPaiCert)
        if pai_cert is None: _util.fail("Missing: PAI cert")
        imp = ImportCommand(paths, args, ImportCommand.PAI, key_id,  pai_cert)
        imp.execute(chan)
        # Certification Declaration
        cd = args.str(ID.kCertification)
        if cd is None: _util.fail("Missing: Certification Declaration")
        imp = ImportCommand(paths, args, ImportCommand.CD, key_id, cd, True)
        imp.execute(chan)
        # Setup
        imp = self.createSetupCommand(paths, args)
        imp.execute(chan)
        print()

    # Creates the SetupCommand. This method allows derived classes
    # to change the encoding of the command arguments
    def createSetupCommand(self, paths, args):
        return SetupCommand(paths, args)


class Command(_enc.Encoder):
    NONE = 0
    INIT = 1
    CSR = 2
    IMPORT = 3
    SETUP = 4

    def __init__(self, paths, args, cid, name):
        super().__init__()
        self.paths = paths
        self.args = args
        self.id = cid
        self.name = name

    def encode(self):
        pass

    def decode(self):
        pass

    def execute(self, chan):
        print("{}:".format(self.name))
        # Encode
        self.addInt8u(self.id)
        self.encode()
        req = self.serialize()
        # Send
        chan.write(req)
        # Receive
        res = chan.read()
        # Decode
        try:
            # print("{}> Sent({}): {}".format(_util.MARGIN, len(req), req.hex()))
            # print("{}< Response({}): {}".format(_util.MARGIN, len(res), res.hex()))
            self.reset(res)
            rid = self.getInt8u()
            err = self.getInt32()
        except:
            self.fail(req, res, "Invalid header.")
        try:
            # Validations
            if rid != self.id:
                raise ValueError("Command mismatch: {} != {}".format(rid, self.id))
            if err:
                raise ValueError("{} failed with error 0x{:08x}".format(self.name, err))
            # Decoding
            self.decode()
        except Exception as e:
            self.fail(req, res, e)

    def fail(self, req, res, message):
        print("{}> Sent({}): {}".format(_util.MARGIN, len(req), req.hex()))
        print("{}< Response({}): {}".format(_util.MARGIN, len(res), res.hex()))
        _util.fail(message, self.paths)


class InitCommand(Command):

    def __init__(self, paths, args):
        super().__init__(paths, args, Command.INIT, 'Init')
        self.flash_addr = args.int(ID.kFlashAddress)
        self.flash_size = args.int(ID.kFlashSize)

    def encode(self):
        self.addInt32u(self.flash_addr)
        self.addInt32u(self.flash_size)

    def decode(self):
        addr = self.getInt32u()
        page = self.getInt32u()
        print("{}+ addr:{:08x}, page:{}".format(_util.MARGIN, addr, page))


class CsrCommand(Command):

    def __init__(self, paths, args):
        super().__init__(paths, args, Command.CSR, 'CSR')

        self.common_name = args.str(ID.kCommonName)
        self.vendor_id = args.int(ID.kVendorId)
        self.product_id = args.int(ID.kProductId)
        self.key_id = args.int(ID.kKeyId)
        self.base_dir = paths.base()
        # PAI cert
        self.pai_cert_path = args.str(ID.kPaiCert)
        if self.pai_cert_path is None:
            _util.fail("Missing PAI certificate path")
        if not os.path.exists(self.pai_cert_path) or not os.path.isfile(self.pai_cert_path):
            _util.fail("Missing PAI certificate file")
        # PAI key
        self.pai_key_path = args.str(ID.kPaiKey)
        if self.pai_key_path is None:
            _util.fail("Missing PAI key path")
        if not os.path.exists(self.pai_key_path) or not os.path.isfile(self.pai_key_path):
            _util.fail("Missing PAI key file")
        # DAC
        self.dac_path = paths.temp('dac_cert.der')
        args.set(ID.kDacCert, self.dac_path)
        # CSR
        self.csr_path = paths.temp('csr.pem')

    def execute(self, chan):
        # Read CSR
        super().execute(chan)
        # Sign
        signer = _pki.SigningServer(self.base_dir, self.csr_path, self.pai_cert_path, self.pai_key_path, self.dac_path)
        signer.sign()

    def encode(self):
        self.addString(self.common_name)
        self.addInt16u(self.vendor_id)
        self.addInt16u(self.product_id)
        self.addInt32u(self.key_id)

    def decode(self):
        kid = self.getInt32u()
        csr = self.getString()
        print("{}+ vendor_id:{:04x}, product_id:{:04x}, key:{}, req({}):\n{}".format(_util.MARGIN, self.vendor_id, self.product_id, kid, len(csr), csr))
        # Write CSR to file
        _util.File(self.csr_path).write(csr)


class ImportCommand(Command):
    KEY = 1
    DAC = 2
    PAI = 3
    CD = 4
    STRINGS = [ '?', 'KEY', 'DAC', 'PAI', 'CD' ]

    def __init__(self, paths, args, fid, kid, path, do_flash = False):
        super().__init__(paths, args, Command.IMPORT, 'Import')
        self.file_id = fid
        self.key_id = kid
        self.do_flash = do_flash
        self.content = _util.BinaryFile(path).read()
        self.name = ImportCommand.STRINGS[fid]

    def encode(self):
        self.addInt8u(self.file_id)
        self.addInt32u(self.key_id)
        self.addInt8u(self.do_flash)
        self.addArray(self.content)

    def decode(self):
        kid = self.getInt32u()
        off = self.getInt32u()
        size = self.getInt32u()
        print("{}+ key:{}, off:{}, size:{}".format(_util.MARGIN, kid, hex(off), size))

class SetupCommand(Command):

    def __init__(self, paths, args):
        super().__init__(paths, args, Command.SETUP, 'Setup')
        # Vendor
        self.vendor_id = args.int(ID.kVendorId)
        self.vendor_name = args.str(ID.kVendorName)
        # Product
        self.product_id = args.int(ID.kProductId)
        self.product_name = args.str(ID.kProductName)
        self.product_label = args.str(ID.kProductLabel)
        self.product_url = args.str(ID.kProductUrl)
        # Hardware
        self.part_number = args.str(ID.kPartNumber)
        self.hw_version = args.int(ID.kHwVersion)
        self.hw_version_str = args.str(ID.kHwVersion)
        self.manufacturing_date = args.str(ID.kHwVersion)
        # Commissioning
        self.unique_id = args.value(ID.kUniqueId)
        self.commissioning_flow = args.int(ID.kCommissioningFlow)
        self.rendezvous_flags = args.int(ID.kRendezvousFlags)
        self.discriminator = args.int(ID.kDiscriminator)
        # SPAKE2+
        self.spake2p_verifier = args.str(ID.kSpake2pVerifier)
        self.spake2p_iterations = args.int(ID.kSpake2pIterations)
        self.spake2p_salt = args.str(ID.kSpake2pSalt)
        self.passcode = args.int(ID.kSpake2pPasscode)

    def encode(self):
        self.addInt32u(self.vendor_id)
        self.addString(self.vendor_name)
        self.addInt32u(self.product_id)
        self.addString(self.product_name)
        self.addString(self.product_label)
        self.addString(self.product_url)
        self.addString(self.part_number)
        self.addInt16u(self.hw_version)
        self.addString(self.hw_version_str)
        self.addString(self.manufacturing_date)
        self.addArray(self.unique_id)
        self.addInt32u(self.commissioning_flow)
        self.addInt32u(self.rendezvous_flags)
        self.addInt16u(self.discriminator)
        self.addString(self.spake2p_verifier)
        self.addArray(self.spake2p_iterations.to_bytes(4, 'little'))
        self.addString(self.spake2p_salt)
        self.addInt32u(self.passcode)

    def decode(self):
        passc = self.getInt32u()
        disc = self.getInt16u()
        unique = self.getArray()
        payload = self.getArray()
        print("{}+ discriminator: {}".format(_util.MARGIN, hex(disc)))
        print("{}+ passcode: {}".format(_util.MARGIN, hex(passc)))
        print("{}+ uid: {}".format(_util.MARGIN, unique.hex()))
        print("{}+ payload: {}".format(_util.MARGIN, payload.hex()))
