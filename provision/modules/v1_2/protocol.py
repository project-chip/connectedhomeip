import modules.v1_0.protocol as _base


class Protocol(_base.Protocol):

    def createSetupCommand(self, paths, args):
        return SetupCommand(paths, args)


class SetupCommand(_base.SetupCommand):

    def encode(self):
        self.addInt32u(self.vendor_id)
        self.addArray(self.vendor_name)
        self.addInt32u(self.product_id)
        self.addArray(self.product_name)
        self.addArray(self.product_label)
        self.addArray(self.product_url)
        self.addArray(self.part_number)
        self.addInt16u(self.hw_version)
        self.addArray(self.hw_version_str)
        self.addArray(self.manufacturing_date)
        self.addArray(self.unique_id)
        self.addInt32u(self.commissioning_flow)
        self.addInt32u(self.rendezvous_flags)
        self.addInt16u(self.discriminator)
        self.addString(self.spake2p_verifier)
        self.addInt32u(self.spake2p_iterations) # Changed in v1.2
        self.addString(self.spake2p_salt)
        self.addInt32u(self.passcode)