import ctypes
import glob
import os
import platform

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


def GetLibraryHandle() -> ctypes.CDLL:
    """Get a memoized handle to the chip native code dll."""

    global _nativeLibraryHandle
    if _nativeLibraryHandle is None:
        _nativeLibraryHandle = ctypes.CDLL(FindNativeLibraryPath())

        setter = NativeLibraryHandleMethodArguments(_nativeLibraryHandle)

        setter.Set("pychip_native_init", None, [])

        _nativeLibraryHandle.pychip_native_init()

    return _nativeLibraryHandle
