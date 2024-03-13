import modules.parameters as _params
import modules.formatter as _format


class ParameterList(_params.ParameterList):
    pass


class Formatter(_format.Formatter):

    def parse(self, main):
        super().parse(main)
        # Matter
        matter = ('matter' in main) and main['matter'] or None
        # Instance Info
        info = matter and ('instance_info' in matter) and matter['instance_info'] or None
        self.extract(info, 'serial_number')
        self.extract(info, 'vendor_id')
        self.extract(info, 'vendor_name')
        self.extract(info, 'product_id')
        self.extract(info, 'product_name')
        self.extract(info, 'product_label')
        self.extract(info, 'product_url')
        self.extract(info, 'part_number')
        self.extract(info, 'hw_version')
        self.extract(info, 'hw_version_str')
        self.extract(info, 'manufacturing_date')
        self.extract(info, 'unique_id')
        # Commissionable Data
        data = matter and ('commissionable_data' in matter) and matter['commissionable_data'] or None
        self.extract(data, 'discriminator')
        self.extract(data, 'passcode', 'spake2p_passcode')
        self.extract(data, 'iterations', 'spake2p_iterations')
        self.extract(data, 'salt', 'spake2p_salt')
        self.extract(data, 'verifier', 'spake2p_verifier')
        self.extract(data, 'commissioning_flow')
        self.extract(data, 'rendezvous_flags')
        # Attestation Credentials
        creds = matter and ('attestation_credentials' in matter) and matter['attestation_credentials'] or None
        self.extract(creds, 'pkcs12')
        self.extract(creds, 'key_id')
        self.extract(creds, 'key_pass')
        self.extract(creds, 'paa_cert')
        self.extract(creds, 'paa_key')
        self.extract(creds, 'pai_cert')
        self.extract(creds, 'pai_key')
        self.extract(creds, 'dac_cert')
        self.extract(creds, 'dac_key')
        self.extract(creds, 'certification')
        self.extract(creds, 'common_name')

    def format(self, main = {}):
        main = super().format(main)
        # Matter
        matter = main['matter']
        # Instance Info
        info = {}
        self.insert(info, 'serial_number')
        self.insert(info, 'vendor_id')
        self.insert(info, 'vendor_name')
        self.insert(info, 'product_id')
        self.insert(info, 'product_name')
        self.insert(info, 'product_label')
        self.insert(info, 'product_url')
        self.insert(info, 'part_number')
        self.insert(info, 'hw_version')
        self.insert(info, 'hw_version_str')
        self.insert(info, 'manufacturing_date')
        self.insert(info, 'unique_id')
        if len(info) > 0: matter['instance_info'] = info
        # Commissionable Data
        data = {}
        self.insert(data, 'discriminator')
        self.insert(data, 'spake2p_passcode', 'passcode')
        self.insert(data, 'spake2p_iterations', 'iterations')
        self.insert(data, 'spake2p_salt', 'salt')
        self.insert(data, 'spake2p_verifier', 'verifier')
        self.insert(data, 'commissioning_flow')
        self.insert(data, 'rendezvous_flags')
        if len(data) > 0: matter['commissionable_data'] = data
        # Attestation Credentials
        creds = {}
        self.insert(creds, 'pkcs12')
        self.insert(creds, 'key_id')
        self.insert(creds, 'key_pass')
        self.insert(creds, 'paa_cert')
        self.insert(creds, 'paa_key')
        self.insert(creds, 'pai_cert')
        self.insert(creds, 'pai_key')
        self.insert(creds, 'dac_cert')
        self.insert(creds, 'dac_key')
        self.insert(creds, 'certification')
        self.insert(creds, 'common_name')
        if len(creds) > 0: matter['attestation_credentials'] = creds
        return main
