import chip.exceptions
import chip.native
from chip.ChipUtility import ChipUtility
import ctypes

from typing import List
from ctypes import c_bool, c_char, c_void_p, c_char_p, c_uint16, c_uint32, c_uint8, c_size_t, c_int, CFUNCTYPE, POINTER, py_object, pythonapi

PythonChipMdnsBrowseFinishedCallback_t = CFUNCTYPE(None, py_object, c_uint32)
PythonChipMdnsBrowseServiceFoundCallback_t = CFUNCTYPE(
    None, py_object, c_char_p, c_char_p, c_int, c_int)

PythonChipMdnsResolveFinishCallback_t = CFUNCTYPE(None, py_object, c_uint32)
PythonChipMdnsResolveTextEntryCallback_t = CFUNCTYPE(
    None, py_object, c_char_p, POINTER(c_char), c_size_t)
PythonChipMdnsResolveAddressCallback_t = CFUNCTYPE(
    None, py_object, c_char_p, c_uint16)


class DNSSDDeviceResolveSession:
    def __init__(self, name: str, serviceType: str, protocol: int, addressType: int):
        self.name = name
        self.serviceType = serviceType
        self.protocol = protocol
        self.addressType = addressType
        self.address = ""
        self.port = 0
        self.textFields = {}

    def ResolveFinished(self, error: int):
        print("Resolve Finished: {}".format(error))
        print("Resolve Finished: {}.{} => {}:{}".format(
            self.name, self.serviceType, self.address, self.port))
        print(self.textFields)
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))

    def AddressResolved(self, address: str, port: int):
        self.address = address
        self.port = port

    def TextEntryResolved(self, key: str, data: bytes):
        self.textFields[key] = data

    def StartResolve(self):
        print("Resolve Started: {}".format(self.name))
        ctypes.pythonapi.Py_IncRef(ctypes.py_object(self))
        res = _GetMDNSLibraryHandle().pychip_ChipMdnsResolve(self.name.encode("utf-8") + b'\0',
                                                             self.serviceType.encode("utf-8") + b'\0', self.protocol, self.addressType, ctypes.py_object(self))
        if res != 0:
            self.ResolveFinished(res)


class DNSSDDeviceDiscovery:
    def __init__(self):
        self.devices = []

    def BrowseFinished(self, error: int):
        print("Browse Finished: {}".format(error))
        ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))

    def BrowseServiceFound(self, name: str, serviceType: str, protocol: int, addressType: int):
        print("Found Service: name={} serviceType={} protocol={} addressType={}".format(
            name, serviceType, protocol, addressType))
        device = DNSSDDeviceResolveSession(
            name, serviceType, protocol, addressType)
        device.StartResolve()
        self.devices.append(device)

    def BrowseService(self, serviceType: str, protocol: int, addressType: int):
        print("Browse Service")
        ctypes.pythonapi.Py_IncRef(ctypes.py_object(self))
        res = _GetMDNSLibraryHandle().pychip_ChipMdnsBrowse(
            serviceType.encode("utf-8") + b'\0', protocol, addressType, ctypes.py_object(self))
        if res != 0:
            self.BrowseFinished(res)


@PythonChipMdnsBrowseFinishedCallback_t
def PythonChipMdnsBrowseFinishedCallback(closure, error: int):
    closure.BrowseFinished(error)


@PythonChipMdnsBrowseServiceFoundCallback_t
def PythonChipMdnsBrowseServiceFoundCallback(closure, name: bytes, serviceType: str, protocol: int, addressType: int):
    closure.BrowseServiceFound(name.decode(
        "utf-8"), serviceType.decode("utf-8"), protocol, addressType)

# PythonChipMdnsResolveFinishCallback_t = CFUNCTYPE(None, py_object, CHIP_ERROR err)


@PythonChipMdnsResolveFinishCallback_t
def PythonChipMdnsResolveFinishCallback(closure, error: int):
    closure.ResolveFinished(error)

# PythonChipMdnsResolveTextEntryCallback_t = CFUNCTYPE(
#     None, py_object, c_char_p, POINTER(c_char), c_size_t)


@PythonChipMdnsResolveTextEntryCallback_t
def PythonChipMdnsResolveTextEntryCallback(closure, key: bytes, data: POINTER(c_char), size: int):
    val = bytes(ChipUtility.VoidPtrToByteArray(data, size))
    closure.TextEntryResolved(key.decode("utf-8"), val)

# PythonChipMdnsResolveAddressCallback_t = CFUNCTYPE(
#     None, py_object, c_char_p, c_uint16)


@PythonChipMdnsResolveAddressCallback_t
def PythonChipMdnsResolveAddressCallback(closure, address: bytes, port: int):
    closure.AddressResolved(address.decode("utf-8"), port)


def _GetMDNSLibraryHandle() -> ctypes.CDLL:
    handle = chip.native.GetLibraryHandle()

    try:
        # Uses one of the type decorators as an indicator for everything being
        # initialized. Native methods default to c_int return types
        if handle.pychip_ChipMdnsBrowse.restype != c_uint32:
            setter = chip.native.NativeLibraryHandleMethodArguments(handle)

            setter.Set('pychip_ChipMdnsBrowse', c_uint32, [
                    c_char_p, c_uint8, c_int, py_object])
            setter.Set('pychip_SetMdnsBrowseCallbacks', None, [
                    PythonChipMdnsBrowseFinishedCallback_t, PythonChipMdnsBrowseServiceFoundCallback_t])
            #    (const char * name, const char * type, uint8_t protocol, int addressType, PyObjectPtr context)
            setter.Set('pychip_SetMdnsResolveCallbacks', None, [
                    PythonChipMdnsResolveFinishCallback_t, PythonChipMdnsResolveTextEntryCallback_t, PythonChipMdnsResolveAddressCallback_t])
            setter.Set('pychip_ChipMdnsResolve', c_uint32, [
                c_char_p, c_char_p, c_uint8, c_int, py_object
            ])

            handle.pychip_SetMdnsBrowseCallbacks(
                PythonChipMdnsBrowseFinishedCallback, PythonChipMdnsBrowseServiceFoundCallback)
            handle.pychip_SetMdnsResolveCallbacks(
                PythonChipMdnsResolveFinishCallback, PythonChipMdnsResolveTextEntryCallback, PythonChipMdnsResolveAddressCallback)
    except:
        # Does not have internal mdns implementation with CHIP
        pass
    finally:
        return handle
