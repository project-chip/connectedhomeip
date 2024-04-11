import os
import hashlib
from enum import Enum
import modules.util as _util
import modules.manager as _base
import modules.signing_server as _pki
from modules.parameters import Types, Formats, ID
from .encoding import *
from .exporter import *


class Protocol(_base.ProvisionProtocol):
    kVersion        = 2
    # Flags
    kResponseFlag   = 0x80
    # Limits
    kMaxPackageSize = 128
    kHeaderSize     = 5 # Version(1) + Command(1) + Count(1) + Size(2)
    kChecksumSize   = 2

    def __init__(self) -> None:
        super().__init__()

    def execute(self, paths, args, chan):
        chan.open()
        action = args.str(ID.kAction)
        # Binary export
        if 'binary' == action:
            e = Exporter(paths, args)
            return e.export()
        # Init
        init = InitCommand(paths, args)
        if 'auto' == action:
            if args.bool(ID.kCsrMode):
                self.csr(paths, args, chan)
            # Write all
            write = AutoCommand(paths, args)
            init.execute(chan)
            write.execute(chan)
            write = FinishCommand(paths, args)
            write.execute(chan)
        elif 'write' == action:
            # Write non-nulls
            write = WriteCommand(paths, args)
            init.execute(chan)
            write.execute(chan)
        elif 'read' == action:
            # Read
            read = ReadCommand(paths, args, args.str(ID.kExtra))
            init.execute(chan)
            read.execute(chan)
        else:
            raise ValueError("Action not supported: \"{}\"".format(action))
        chan.close()

    def csr(self, paths, args, chan):
        base_dir = paths.base()
        # PAI cert
        pai_cert_path = args.str(ID.kPaiCert)
        if pai_cert_path is None:
            raise ValueError("Missing PAI certificate path")
        if not os.path.exists(pai_cert_path) or not os.path.isfile(pai_cert_path):
            raise ValueError("Missing PAI certificate file")
        # PAI key
        pai_key_path = args.str(ID.kPaiKey)
        if pai_key_path is None:
            raise ValueError("Missing PAI key path")
        if not os.path.exists(pai_key_path) or not os.path.isfile(pai_key_path):
            raise ValueError("Missing PAI key file")
        # DAC
        dac_path = paths.temp('dac_cert.der')
        args.set(ID.kDacCert, dac_path)
        # CSR
        cmd = CsrCommand(paths, args)
        cmd.execute(chan)
        # Write CSR to file
        csr_path = paths.temp('csr.pem')
        _util.File(csr_path).write(args.value(ID.kCsrFile))
        # Sign
        signer = _pki.SigningServer(base_dir, csr_path, pai_cert_path, pai_key_path, dac_path)
        signer.sign()


class Command:
    NONE    = 0
    INIT    = 1
    FINISH  = 2
    WRITE   = 3
    READ    = 4
    CSR     = 5

    def __init__(self, paths, args, cid, name, send_values = True):
        self.paths = paths
        self.args = args
        self.id = cid
        self.name = name
        self.out_list = []
        self.in_list = []
        self.in_map = {}
        self.send_values = send_values
        self.incoming = Context()

    def add(self, a, include_nulls = False, feedback = False):
        if include_nulls or (a.value is not None):
            self.out_list.append(a)
        if feedback:
            self.in_map[a.id] = a

    def addCustom(self):
        for k, g in self.args.custom.items():
            for k, a in g.items():
                if a.is_user_input and not a.hidden:
                    self.put(k, False) # No nulls, no feedback

    def put(self, k, include_nulls = False, feedback = False):
        a = self.args.get(k)
        return self.add(a, include_nulls, feedback or a.feedback)

    def execute(self, chan):
        print("-------- {} --------".format(self.name))
        # Send all arguments
        pack_count, done = self.sendArguments(chan, 1)
        # Receive remaining arguments
        if not done:
            self.receiveArguments(chan, pack_count)
        # Print feedback
        self.printIncoming()

    def sendArguments(self, chan, pack_count):
        arg = None
        arg_count = 0
        arg_total = len(self.out_list)
        outgoing = Buffer()
        completed = False
        self.incoming.reset()
        # Transmit arguments
        while arg_count < arg_total:
            print("Package #{}".format(pack_count))
            # Pack as many arguments as possible in the payload
            payload = Buffer(Protocol.kMaxPackageSize - Protocol.kHeaderSize - Protocol.kChecksumSize)
            while (payload.spare() > 0) and (arg_count < arg_total):
                arg = self.out_list[arg_count]
                # Encode argument (or use leftover from previous package)
                if 0 == outgoing.left():
                    Coder.encode(arg, self.encodeValue(arg), (arg.id in self.in_map), outgoing)
                # Add encoded argument (or fragment thereof)
                frag = outgoing.getBinary(min(outgoing.left(), payload.spare()))
                payload.addBinary(frag)
                # If the argument is fully sent, continue with next
                if 0 == outgoing.left():
                    arg_count += 1
                # Debug
                pos1 = "({}/{})".format(arg_count, arg_total)
                pos2 = "({}/{})".format(len(frag), outgoing.size())
                max_hex = 32
                frag_hex = (len(frag) < max_hex) and frag.hex() or (frag[:max_hex].hex() + '...')
                print("{:>10} {:04x} {:20} {:>10} {}".format(pos1, arg.id, arg.name, pos2, frag_hex))

            # Send package
            req = self.sendPackage(chan, pack_count, payload.serialize())
            # Receive acknowledge + data
            completed = self.receivePackage(chan, pack_count, req)
            pack_count = (pack_count + 1) % 0xff
        print()
        return pack_count, completed

    def receiveArguments(self, chan, pack_count):
        req = res = None
        completed = False
        while not completed:
            # Send follow-up request
            req = self.sendPackage(chan, pack_count, None)
            # Receive acknowledge + data
            completed = self.receivePackage(chan, pack_count, req)
            pack_count = (pack_count + 1) % 0xff

    def sendPackage(self, chan, counter, payload):
        if payload is None: payload = bytearray()
        # Encode header
        pack = Buffer(Protocol.kMaxPackageSize)
        pack.addInt8u(Protocol.kVersion)
        pack.addInt8u(self.id)
        pack.addInt8u(counter)
        # Add payload
        pack.addInt8u(len(payload))
        pack.addBinary(payload)
        # Add hash
        pack_hash = hashlib.sha256(pack.data).digest()
        pack.addBinary(pack_hash[:2])
        req = pack.serialize()
        # Write data to the channel
        chan.write(req)
        # print("{}>> Sent({}): {}".format(_util.MARGIN, len(req), req.hex()))
        return req

    def receivePackage(self, chan, counter, req):
        res = chan.read()
        # print("{}<< Response({}): {}".format(_util.MARGIN, len(res), res.hex()))
        pack = Buffer(None, res[:(len(res) - Protocol.kChecksumSize)])
        # Decode header
        try:
            ver = pack.getInt8u()
            rid = pack.getInt8u()
            cnt = pack.getInt8u()
            err = pack.getInt32u()
        except:
            self.fail(req, res, "Invalid header.")
        # Decode payload
        try:
            # Validations
            if ver != Protocol.kVersion:
                raise ValueError("Version mismatch: {} != {}".format(ver, Protocol.kVersion))
            if rid != (self.id | Protocol.kResponseFlag):
                raise ValueError("Command mismatch: 0x{:02x} != 0x{:02x}".format(rid, self.id | Protocol.kResponseFlag))
            if cnt != counter:
                raise ValueError("Counter mismatch: {} != {}".format(cnt, counter))
            if err:
                raise ValueError("{} failed with error 0x{:08x}".format(self.name, err))
            # Decoding
            return self.decodePayload(pack)
        except Exception as e:
            self.fail(req, res, e)

    def encodeValue(self, a):
        if not self.send_values:
            return None
        elif (Types.BINARY == a.type) and (Formats.PATH == a.format) and os.path.isfile(a.value):
            return _util.BinaryFile(a.value).read()
        else:
            return a.value

    def decodePayload(self, pack):
        size = pack.getInt8u()
        while pack.left() > 0:
            comp = Coder.decode(self.incoming, pack)
            if comp:
                # Complete package
                self.processIncoming(self.incoming, self.incoming.value)
                self.incoming.reset()
            else:
                # Incomplete package
                return False
        return True

    def processIncoming(self, ctx, value):
        arg = self.args.get(ctx.id)
        if Formats.PATH == arg.format:
            if arg.value is None:
                # Use temporary file
                arg.set(self.paths.temp("{}.bin".format(arg.name)))
            _util.BinaryFile(arg.value).write(value)
        else:
            arg.set(value, None, False)
        self.in_list.append(arg)

    def printIncoming(self):
        incoming_count = len(self.in_list);
        if incoming_count > 0:
            print("Incoming({}):".format(incoming_count))
            for a in self.in_list:
                print("{}+ {}".format(_util.MARGIN, a))
            print()

    def fail(self, req, res, message):
        if req is not None:
            print("\n{}> Sent({}): {}".format(_util.MARGIN, len(req), req.hex()))
        if res is not None:
            print("{}< Response({}): {}".format(_util.MARGIN, len(res), res.hex()))
        _util.fail(message, self.paths)


class InitCommand(Command):

    def __init__(self, paths, args):
        super().__init__(paths, args, Command.INIT, 'Init')
        self.put(ID.kFlashAddress)
        self.put(ID.kFlashSize)
        self.put(ID.kFlashPageSize, True, True)
        self.put(ID.kBaseAddress, True, True)


class CsrCommand(Command):

    def __init__(self, paths, args):
        super().__init__(paths, args, Command.CSR, 'CSR')
        # Outgoing
        self.put(ID.kVendorId)
        self.put(ID.kProductId)
        self.put(ID.kCommonName)
        self.put(ID.kCsrFile, True, True)


class WriteCommand(Command):

    def __init__(self, paths, args, feedback_list = []):
        super().__init__(paths, args, Command.WRITE, "Write")
        # Well-known arguments
        for n, g in args.groups.items():
            if 'options' == n: continue
            for k, a in g.items():
                if a.is_user_input:
                    # No nulls, no feedback
                    self.add(a)
        # Custom arguments
        self.addCustom()
        if len(self.out_list) < 1: raise ValueError("Nothing to write.")


class AutoCommand(Command):
    OUTGOING = [
        ID.kVersion,
        # Instance Info
        ID.kSerialNumber,
        ID.kVendorId,
        ID.kVendorName,
        ID.kProductId,
        ID.kProductName,
        ID.kProductLabel,
        ID.kProductUrl,
        ID.kPartNumber,
        ID.kHwVersion,
        ID.kHwVersionStr,
        ID.kManufacturingDate,
        ID.kUniqueId,
        # Commissionable Data
        ID.kDiscriminator,
        ID.kSpake2pPasscode,
        ID.kSpake2pIterations,
        ID.kSpake2pSalt,
        ID.kSpake2pVerifier,
        ID.kSetupPayload,
        ID.kCommissioningFlow,
        ID.kRendezvousFlags,
        # Attestation Credentials
        ID.kFirmwareInfo,
        ID.kDacCert,
        ID.kPaiCert,
        ID.kCertification
    ]

    INCOMING = [
        ID.kDiscriminator,
        ID.kSpake2pPasscode,
        ID.kUniqueId,
        ID.kSetupPayload,
        ID.kHwVersionStr
    ]

    def __init__(self, paths, args):
        super().__init__(paths, args, Command.WRITE, "Write")
        # Well-known arguments. Include nulls, and non-user-inputs
        for k in AutoCommand.OUTGOING:
            # Include nulls, feedback if incoming
            self.put(k, True, k in AutoCommand.INCOMING)
        # DAC key is required if CSR is not used
        if not self.args.bool(ID.kCsrMode):
            # Include even if null, no feedback
            self.put(ID.kDacKey, True, False)
        # Custom arguments
        self.addCustom()
        if len(self.out_list) < 1: raise ValueError("Nothing to write.")


class ReadCommand(Command):

    def __init__(self, paths, args, names):
        super().__init__(paths, args, Command.READ, 'Read', False)

        arg_list = args.findList(names)
        if len(arg_list) < 1: raise ValueError("Nothing to read.")
        for i, a in arg_list.items():
            if Formats.PATH != a.format: a.set(None)
            # Add both out_list and in_map
            self.add(a, True, True)
        if len(self.in_map) < 1: raise ValueError("Nothing to read.")


class FinishCommand(Command):

    def __init__(self, paths, args):
        super().__init__(paths, args, Command.FINISH, 'Finish', False)
