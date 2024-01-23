class DiscoveryNotPerformedError(Exception):
    """Exception raised when MDNS discovery has not been performed."""

    def __init__(self, message="MDNS Discovery has not been performed. Ensure to call and await the `discover` method on this instance first."):
        self.message = message
        super().__init__(self.message)
