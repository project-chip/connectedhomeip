import ctypes
import glob
import os
import platform
import construct
import chip.exceptions
import typing
import enum

NATIVE_LIBRARY_BASE_NAME = "_ChipDeviceCtrl.so"


def _AllDirsToRoot(dir):
    """Return all parent paths of a directory."""
    dir = os.path.abspath(dir)
    while True:
        yield dir
        parent = os.path.dirname(dir)
        if parent == "" or parent == dir:
            break
        dir = parent


class ErrorRange(enum.IntEnum):
    ''' The enum of chip::ChipError::Range
    '''
    SDK = 0x0
    OS = 0x1
    POSIX = 0x2
    LWIP = 0x3
    OPENTHREAD = 0x4
    PLATFROM = 0x5


class ErrorSDKPart(enum.IntEnum):
    ''' The enum of chip::ChipError::SDKPart
    '''
    CORE = 0
    INET = 1
    DEVICE = 2
    ASN1 = 3
    BLE = 4
    IM_GLOBAL_STATUS = 5
    IM_CLUSTER_STATUS = 6
    APPLICATION = 7


class PyChipError(ctypes.Structure):
    ''' The ChipError for Python library.

    We are using the following struct for passing the infomations of CHIP_ERROR between C++ and Python:

    ```c
    struct PyChipError
    {
        uint32_t mCode;
        uint32_t mLine;
        const char * mFile;
    };
    ```
    '''
    _fields_ = [('code', ctypes.c_uint32), ('line', ctypes.c_uint32), ('file', ctypes.c_void_p)]

    def raise_on_error(self) -> None:
        if self.code != 0:
            raise self.to_exception()

    @property
    def is_success(self) -> bool:
        return self.code == 0

    @property
    def is_sdk_error(self) -> bool:
        return self.range == ErrorRange.SDK

    @property
    def range(self) -> ErrorRange:
        return ErrorRange((self.code >> 24) & 0xFF)

    @property
    def value(self) -> int:
        return (self.code) & 0xFFFFFF

    @property
    def sdk_part(self) -> ErrorSDKPart:
        if not self.is_sdk_error:
            return None
        return ErrorSDKPart((self.code >> 8) & 0x07)

    @property
    def sdk_code(self) -> int:
        if not self.is_sdk_error:
            return None
        return self.code & 0xFF

    def to_exception(self) -> typing.Union[None, chip.exceptions.ChipStackError]:
        if not self.is_success:
            return chip.exceptions.ChipStackError(self.code, str(self))

    def __str__(self):
        buf = ctypes.create_string_buffer(256)
        GetLibraryHandle().pychip_FormatError(ctypes.pointer(self), buf, 256)
        return buf.value.decode()

    def __eq__(self, other):
        if isinstance(other, int):
            return self.code == other
        if isinstance(other, PyChipError):
            return self.code == other.code
        if isinstance(other, chip.exceptions.ChipStackError):
            return self.code == other.err
        raise ValueError(f"Cannot compare PyChipError with {type(other)}")

    def __ne__(self, other):
        return not self == other


def FindNativeLibraryPath() -> str:
    """Find the native CHIP dll/so path."""

    scriptDir = os.path.dirname(os.path.abspath(__file__))

    # When properly installed in the chip package, the Chip Device Manager DLL will
    # be located in the package root directory, along side the package's
    # modules.
    dmDLLPath = os.path.join(
        os.path.dirname(scriptDir),  # file should be inside 'chip'
        NATIVE_LIBRARY_BASE_NAME)
    if os.path.exists(dmDLLPath):
        return dmDLLPath

    # For the convenience of developers, search the list of parent paths relative to the
    # running script looking for an CHIP build directory containing the Chip Device
    # Manager DLL. This makes it possible to import and use the ChipDeviceMgr module
    # directly from a built copy of the CHIP source tree.
    buildMachineGlob = "%s-*-%s*" % (platform.machine(),
                                     platform.system().lower())
    relDMDLLPathGlob = os.path.join(
        "build",
        buildMachineGlob,
        "src/controller/python/.libs",
        NATIVE_LIBRARY_BASE_NAME,
    )
    for dir in _AllDirsToRoot(scriptDir):
        dmDLLPathGlob = os.path.join(dir, relDMDLLPathGlob)
        for dmDLLPath in glob.glob(dmDLLPathGlob):
            if os.path.exists(dmDLLPath):
                return dmDLLPath

    raise Exception(
        "Unable to locate Chip Device Manager DLL (%s); expected location: %s" %
        (NATIVE_LIBRARY_BASE_NAME, scriptDir))


class NativeLibraryHandleMethodArguments:
    """Convenience wrapper to set native method argtype and restype for methods."""

    def __init__(self, handle):
        self.handle = handle

    def Set(self, methodName: str, resultType, argumentTypes: list):
        method = getattr(self.handle, methodName)
        method.restype = resultType
        method.argtype = argumentTypes


_nativeLibraryHandle: ctypes.CDLL = None


def _GetLibraryHandle(shouldInit: bool) -> ctypes.CDLL:
    """Get a memoized handle to the chip native code dll."""

    global _nativeLibraryHandle
    if _nativeLibraryHandle is None:
        if shouldInit:
            raise Exception("Common stack has not been initialized!")
        _nativeLibraryHandle = ctypes.CDLL(FindNativeLibraryPath())
        setter = NativeLibraryHandleMethodArguments(_nativeLibraryHandle)
        setter.Set("pychip_CommonStackInit", PyChipError, [ctypes.c_char_p])
        setter.Set("pychip_FormatError", None, [ctypes.POINTER(PyChipError), ctypes.c_char_p, ctypes.c_uint32])

    return _nativeLibraryHandle


def Init(bluetoothAdapter: int = None):
    CommonStackParams = construct.Struct(
        "BluetoothAdapterId" / construct.Int32ul,
    )
    params = CommonStackParams.parse(b'\x00' * CommonStackParams.sizeof())
    params.BluetoothAdapterId = bluetoothAdapter if bluetoothAdapter is not None else 0
    params = CommonStackParams.build(params)

    _GetLibraryHandle(False).pychip_CommonStackInit(ctypes.c_char_p(params))


def GetLibraryHandle():
    return _GetLibraryHandle(True)
