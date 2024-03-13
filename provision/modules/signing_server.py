import datetime
import modules.util as _util
import modules.arguments as _args

class SigningServer:

    def __init__(self, base_path, csr_path, pai_cert, pai_key, dac_cert):
        self.config_path = base_path + '/csa_openssl.cnf'
        self.csr_path = csr_path
        self.pai_cert_path = pai_cert
        self.pai_key_path = pai_key
        self.dac_cert_der = dac_cert

    def generateSerial(self):
        base_time = datetime.datetime(2000, 1, 1)
        delta = datetime.datetime.now() - base_time
        return delta.seconds

    def sign(self):
        serial_num = self.generateSerial()
        print("Sign:\n{}+ serial number:  {}".format(_util.MARGIN, serial_num))

        # Generate DAC
        _util.execute(['openssl', 'x509', '-sha256', '-req', '-days', '18250',
            '-extensions', 'v3_ica', '-extfile', self.config_path,
            '-set_serial', str(serial_num),
            '-CA', self.pai_cert_path, '-CAkey', self.pai_key_path, '-CAform', 'DER', '-CAkeyform', 'DER',
            '-in', self.csr_path, '-outform', 'der', '-out', self.dac_cert_der])

        return (self.pai_cert_path, self.dac_cert_der)
