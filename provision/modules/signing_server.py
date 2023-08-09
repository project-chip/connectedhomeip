from .util import *


class SigningServer:

    def __init__(self, base_dir):
        self.base_dir = base_dir
        temp_dir = "{}/temp".format(base_dir)
        self.csr_pem = "{}/csr.pem".format(temp_dir)
        self.pai_cert_der = "{}/pai_cert.der".format(temp_dir)
        self.pai_key_der = "{}/pai_key.der".format(temp_dir)
        self.dac_cert_der = "{}/dac_cert.der".format(temp_dir)
        self.config_file = "{}/csa_openssl.cnf".format(base_dir)

    def sign(self, csr):

        serial_num = generateSerial()
        print("\n◆ Sign\n∙ serial number:  {}".format(serial_num))

        # Write CSR to file
        with open(self.csr_pem, 'w') as f:
            f.write(csr)

        # Generate DAC
        execute(['openssl', 'x509', '-sha256', '-req', '-days', '18250',
            '-extensions', 'v3_ica', '-extfile', self.config_file,
            '-set_serial', str(serial_num),
            '-CA', self.pai_cert_der, '-CAkey', self.pai_key_der, '-CAform', 'DER', '-CAkeyform', 'DER',
            '-in', self.csr_pem, '-outform', 'der', '-out', self.dac_cert_der])

        return (self.pai_cert_der, self.dac_cert_der)
