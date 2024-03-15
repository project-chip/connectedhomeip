import os
import shutil
import subprocess
import modules.util as _util
import modules.tools as _tools
from modules.parameters import Types, Formats, ID


class Credentials:
    DEFAULT_CD_CERT = "credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem"
    DEFAULT_CD_KEY = "credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem"
    DEFAULT_PAA_CERT = "credentials/test/attestation/Chip-Test-PAA-NoVID-Cert.der"
    DEFAULT_PAA_KEY = "credentials/test/attestation/Chip-Test-PAA-NoVID-Key.der"
    PKCS_GENERATED = 'certs.p12'
    DEFAULT_HEADER_TEMPLATE = 'silabs_creds.tmpl'
    DEFAULT_HEADER_FILE = 'silabs_creds.h'
    DEFAULT_CERT_TOOL = 'out/tools/chip-cert'

    def __init__(self, paths, args) -> None:
        self.paths = paths
        self.args = args
        self.version = args.str(ID.kVersion)
        vid = args.hex(ID.kVendorId)
        pid = args.hex(ID.kProductId)
        tool_path = args.str(ID.kCertToolPath) or paths.root(Credentials.DEFAULT_CERT_TOOL)
        self.cert_tool = _tools.CertTool(tool_path, vid, pid)
        self.generate = args.bool(ID.kGenerateCreds)
        self.csr_mode = args.bool(ID.kCsrMode)


    def collect(self):
        # Certification Declaration
        cdc = self.args.get(ID.kCdCert)
        cdk = self.args.get(ID.kCdKey)
        cd = self.args.get(ID.kCertification)
        self.collectCD(cdc, cdk, cd)

        # Certificates
        pkcs12 = self.args.get(ID.kPKCS12)
        paa_cert = self.args.get(ID.kPaaCert)
        paa_key = self.args.get(ID.kPaaKey)
        pai_cert = self.args.get(ID.kPaiCert)
        pai_key = self.args.get(ID.kPaiKey)
        dac_cert = self.args.get(ID.kDacCert)
        dac_key = self.args.get(ID.kDacKey)

        if pkcs12.value is None:
            # Collect individual certificates
            self.collectCertificates(paa_cert, paa_key, pai_cert, pai_key, dac_cert, dac_key)
        else:
            # Collect PKCS#12 bundle
            self.collectPKCS12(pkcs12, pai_cert, dac_cert, dac_key)

        #Calculate offsets sizes here and generate the silabs_cred.h header file

        self.generateLegacyHeader(dac_cert, pai_cert, cd)


    def generateLegacyHeader(self, dac_cert, pai_cert, cd):
        # Calculate offsets
        dac_stats = os.stat(dac_cert.str())
        pai_stats = os.stat(pai_cert.str())
        cd_stats = os.stat(cd.str())
        use_legacy_format = ('1.0' == self.version)
        if use_legacy_format:
            # print("Using old credentials format\n")
            dac_offset = 0
            pai_offset = _util.roundNearest(dac_offset + dac_stats.st_size, 64)
            cd_offset = _util.roundNearest(pai_offset + pai_stats.st_size, 64)
            end_offset = _util.roundNearest(cd_offset + cd_stats.st_size, 1024)
        else:
            # print("Using new credentials format\n")
            pai_offset =  0x0200
            dac_offset  = 0x0000
            cd_offset =   0x0400

        # Generate header
        header = _util.File(self.paths.base(Credentials.DEFAULT_HEADER_TEMPLATE)).read()
        header = header.replace('{{use_legacy_certs_format}}', use_legacy_format and '1' or '0')
        header = header.replace('{{dac_offset}}', "0x{:04x}".format(dac_offset))
        header = header.replace('{{pai_offset}}', "0x{:04x}".format(pai_offset))
        header = header.replace('{{cd_offset}}',  "0x{:04x}".format(cd_offset))
        header = header.replace('{{dac_size}}',   str(dac_stats.st_size))
        header = header.replace('{{pai_size}}',   str(pai_stats.st_size))
        header = header.replace('{{cd_size}}',    str(cd_stats.st_size))
        # Write header
        _util.File(self.paths.temp(Credentials.DEFAULT_HEADER_FILE)).write(header)


    def collectCD(self, cdc, cdk, cd):
        cd_temp = self.paths.temp('cd.bin')
        if cd.value is not None:
            # Copy CD
            self.copy(cd.value, cd_temp)
        elif self.generate:
            # Signing Certificate
            if cdc.value is None:
                cdc.set(self.paths.root(Credentials.DEFAULT_CD_CERT))
            # Signing Key
            if cdk.value is None:
                cdk.set(self.paths.root(Credentials.DEFAULT_CD_KEY))
            # Generate CD
            cd.set(cd_temp, None, False)
            self.cert_tool.generateCD(cdc.value, cdk.value, cd.value)
        else:
            # Missing CD
            _util.fail("Missing: Certification Declaration")


    def collectCertificates(self, paa_cert, paa_key, pai_cert, pai_key, dac_cert, dac_key):

        cd_gen = dac_gen = pai_gen = False
        if not self.generate:
            pass
        elif self.csr_mode:
                dac_gen = False
                pai_gen = (pai_cert.value is None) or (pai_key.value is None)
        else:
            dac_gen = (dac_cert.value is None) or (dac_key.value is None)
            pai_gen = (pai_cert.value is None) or ((dac_gen or self.csr_mode) and (pai_key.value is None))
            dac_gen = dac_gen or pai_gen

        # Collect certificates
        self.collectPAA(paa_cert, paa_key)
        paic_pem, paik_pem, _, _  = self.collectPAI(paa_cert, paa_key, pai_cert, pai_key, pai_gen)
        dacc_pem, dack_pem, _, _ = self.collectDAC(pai_cert, pai_key, dac_cert, dac_key, dac_gen)

        # Generate PKCS#12 file
        if pai_gen and dac_gen:
            key_pass = self.args.str(ID.kKeyPass) or ''
            pass_arg = "pass:\"{}\"".format(key_pass)
            out_arg = self.paths.temp(Credentials.PKCS_GENERATED)
            _util.execute([ 'openssl', 'pkcs12', '-export', '' '-inkey', dack_pem, '-in', dacc_pem, '-certfile', paic_pem, '-out', out_arg, '-password', pass_arg ])


    def collectPAA(self, paa_cert, paa_key):
        # The PAA is never generated, either use the provided certificate or default
        if (paa_cert.value is None) or (paa_key.value is None):
            # Either PAA or PAA key missing, use defaults
            if paa_cert.value is not None:
                _util.warn("PAA certificate ignored: {}".format(paa_cert.value))
            if paa_key.value is not None:
                _util.warn("PAA key ignored: {}".format(paa_key.value))
        else:
            # Use provided PAA and PAA key
            if paa_cert.value is None:
                _util.fail("Missing: PAA certificate")
            if paa_key.value is None:
                _util.fail("Missing: PAA key")
            cert_pem, cert_der = self.x509Copy(paa_cert.value, 'paa_cert')
            key_pem, key_der = self.x509Copy(paa_key.value, 'paa_key', True)

    def collectPAI(self, paa_cert, paa_key, pai_cert, pai_key, generate):
        if generate:
            if (paa_cert.value is None) or (paa_key.value is None):
                _util.warn("Using default PAA certificate and key")
                paa_cert.set(self.paths.root(Credentials.DEFAULT_PAA_CERT))
                paa_key.set(self.paths.root(Credentials.DEFAULT_PAA_KEY))
            if pai_cert.value is not None:
                _util.warn("PAI certificate ignored: {}".format(pai_cert.value))
            if pai_key.value is not None:
                _util.warn("PAI key ignored: {}".format(pai_key.value))
            # Generate PAI
            pai_cert.set(self.paths.temp('pai_cert.pem'), None, False)
            pai_key.set(self.paths.temp('pai_key.pem'), None, False)
            self.cert_tool.generatePAI(paa_cert.value, paa_key.value, pai_cert.value, pai_key.value)
        else:
            # No PAA needed
            if paa_cert.value is not None:
                _util.warn("PAA certificate ignored: {}".format(paa_cert.value))
            if paa_key.value is not None:
                _util.warn("PAA key ignored: {}".format(paa_key.value))

        # Copy into DER format (if needed)
        paic_pem = paik_pem = paic_der = paik_der = None
        if pai_cert.value is None:
            _util.fail("Missing PAI certificate")
        paic_pem, paic_der = self.x509Copy(pai_cert.value, 'pai_cert')
        pai_cert.set(paic_der)
        if pai_key.value is not None:
            paik_pem, paik_der = self.x509Copy(pai_key.value, 'pai_key', True)
            pai_key.set(paik_der)
        return paic_pem, paik_pem, paic_der, paik_der


    def collectDAC(self, pai_cert, pai_key, dac_cert, dac_key, generate):
        if generate:
            if dac_cert.value is not None:
                _util.warn("DAC certificate ignored: {}".format(dac_cert.value))
            if dac_key.value is not None:
                _util.warn("DAC key ignored: {}".format(dac_key.value))
            # Generate DAC
            dac_cert.set(self.paths.temp('dac_cert.pem'), None, False)
            dac_key.set(self.paths.temp('dac_key.pem'), None, False)
            cn_arg = self.args.get(ID.kCommonName)
            self.cert_tool.generateDAC(pai_cert.value, pai_key.value, dac_cert.value, dac_key.value, cn_arg.str())
        else:
            # No PAI key needed
            if pai_key.value is not None:
                _util.warn("PAI key ignored: {}".format(pai_key.value))
        # Copy into DER format (if needed)
        if self.csr_mode:
            return None, None, None, None
        if dac_cert.value is None:
            _util.fail("Missing DAC certificate")
        if dac_key.value is None:
            _util.fail("Missing DAC key")
        dacc_pem, dacc_der = self.x509Copy(dac_cert.value, 'dac_cert')
        dack_pem, dack_der = self.x509Copy(dac_key.value, 'dac_key', True)
        dac_cert.set(dacc_der)
        dac_key.set(dack_der)
        return dacc_pem, dack_pem, dacc_der, dack_der


    def collectPKCS12(self, pkcs12, pai_cert, dac_cert, dac_key):
        pkcs12_path = pkcs12.str()
        pkcs12_temp = self.paths.temp(Credentials.PKCS_GENERATED)
        dacc_temp = self.paths.temp('dac_cert.pem')
        dack_temp = self.paths.temp('dac_key.der')
        paic_temp = self.paths.temp('pai_cert.pem')
        key_pass = self.args.get(ID.kKeyPass).value or ''
        self.copy(pkcs12_path, pkcs12_temp)

        # Extract key from PKCS#12
        password_arg = "pass:{}".format(key_pass)
        ps = subprocess.Popen(('openssl', 'pkcs12', '-nodes', '-nocerts', '-in', pkcs12_path, '-passin', password_arg), stdout=subprocess.PIPE)
        subprocess.check_output(('openssl', 'ec', '-outform', 'der', '-out', dack_temp), stdin=ps.stdout)

        # Extract certificates from PKCS#12
        out = _util.execute([ 'openssl', 'pkcs12', '-nodes', '-nokeys', '-in', pkcs12_path, '-passin', password_arg ], True, True)

        # Parse certificates
        # print("\n~~~~~~~~\n{}\n~~~~~~~~\n".format(s))
        certs = self.parsePKCSCerts(out.decode("utf-8"))
        _util.File(dacc_temp).write(certs[0])
        _util.File(paic_temp).write(certs[1])

        # Convert to DER
        paic_pem, paic_der = self.x509Copy(paic_temp, 'pai_cert')
        dacc_pem, dacc_der = self.x509Copy(dacc_temp, 'dac_cert')
        pai_cert.set(paic_der)
        dac_cert.set(dacc_der)
        dac_key.set(dack_temp)


    def copy(self, src, dest):
        if (not os.path.exists(dest)) or (not os.path.samefile(src, dest)):
            shutil.copy(src, dest)


    def x509Copy(self, in_path, out_name, is_key = False):
        if in_path is None:
            _util.fail("Missing X509 path: {}".format(out_name))
        (in_dir, in_base) = os.path.split(in_path)
        (in_root, in_ext) = os.path.splitext(in_base)
        out_path = self.paths.temp("{}{}".format(out_name, in_ext))
        self.copy(in_path, out_path)
        return self.x509Translate(out_path, is_key)


    def x509Translate(self, in_path, is_key = False):
        (in_base, in_ext) = os.path.splitext(in_path)
        in_ext = in_ext.strip('.')
        if 'der' == in_ext:
            out_ext = 'pem'
            der_path = in_path
            pem_path = out_path = "{}.{}".format(in_base, out_ext)
        elif 'pem' == in_ext:
            out_ext = 'der'
            pem_path = in_path
            der_path = out_path = "{}.{}".format(in_base, out_ext)
        else:
            _util.fail("Invalid certificate extension: \"{}\"".format(in_ext))
        if is_key:
            _util.execute(['openssl', 'ec', '-inform', in_ext, '-in', in_path, '-outform', out_ext, '-out', out_path])
        else:
            _util.execute(['openssl', 'x509', '-inform', in_ext, '-outform', out_ext, '-in', in_path, '-out', out_path])
        return pem_path, der_path


    def parsePKCSCerts(self, certs):
        BEGIN = '-----BEGIN CERTIFICATE-----'
        END = '-----END CERTIFICATE-----'
        offset = 0
        size = len(certs)
        cert_list = []
        while offset < size:
            b = certs.find(BEGIN, offset)
            if b < 0:
                break;
            e = certs.find(END, b)
            assert(e > 0)
            e += len(END)
            cert_list.append(certs[b:e])
            offset = e + 1
        return cert_list
