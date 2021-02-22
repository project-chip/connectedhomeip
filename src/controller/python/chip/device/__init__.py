import chip.exceptions

__all__ = ["Device"]

validDeviceAttrs = {
    "paired": bool,
    "nodeid": int,
    "name": str,

    "address": str,
    "port": int,

    "discriminator": int,
    "vendor": int,
    "product": int,
}

class DeviceDescriptor:
    def __init__(self, network: str, **kwargs):
        if network not in ["ble", "ip", "any"]:
            raise chip.exceptions.InvalidArgument("network", network)
        self.network = network

        for key, value in kwargs.items():
            expectedType = validDeviceAttrs.get(key, None)
            if not expectedType:
                raise chip.exceptions.UnexpectedArgument(key, type(value))
            if type(value) != expectedType:
                raise chip.exceptions.InvalidArgumentType(expectedType, type(value), key)
            setattr(self, key, value)

    def __str__(self):
        return "Device({})".format(vars(self))

    def __repr__(self):
        return str(self)
