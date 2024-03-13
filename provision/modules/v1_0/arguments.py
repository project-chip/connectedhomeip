import modules.parameters as _params
import modules.formatter as _format


class ParameterList(_params.ParameterList):

    PARAMS_PATH = 'modules/v1_0/parameters.yaml'

    def __init__(self, paths, custom_path = None) -> None:
        super().__init__(paths, custom_path)
        self.load(paths.base(ParameterList.PARAMS_PATH))


class Formatter(_format.Formatter):

    def parse(self, main):
        super().parse(main)
        # Matter
        matter = ('matter' in main) and main['matter'] or None
        self.extract(matter, 'generate')
        self.extract(matter, 'serial_number')
        self.extract(matter, 'vendor_id')
        self.extract(matter, 'vendor_name')
        self.extract(matter, 'product_id')
        self.extract(matter, 'product_name')
        self.extract(matter, 'product_label')
        self.extract(matter, 'product_url')
        self.extract(matter, 'part_number')
        self.extract(matter, 'hw_version')
        self.extract(matter, 'hw_version_str')
        self.extract(matter, 'manufacturing_date')
        self.extract(matter, 'unique_id')
        self.extract(matter, 'commissioning_flow')
        self.extract(matter, 'rendezvous_flags')
        self.extract(matter, 'discriminator')
        # SPAKE2+
        spake = matter and ('spake2p' in matter) and matter['spake2p'] or None
        self.extract(spake, 'passcode', 'spake2p_passcode')
        self.extract(spake, 'iterations', 'spake2p_iterations')
        self.extract(spake, 'salt', 'spake2p_salt')
        self.extract(spake, 'verifier', 'spake2p_verifier')
        # Attestation
        attest = matter and ('attestation' in matter) and matter['attestation'] or None
        self.extract(attest, 'pkcs12')
        self.extract(attest, 'key_id')
        self.extract(attest, 'key_pass')
        self.extract(attest, 'paa_cert')
        self.extract(attest, 'paa_key')
        self.extract(attest, 'pai_cert')
        self.extract(attest, 'pai_key')
        self.extract(attest, 'dac_cert')
        self.extract(attest, 'dac_key')
        self.extract(attest, 'certification')
        self.extract(attest, 'common_name')

    def format(self, main = {}):
        main = super().format(main)
        # Matter
        matter = main['matter']
        self.insert(matter, 'generate')
        self.insert(matter, 'serial_number')
        self.insert(matter, 'vendor_id')
        self.insert(matter, 'vendor_name')
        self.insert(matter, 'product_id')
        self.insert(matter, 'product_name')
        self.insert(matter, 'product_label')
        self.insert(matter, 'product_url')
        self.insert(matter, 'part_number')
        self.insert(matter, 'hw_version')
        self.insert(matter, 'hw_version_str')
        self.insert(matter, 'unique_id')
        self.insert(matter, 'commissioning_flow')
        self.insert(matter, 'rendezvous_flags')
        self.insert(matter, 'discriminator')
        # SPAKE2+
        spake = {}
        self.insert(spake, 'spake2p_passcode', 'passcode')
        self.insert(spake, 'spake2p_iterations', 'iterations')
        self.insert(spake, 'spake2p_salt', 'salt')
        self.insert(spake, 'spake2p_verifier', 'verifier')
        if len(spake) > 0: matter['spake2p'] = spake
        # Attestation
        attest = {}
        self.insert(attest, 'pkcs12')
        self.insert(attest, 'key_id')
        self.insert(attest, 'key_pass')
        self.insert(attest, 'paa_cert')
        self.insert(attest, 'paa_key')
        self.insert(attest, 'pai_cert')
        self.insert(attest, 'pai_key')
        self.insert(attest, 'dac_cert')
        self.insert(attest, 'dac_key')
        self.insert(attest, 'certification')
        self.insert(attest, 'common_name')
        if len(attest) > 0: matter['attestation'] = attest
        return main
