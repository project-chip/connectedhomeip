from threading import Lock, Event
from pybindings.PyChip import ChipExceptions 

class Caller:
    completeEvent = Event()
    networkLock = Lock()
    blockingCB = None

    def Call(self, callFunct):
        # throw error if op in progress
        callbackRes = None
        self.completeEvent.clear()
        with self.networkLock:
            res = callFunct()
        self.completeEvent.set()
        if res == 0 and callbackRes != None:
            return callbackRes
        return res

    def CallAsync(self, callFunct):
        # throw error if op in progress
        callbackRes = None
        self.completeEvent.clear()
        with self.networkLock:
            res = callFunct()

        if res != 0:
            self.completeEvent.set()
            raise ChipExceptions.CHIPErrorToException(res)
        while not self.completeEvent.isSet():
            if self.blockingCB:
                self.blockingCB()

            self.completeEvent.wait(0.05)
        return callbackRes
