import modules.util as _util
import modules.tools as _tools


class ProvisionDefaults:

    def __init__(self, args) -> None:
        self.args = args

    def getPartNumber(self):
        chan = self.args.channel().value
        comm = _tools.Commander(chan)
        info = comm.info()
        if info is not None:
            return info['Part Number'].lower()

