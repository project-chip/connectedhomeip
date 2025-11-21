from construct.lib.py3compat import *
import re
import collections
import sys

OrderedDict = dict
if sys.version_info < (3, 7):
    OrderedDict = collections.OrderedDict


globalPrintFullStrings = False
globalPrintFalseFlags = False
globalPrintPrivateEntries = False


def setGlobalPrintFullStrings(enabled=False):
    r"""
    When enabled, Container __str__ produces full content of bytes and unicode strings, otherwise and by default, it produces truncated output (16 bytes and 32 characters).

    :param enabled: bool
    """
    global globalPrintFullStrings
    globalPrintFullStrings = enabled


def setGlobalPrintFalseFlags(enabled=False):
    r"""
    When enabled, Container __str__ that was produced by FlagsEnum parsing prints all values, otherwise and by default, it prints only the values that are True.

    :param enabled: bool
    """
    global globalPrintFalseFlags
    globalPrintFalseFlags = enabled


def setGlobalPrintPrivateEntries(enabled=False):
    r"""
    When enabled, Container __str__ shows keys like _ _index _etc, otherwise and by default, it hides those keys. __repr__ never shows private entries.

    :param enabled: bool
    """
    global globalPrintPrivateEntries
    globalPrintPrivateEntries = enabled


def recursion_lock(retval="<recursion detected>", lock_name="__recursion_lock__"):
    """Used internally."""
    def decorator(func):
        def wrapper(self, *args, **kw):
            if getattr(self, lock_name, False):
                return retval
            setattr(self, lock_name, True)
            try:
                return func(self, *args, **kw)
            finally:
                delattr(self, lock_name)

        wrapper.__name__ = func.__name__
        return wrapper

    return decorator


class Container(OrderedDict):
    r"""
    Generic ordered dictionary that allows both key and attribute access, and preserves key order by insertion. Adding keys is preferred using \*\*entrieskw (requires Python 3.6). Equality does NOT check item order. Also provides regex searching.

    Note that not all parameters can be accessed via attribute access (dot operator). If the name of an item matches a method name of the Container, it can only be accessed via key acces (square brackets). This includes the following names: clear, copy, fromkeys, get, items, keys, move_to_end, pop, popitem, search, search_all, setdefault, update, values.

    Example::

        # empty dict
        >>> Container()
        # list of pairs, not recommended
        >>> Container([ ("name","anonymous"), ("age",21) ])
        # This syntax requires Python 3.6
        >>> Container(name="anonymous", age=21)
        # copies another dict
        >>> Container(dict2)
        >>> Container(container2)

    ::

        >>> print(repr(obj))
        Container(text='utf8 decoded string...', value=123)
        >>> print(obj)
        Container
            text = u'utf8 decoded string...' (total 22)
            value = 123
    """
    __slots__ = ["__recursion_lock__"]

    def __getattr__(self, name):
        try:
            if name in self.__slots__:
                return object.__getattribute__(self, name)
            else:
                return self[name]
        except KeyError:
            raise AttributeError(name)

    def __setattr__(self, name, value):
        try:
            if name in self.__slots__:
                return object.__setattr__(self, name, value)
            else:
                self[name] = value
        except KeyError:
            raise AttributeError(name)

    def __delattr__(self, name):
        try:
            if name in self.__slots__:
                return object.__delattr__(self, name)
            else:
                del self[name]
        except KeyError:
            raise AttributeError(name)

    def update(self, seqordict):
        """Appends items from another dict/Container or list-of-tuples."""
        if isinstance(seqordict, dict):
            seqordict = seqordict.items()
        for k,v in seqordict:
            self[k] = v

    def copy(self):
        return Container(self)

    __update__ = update

    __copy__ = copy

    def __dir__(self):
        """For auto completion of attributes based on container values."""
        return list(self.keys()) + list(self.__class__.__dict__) + dir(super(Container, self))

    def __eq__(self, other):
        if self is other:
            return True
        if not isinstance(other, dict):
            return False
        def isequal(v1, v2):
            if v1.__class__.__name__ == "ndarray" or v2.__class__.__name__ == "ndarray":
                import numpy
                return numpy.array_equal(v1, v2)
            return v1 == v2
        for k,v in self.items():
            if isinstance(k, unicodestringtype) and k.startswith(u"_"):
                continue
            if isinstance(k, bytestringtype) and k.startswith(b"_"):
                continue
            if k not in other or not isequal(v, other[k]):
                return False
        for k,v in other.items():
            if isinstance(k, unicodestringtype) and k.startswith(u"_"):
                continue
            if isinstance(k, bytestringtype) and k.startswith(b"_"):
                continue
            if k not in self or not isequal(v, self[k]):
                return False
        return True

    def __ne__(self, other):
       return not self == other

    @recursion_lock()
    def __repr__(self):
        parts = []
        for k,v in self.items():
            if isinstance(k, str) and k.startswith("_"):
                continue
            if isinstance(v, stringtypes):
                parts.append(str(k) + "=" + reprstring(v))
            else:
                parts.append(str(k) + "=" + repr(v))
        return "Container(%s)" % ", ".join(parts)

    @recursion_lock()
    def __str__(self):
        indentation = "\n    "
        text = ["Container: "]
        isflags = getattr(self, "_flagsenum", False)
        for k,v in self.items():
            if isinstance(k, str) and k.startswith("_") and not globalPrintPrivateEntries:
                continue
            if isflags and not v and not globalPrintFalseFlags:
                continue
            text.extend([indentation, str(k), " = "])
            if v.__class__.__name__ == "EnumInteger":
                text.append("(enum) (unknown) %s" % (v, ))
            elif v.__class__.__name__ == "EnumIntegerString":
                text.append("(enum) %s %s" % (v, v.intvalue, ))
            elif v.__class__.__name__ in ["HexDisplayedBytes", "HexDumpDisplayedBytes"]:
                text.append(indentation.join(str(v).split("\n")))
            elif isinstance(v, bytestringtype):
                printingcap = 16
                if len(v) <= printingcap or globalPrintFullStrings:
                    text.append("%s (total %d)" % (reprstring(v), len(v)))
                else:
                    text.append("%s... (truncated, total %d)" % (reprstring(v[:printingcap]), len(v)))
            elif isinstance(v, unicodestringtype):
                printingcap = 32
                if len(v) <= printingcap or globalPrintFullStrings:
                    text.append("%s (total %d)" % (reprstring(v), len(v)))
                else:
                    text.append("%s... (truncated, total %d)" % (reprstring(v[:printingcap]), len(v)))
            else:
                text.append(indentation.join(str(v).split("\n")))
        return "".join(text)

    def _search(self, compiled_pattern, search_all):
        items = []
        for key in self.keys():
            try:
                if isinstance(self[key], (Container,ListContainer)):
                    ret = self[key]._search(compiled_pattern, search_all)
                    if ret is not None:
                        if search_all:
                            items.extend(ret)
                        else:
                            return ret
                elif compiled_pattern.match(key):
                    if search_all:
                        items.append(self[key])
                    else:
                        return self[key]
            except:
                pass
        if search_all:
            return items
        else:
            return None

    def search(self, pattern):
        """
        Searches a container (non-recursively) using regex.
        """
        compiled_pattern = re.compile(pattern)
        return self._search(compiled_pattern, False)

    def search_all(self, pattern):
        """
        Searches a container (recursively) using regex.
        """
        compiled_pattern = re.compile(pattern)
        return self._search(compiled_pattern, True)

    def __getstate__(self):
        """
        Used by pickle to serialize an instance to a dict.
        """
        ret = OrderedDict(self)
        return ret

    def __setstate__(self, state):
        """
        Used by pickle to de-serialize from a dict.
        """
        self.clear()
        self.update(state)


class ListContainer(list):
    r"""
    Generic container like list. Provides pretty-printing. Also provides regex searching.

    Example::

        >>> ListContainer()
        >>> ListContainer([1, 2, 3])

    ::

        >>> print(repr(obj))
        [1, 2, 3]
        >>> print(obj)
        ListContainer
            1
            2
            3
    """

    @recursion_lock()
    def __repr__(self):
        return "ListContainer(%s)" % (list.__repr__(self), )

    @recursion_lock()
    def __str__(self):
        indentation = "\n    "
        text = ["ListContainer: "]
        for k in self:
            text.append(indentation)
            lines = str(k).split("\n")
            text.append(indentation.join(lines))
        return "".join(text)

    def _search(self, compiled_pattern, search_all):
        items = []
        for item in self:
            try:
                ret = item._search(compiled_pattern, search_all)
            except:
                continue
            if ret is not None:
                if search_all:
                    items.extend(ret)
                else:
                    return ret
        if search_all:
            return items
        else:
            return None

    def search(self, pattern):
        """
        Searches a container (non-recursively) using regex.
        """
        compiled_pattern = re.compile(pattern)
        return self._search(compiled_pattern, False)

    def search_all(self, pattern):
        """
        Searches a container (recursively) using regex.
        """
        compiled_pattern = re.compile(pattern)
        return self._search(compiled_pattern, True)
