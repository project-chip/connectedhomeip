import sys, platform

PY = sys.version_info[:2]
PY2 = sys.version_info[0] == 2
PY3 = sys.version_info[0] == 3
PYPY = '__pypy__' in sys.builtin_module_names
ONWINDOWS = platform.system() == "Windows"

stringtypes = (bytes, str, )
integertypes = (int, )
unicodestringtype = str
bytestringtype = bytes

INT2BYTE_CACHE = {i:bytes([i]) for i in range(256)}
def int2byte(character):
    """Converts (0 through 255) integer into b'...' character."""
    return INT2BYTE_CACHE[character]

def byte2int(character):
    """Converts b'...' character into (0 through 255) integer."""
    return character[0]

def str2bytes(string):
    """Converts '...' string into b'...' string. On PY2 they are equivalent. On PY3 its utf8 encoded."""
    return string.encode("utf8")

def bytes2str(string):
    """Converts b'...' string into '...' string. On PY2 they are equivalent. On PY3 its utf8 decoded."""
    return string.decode("utf8")

def reprstring(data):
    """Ensures there is b- u- prefix before the string."""
    if isinstance(data, bytes):
        return repr(data)
    if isinstance(data, str):
        return 'u' + repr(data)

def trimstring(data):
    """Trims b- u- prefix"""
    if isinstance(data, bytes):
        return repr(data)[1:]
    if isinstance(data, str):
        return repr(data)

def integers2bytes(ints):
    """Converts integer generator into bytes."""
    return bytes(ints)

def bytes2integers(data):
    """Converts bytes into integer list, so indexing/iterating yields integers."""
    return list(data)
