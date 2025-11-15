# -*- coding: utf-8 -*-

import struct, io, binascii, itertools, collections, pickle, sys, os, hashlib, importlib, importlib.machinery, importlib.util

from construct.lib import *
from construct.expr import *
from construct.version import *


#===============================================================================
# exceptions
#===============================================================================
class ConstructError(Exception):
    """
    This is the root of all exceptions raised by parsing classes in this library. Note that the helper functions in lib module can raise standard ValueError (but parsing classes are not allowed to).
    """
    def __init__(self, message='', path=None):
        self.path = path
        if path is None:
            super().__init__(message)
        else:
            message = "Error in path {}\n".format(path) + message
            super().__init__(message)
class SizeofError(ConstructError):
    """
    Parsing classes sizeof() methods are only allowed to either return an integer or raise SizeofError instead. Note that this exception can mean the parsing class cannot be measured apriori in principle, however it can also mean that it just cannot be measured in these particular circumstances (eg. there is a key missing in the context dictionary at this time).
    """
    pass
class AdaptationError(ConstructError):
    """
    Currently not used.
    """
    pass
class ValidationError(ConstructError):
    """
    Validator ExprValidator derived parsing classes can raise this exception: OneOf NoneOf. It can mean that the parse or build value is or is not one of specified values.
    """
    pass
class StreamError(ConstructError):
    """
    Almost all parsing classes can raise this exception: it can mean a variety of things. Maybe requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, could not write all bytes, stream is not seekable, stream is not tellable, etc. Note that there are a few parsing classes that do not use the stream to compute output and therefore do not raise this exception.
    """
    pass
class FormatFieldError(ConstructError):
    """
    Only one parsing class can raise this exception: FormatField. It can either mean the format string is invalid or the value is not valid for provided format string. See standard struct module for what is acceptable.
    """
    pass
class IntegerError(ConstructError):
    """
    Only some numeric parsing classes can raise this exception: BytesInteger BitsInteger VarInt ZigZag. It can mean either the length parameter is invalid, the value is not an integer, the value is negative or too low or too high for given parameters, or the selected endianness cannot be applied.
    """
    pass
class StringError(ConstructError):
    """
    Almost all parsing classes can raise this exception: It can mean a unicode string was passed instead of bytes, or a bytes was passed instead of a unicode string. Also some classes can raise it explicitly: PascalString CString GreedyString. It can mean no encoding or invalid encoding was selected. Note that currently, if the data cannot be encoded decoded given selected encoding then UnicodeEncodeError UnicodeDecodeError are raised, which are not rooted at ConstructError.
    """
    pass
class MappingError(ConstructError):
    """
    Few parsing classes can raise this exception: Enum FlagsEnum Mapping. It can mean the build value is not recognized and therefore cannot be mapped onto bytes.
    """
    pass
class RangeError(ConstructError):
    """
    Few parsing classes can raise this exception: Array PrefixedArray LazyArray. It can mean the count parameter is invalid, or the build object has too little or too many elements.
    """
    pass
class RepeatError(ConstructError):
    """
    Only one parsing class can raise this exception: RepeatUntil. It can mean none of the elements in build object passed the given predicate.
    """
    pass
class ConstError(ConstructError):
    """
    Only one parsing class can raise this exception: Const. It can mean the wrong data was parsed, or wrong object was built from.
    """
    pass
class IndexFieldError(ConstructError):
    """
    Only one parsing class can raise this exception: Index. It can mean the class was not nested in an array parsing class properly and therefore cannot access the _index context key.
    """
    pass
class CheckError(ConstructError):
    """
    Only one parsing class can raise this exception: Check. It can mean the condition lambda failed during a routine parsing building check.
    """
    pass
class ExplicitError(ConstructError):
    """
    Only one parsing class can raise this exception: Error. It can mean the parsing class was merely parsed or built with.
    """
    pass
class NamedTupleError(ConstructError):
    """
    Only one parsing class can raise this exception: NamedTuple. It can mean the subcon is not of a valid type.
    """
    pass
class TimestampError(ConstructError):
    """
    Only one parsing class can raise this exception: Timestamp. It can mean the subcon unit or epoch are invalid.
    """
    pass
class UnionError(ConstructError):
    """
    Only one parsing class can raise this exception: Union. It can mean none of given subcons was properly selected, or trying to build without providing a proper value.
    """
    pass
class SelectError(ConstructError):
    """
    Only one parsing class can raise this exception: Select. It can mean neither subcon succeded when parsing or building.
    """
    pass
class SwitchError(ConstructError):
    """
    Currently not used.
    """
    pass
class StopFieldError(ConstructError):
    """
    Only one parsing class can raise this exception: StopIf. It can mean the given condition was met during parsing or building.
    """
    pass
class PaddingError(ConstructError):
    """
    Multiple parsing classes can raise this exception: PaddedString Padding Padded Aligned FixedSized NullTerminated NullStripped. It can mean multiple issues: the encoded string or bytes takes more bytes than padding allows, length parameter was invalid, pattern terminator or pad is not a proper bytes value, modulus was less than 2.
    """
    pass
class TerminatedError(ConstructError):
    """
    Only one parsing class can raise this exception: Terminated. It can mean EOF was not found as expected during parsing.
    """
    pass
class RawCopyError(ConstructError):
    """
    Only one parsing class can raise this exception: RawCopy. It can mean it cannot build as both data and value keys are missing from build dict object.
    """
    pass
class RotationError(ConstructError):
    """
    Only one parsing class can raise this exception: ProcessRotateLeft. It can mean the specified group is less than 1, data is not of valid length.
    """
    pass
class ChecksumError(ConstructError):
    """
    Only one parsing class can raise this exception: Checksum. It can mean expected and actual checksum do not match.
    """
    pass
class CancelParsing(ConstructError):
    """
    This exception can only be raise explicitly by the user, and it causes the parsing class to stop what it is doing (interrupts parsing or building).
    """
    pass
class CipherError(ConstructError):
    """
    Two parsing classes can raise this exception: EncryptedSym EncryptedSymAead. It can mean none or invalid cipher object was provided.
    """
    pass



#===============================================================================
# used internally
#===============================================================================
def singleton(arg):
    x = arg()
    return x


def stream_read(stream, length, path):
    if length < 0:
        raise StreamError("length must be non-negative, found %s" % length, path=path)
    try:
        data = stream.read(length)
    except Exception:
        raise StreamError("stream.read() failed, requested %s bytes" % (length,), path=path)
    if len(data) != length:
        raise StreamError("stream read less than specified amount, expected %d, found %d" % (length, len(data)), path=path)
    return data


def stream_read_entire(stream, path):
    try:
        return stream.read()
    except Exception:
        raise StreamError("stream.read() failed when reading until EOF", path=path)


def stream_write(stream, data, length, path):
    if not isinstance(data, bytestringtype):
        raise StringError("given non-bytes value, perhaps unicode? %r" % (data,), path=path)
    if length < 0:
        raise StreamError("length must be non-negative, found %s" % length, path=path)
    if len(data) != length:
        raise StreamError("bytes object of wrong length, expected %d, found %d" % (length, len(data)), path=path)
    try:
        written = stream.write(data)
    except Exception:
        raise StreamError("stream.write() failed, given %r" % (data,), path=path)
    if written != length:
        raise StreamError("stream written less than specified, expected %d, written %d" % (length, written), path=path)


def stream_seek(stream, offset, whence, path):
    try:
        return stream.seek(offset, whence)
    except Exception:
        raise StreamError("stream.seek() failed, offset %s, whence %s" % (offset, whence), path=path)


def stream_tell(stream, path):
    try:
        return stream.tell()
    except Exception:
        raise StreamError("stream.tell() failed", path=path)


def stream_size(stream):
    try:
        fallback = stream.tell()
        end = stream.seek(0, 2)
        stream.seek(fallback)
        return end
    except Exception:
        raise StreamError("stream. seek() tell() failed", path="???")


def stream_iseof(stream):
    try:
        fallback = stream.tell()
        data = stream.read(1)
        stream.seek(fallback)
        return not data
    except Exception:
        raise StreamError("stream. read() seek() tell() failed", path="???")


class BytesIOWithOffsets(io.BytesIO):
    @staticmethod
    def from_reading(stream, length: int, path: str):
        offset = stream_tell(stream, path)
        contents = stream_read(stream, length, path)
        return BytesIOWithOffsets(contents, stream, offset)

    def __init__(self, contents: bytes, parent_stream, offset: int):
        super().__init__(contents)
        self.parent_stream = parent_stream
        self.parent_stream_offset = offset

    def tell(self) -> int:
        return super().tell() + self.parent_stream_offset

    def seek(self, offset: int, whence: int = io.SEEK_SET) -> int:
        if whence != io.SEEK_SET:
            super().seek(offset, whence)
        else:
            super().seek(offset - self.parent_stream_offset)
        return self.tell()


class CodeGen:
    def __init__(self):
        self.blocks = []
        self.nextid = 0
        self.parsercache = {}
        self.buildercache = {}
        self.linkedinstances = {}
        self.linkedparsers = {}
        self.linkedbuilders = {}

    def allocateId(self):
        self.nextid += 1
        return self.nextid

    def append(self, block):
        block = [s for s in block.splitlines() if s.strip()]
        firstline = block[0]
        trim = len(firstline) - len(firstline.lstrip())
        block = "\n".join(s[trim:] for s in block)
        if block not in self.blocks:
            self.blocks.append(block)

    def toString(self):
        return "\n".join(self.blocks + [""])


class KsyGen:
    def __init__(self):
        self.instances = {}
        self.enums = {}
        self.types = {}
        self.nextid = 0

    def allocateId(self):
        self.nextid += 1
        return self.nextid


def hyphenatedict(d):
    return {k.replace("_","-").rstrip("-"):v for k,v in d.items()}


def hyphenatelist(l):
    return [hyphenatedict(d) for d in l]


def extractfield(sc):
    if isinstance(sc, Renamed):
        return extractfield(sc.subcon)
    return sc


def evaluate(param, context):
    return param(context) if callable(param) else param


#===============================================================================
# abstract constructs
#===============================================================================
class Construct(object):
    r"""
    The mother of all constructs.

    This object is generally not directly instantiated, and it does not directly implement parsing and building, so it is largely only of interest to subclass implementors. There are also other abstract classes sitting on top of this one.

    The external user API:

    * `parse`
    * `parse_stream`
    * `parse_file`
    * `build`
    * `build_stream`
    * `build_file`
    * `sizeof`
    * `compile`
    * `benchmark`

    Subclass authors should not override the external methods. Instead, another API is available:

    * `_parse`
    * `_build`
    * `_sizeof`
    * `_actualsize`
    * `_emitparse`
    * `_emitbuild`
    * `_emitseq`
    * `_emitprimitivetype`
    * `_emitfulltype`
    * `__getstate__`
    * `__setstate__`

    Attributes and Inheritance:

    All constructs have a name and flags. The name is used for naming struct members and context dictionaries. Note that the name can be a string, or None by default. A single underscore "_" is a reserved name, used as up-level in nested containers. The name should be descriptive, short, and valid as a Python identifier, although these rules are not enforced. The flags specify additional behavioral information about this construct. Flags are used by enclosing constructs to determine a proper course of action. Flags are often inherited from inner subconstructs but that depends on each class.
    """

    def __init__(self):
        self.name = None
        self.docs = ""
        self.flagbuildnone = False
        self.parsed = None

    def __repr__(self):
        return "<%s%s%s%s>" % (self.__class__.__name__, " "+self.name if self.name else "", " +nonbuild" if self.flagbuildnone else "", " +docs" if self.docs else "", )

    def __getstate__(self):
        attrs = {}
        if hasattr(self, "__dict__"):
            attrs.update(self.__dict__)
        slots = []
        c = self.__class__
        while c is not None:
            if hasattr(c, "__slots__"):
                slots.extend(c.__slots__)
            c = c.__base__
        for name in slots:
            if hasattr(self, name):
                attrs[name] = getattr(self, name)
        return attrs

    def __setstate__(self, attrs):
        for name, value in attrs.items():
            setattr(self, name, value)

    def __copy__(self):
        self2 = object.__new__(self.__class__)
        self2.__setstate__(self.__getstate__())
        return self2

    def parse(self, data, **contextkw):
        r"""
        Parse an in-memory buffer (often bytes object). Strings, buffers, memoryviews, and other complete buffers can be parsed with this method.

        Whenever data cannot be read, ConstructError or its derivative is raised. This method is NOT ALLOWED to raise any other exceptions although (1) user-defined lambdas can raise arbitrary exceptions which are propagated (2) external libraries like numpy can raise arbitrary exceptions which are propagated (3) some list and dict lookups can raise IndexError and KeyError which are propagated.

        Context entries are passed only as keyword parameters \*\*contextkw.

        :param \*\*contextkw: context entries, usually empty

        :returns: some value, usually based on bytes read from the stream but sometimes it is computed from nothing or from the context dictionary, sometimes its non-deterministic

        :raises ConstructError: raised for any reason
        """
        return self.parse_stream(io.BytesIO(data), **contextkw)

    def parse_stream(self, stream, **contextkw):
        r"""
        Parse a stream. Files, pipes, sockets, and other streaming sources of data are handled by this method. See parse().
        """
        context = Container(**contextkw)
        context._parsing = True
        context._building = False
        context._sizing = False
        context._params = context
        try:
            return self._parsereport(stream, context, "(parsing)")
        except CancelParsing:
            pass

    def parse_file(self, filename, **contextkw):
        r"""
        Parse a closed binary file. See parse().
        """
        with open(filename, 'rb') as f:
            return self.parse_stream(f, **contextkw)

    def _parsereport(self, stream, context, path):
        obj = self._parse(stream, context, path)
        if self.parsed is not None:
            self.parsed(obj, context)
        return obj

    def _parse(self, stream, context, path):
        """Override in your subclass."""
        raise NotImplementedError

    def build(self, obj, **contextkw):
        r"""
        Build an object in memory (a bytes object).

        Whenever data cannot be written, ConstructError or its derivative is raised. This method is NOT ALLOWED to raise any other exceptions although (1) user-defined lambdas can raise arbitrary exceptions which are propagated (2) external libraries like numpy can raise arbitrary exceptions which are propagated (3) some list and dict lookups can raise IndexError and KeyError which are propagated.

        Context entries are passed only as keyword parameters \*\*contextkw.

        :param \*\*contextkw: context entries, usually empty

        :returns: bytes

        :raises ConstructError: raised for any reason
        """
        stream = io.BytesIO()
        self.build_stream(obj, stream, **contextkw)
        return stream.getvalue()

    def build_stream(self, obj, stream, **contextkw):
        r"""
        Build an object directly into a stream. See build().
        """
        context = Container(**contextkw)
        context._parsing = False
        context._building = True
        context._sizing = False
        context._params = context
        self._build(obj, stream, context, "(building)")

    def build_file(self, obj, filename, **contextkw):
        r"""
        Build an object into a closed binary file. See build().
        """
        # Open the file for reading as well as writing. This allows builders to
        # read back the stream just written. For example. RawCopy does this.
        # See issue #888.
        with open(filename, 'w+b') as f:
            self.build_stream(obj, f, **contextkw)

    def _build(self, obj, stream, context, path):
        """Override in your subclass."""
        raise NotImplementedError

    def sizeof(self, **contextkw):
        r"""
        Calculate the size of this object, optionally using a context.

        Some constructs have fixed size (like FormatField), some have variable-size and can determine their size given a context entry (like Bytes(this.otherfield1)), and some cannot determine their size (like VarInt).

        Whenever size cannot be determined, SizeofError is raised. This method is NOT ALLOWED to raise any other exception, even if eg. context dictionary is missing a key, or subcon propagates ConstructError-derivative exception.

        Context entries are passed only as keyword parameters \*\*contextkw.

        :param \*\*contextkw: context entries, usually empty

        :returns: integer if computable, SizeofError otherwise

        :raises SizeofError: size could not be determined in actual context, or is impossible to be determined
        """
        context = Container(**contextkw)
        context._parsing = False
        context._building = False
        context._sizing = True
        context._params = context
        return self._sizeof(context, "(sizeof)")

    def _sizeof(self, context, path):
        """Override in your subclass."""
        raise SizeofError(path=path)

    def _actualsize(self, stream, context, path):
        return self._sizeof(context, path)

    def compile(self, filename=None):
        """
        Transforms a construct into another construct that does same thing (has same parsing and building semantics) but is much faster when parsing. Already compiled instances just compile into itself.

        Optionally, partial source code can be saved to a text file. This is meant only to inspect the generated code, not to import it from external scripts.

        :returns: Compiled instance
        """

        code = CodeGen()
        code.append("""
            # generated by Construct, this source is for inspection only! do not import!

            from construct import *
            from construct.lib import *
            from io import BytesIO
            import struct
            import collections
            import itertools

            def restream(data, func):
                return func(BytesIO(data))
            def reuse(obj, func):
                return func(obj)

            linkedinstances = {}
            linkedparsers = {}
            linkedbuilders = {}

            len_ = len
            sum_ = sum
            min_ = min
            max_ = max
            abs_ = abs
        """)
        code.append(f"""
            def parseall(io, this):
                return {self._compileparse(code)}
            def buildall(obj, io, this):
                return {self._compilebuild(code)}
            compiled = Compiled(parseall, buildall)
        """)
        source = code.toString()

        if filename:
            with open(filename, "wt") as f:
                f.write(source)

        modulename = hexlify(hashlib.sha1(source.encode()).digest()).decode()
        module_spec = importlib.machinery.ModuleSpec(modulename, None)
        module = importlib.util.module_from_spec(module_spec)
        c = compile(source, '', 'exec')
        exec(c, module.__dict__)

        module.linkedinstances = code.linkedinstances
        module.linkedparsers = code.linkedparsers
        module.linkedbuilders = code.linkedbuilders
        compiled = module.compiled
        compiled.source = source
        compiled.module = module
        compiled.modulename = modulename
        compiled.defersubcon = self
        return compiled

    def _compileinstance(self, code):
        """Used internally."""
        if id(self) in code.linkedinstances:
            return
        code.append(f"""
            # linkedinstances[{id(self)}] is {self}
        """)
        field = extractfield(self)
        code.linkedinstances[id(self)] = field
        code.linkedparsers[id(self)] = field._parse
        code.linkedbuilders[id(self)] = field._build

    def _compileparse(self, code):
        """Used internally."""
        try:
            if id(self) in code.parsercache:
                return code.parsercache[id(self)]
            emitted = self._emitparse(code)
            code.parsercache[id(self)] = emitted
            return emitted
        except NotImplementedError:
            self._compileinstance(code)
            return f"linkedparsers[{id(self)}](io, this, '(???)')"

    def _compilebuild(self, code):
        """Used internally."""
        try:
            if id(self) in code.buildercache:
                return code.buildercache[id(self)]
            emitted = self._emitbuild(code)
            code.buildercache[id(self)] = emitted
            return emitted
        except NotImplementedError:
            self._compileinstance(code)
            return f"linkedbuilders[{id(self)}](obj, io, this, '(???)')"

    def _emitparse(self, code):
        """Override in your subclass."""
        raise NotImplementedError

    def _emitbuild(self, code):
        """Override in your subclass."""
        raise NotImplementedError

    def benchmark(self, sampledata, filename=None):
        """
        Measures performance of your construct (its parsing and building runtime), both for the original instance and the compiled instance. Uses timeit module, over at min 1 loop, and at max over 100 millisecond time.

        Optionally, results are saved to a text file for later inspection. Otherwise you can print the resulting string to terminal.

        :param sampledata: bytes, a valid blob parsable by this construct
        :param filename: optional, string, results are saved to that file

        :returns: string containing measurements
        """
        from timeit import timeit

        sampleobj = self.parse(sampledata)
        parsetime = timeit(lambda: self.parse(sampledata), number=1)
        runs = int(0.1/parsetime)
        if runs > 1:
            parsetime = timeit(lambda: self.parse(sampledata), number=runs)/runs
        parsetime = "{:.10f} sec/call".format(parsetime)

        self.build(sampleobj)
        buildtime = timeit(lambda: self.build(sampleobj), number=1)
        runs = int(0.1/buildtime)
        if runs > 1:
            buildtime = timeit(lambda: self.build(sampleobj), number=runs)/runs
        buildtime = "{:.10f} sec/call".format(buildtime)

        compiled = self.compile()
        compiled.parse(sampledata)
        parsetime2 = timeit(lambda: compiled.parse(sampledata), number=1)
        runs = int(0.1/parsetime2)
        if runs > 1:
            parsetime2 = timeit(lambda: compiled.parse(sampledata), number=runs)/runs
        parsetime2 = "{:.10f} sec/call".format(parsetime2)

        compiled.build(sampleobj)
        buildtime2 = timeit(lambda: compiled.build(sampleobj), number=1)
        runs = int(0.1/buildtime2)
        if runs > 1:
            buildtime2 = timeit(lambda: compiled.build(sampleobj), number=runs)/runs
        buildtime2 = "{:.10f} sec/call".format(buildtime2)

        lines = [
            "Compiled instance performance:",
            "parsing:            {}",
            "parsing compiled:   {}",
            "building:           {}",
            "building compiled:  {}",
            ""
        ]
        results = "\n".join(lines).format(parsetime, parsetime2, buildtime, buildtime2)

        if filename:
            with open(filename, "wt") as f:
                f.write(results)

        return results

    def export_ksy(self, schemaname="unnamed_schema", filename=None):
        from ruamel.yaml import YAML
        yaml = YAML()
        yaml.default_flow_style = False
        output = io.StringIO()
        gen = KsyGen()
        main = dict(meta=dict(id=schemaname), seq=self._compileseq(gen), instances=gen.instances, enums=gen.enums, types=gen.types)
        yaml.dump(main, output)
        source = output.getvalue()

        if filename:
            with open(filename, "wt") as f:
                f.write(source)
        return source

    def _compileseq(self, ksy, bitwise=False, recursion=0):
        if recursion >= 3:
            raise ConstructError("construct does not implement KSY export")
        try:
            return hyphenatelist(self._emitseq(ksy, bitwise))
        except NotImplementedError:
            return [dict(id="x", **self._compilefulltype(ksy, bitwise, recursion+1))]

    def _compileprimitivetype(self, ksy, bitwise=False, recursion=0):
        if recursion >= 3:
            raise ConstructError("construct does not implement KSY export")
        try:
            return self._emitprimitivetype(ksy, bitwise)
        except NotImplementedError:
            name = "type_%s" % ksy.allocateId()
            ksy.types[name] = dict(seq=self._compileseq(ksy, bitwise, recursion+1))
            return name

    def _compilefulltype(self, ksy, bitwise=False, recursion=0):
        if recursion >= 3:
            raise ConstructError("construct does not implement KSY export")
        try:
            return hyphenatedict(self._emitfulltype(ksy, bitwise))
        except NotImplementedError:
            return dict(type=self._compileprimitivetype(ksy, bitwise, recursion+1))

    def _emitseq(self, ksy, bitwise):
        """Override in your subclass."""
        raise NotImplementedError

    def _emitprimitivetype(self, ksy, bitwise):
        """Override in your subclass."""
        raise NotImplementedError

    def _emitfulltype(self, ksy, bitwise):
        """Override in your subclass."""
        raise NotImplementedError

    def __rtruediv__(self, name):
        """
        Used for renaming subcons, usually part of a Struct, like Struct("index" / Byte).
        """
        return Renamed(self, newname=name)

    __rdiv__ = __rtruediv__

    def __mul__(self, other):
        """
        Used for adding docstrings and parsed hooks to subcons, like "field" / Byte * "docstring" * processfunc.
        """
        if isinstance(other, stringtypes):
            return Renamed(self, newdocs=other)
        if callable(other):
            return Renamed(self, newparsed=other)
        raise ConstructError("operator * can only be used with string or lambda")

    def __rmul__(self, other):
        """
        Used for adding docstrings and parsed hooks to subcons, like "field" / Byte * "docstring" * processfunc.
        """
        if isinstance(other, stringtypes):
            return Renamed(self, newdocs=other)
        if callable(other):
            return Renamed(self, newparsed=other)
        raise ConstructError("operator * can only be used with string or lambda")

    def __add__(self, other):
        """
        Used for making Struct like ("index"/Byte + "prefix"/Byte).
        """
        lhs = self.subcons  if isinstance(self,  Struct) else [self]
        rhs = other.subcons if isinstance(other, Struct) else [other]
        return Struct(*(lhs + rhs))

    def __rshift__(self, other):
        """
        Used for making Sequences like (Byte >> Short).
        """
        lhs = self.subcons  if isinstance(self,  Sequence) else [self]
        rhs = other.subcons if isinstance(other, Sequence) else [other]
        return Sequence(*(lhs + rhs))

    def __getitem__(self, count):
        """
        Used for making Arrays like Byte[5] and Byte[this.count].
        """
        if isinstance(count, slice):
            raise ConstructError("subcon[N] syntax can only be used for Arrays, use GreedyRange(subcon) instead?")
        if isinstance(count, int) or callable(count):
            return Array(count, self)
        raise ConstructError("subcon[N] syntax expects integer or context lambda")


class Subconstruct(Construct):
    r"""
    Abstract subconstruct (wraps an inner construct, inheriting its name and flags). Parsing and building is by default deferred to subcon, same as sizeof.

    :param subcon: Construct instance
    """
    def __init__(self, subcon):
        if not isinstance(subcon, Construct):
            raise TypeError("subcon should be a Construct field")
        super().__init__()
        self.subcon = subcon
        self.flagbuildnone = subcon.flagbuildnone

    def __repr__(self):
        return "<%s%s%s%s %s>" % (self.__class__.__name__, " "+self.name if self.name else "", " +nonbuild" if self.flagbuildnone else "", " +docs" if self.docs else "", repr(self.subcon), )

    def _parse(self, stream, context, path):
        return self.subcon._parsereport(stream, context, path)

    def _build(self, obj, stream, context, path):
        return self.subcon._build(obj, stream, context, path)

    def _sizeof(self, context, path):
        return self.subcon._sizeof(context, path)


class Adapter(Subconstruct):
    r"""
    Abstract adapter class.

    Needs to implement `_decode()` for parsing and `_encode()` for building.

    :param subcon: Construct instance
    """
    def _parse(self, stream, context, path):
        obj = self.subcon._parsereport(stream, context, path)
        return self._decode(obj, context, path)

    def _build(self, obj, stream, context, path):
        obj2 = self._encode(obj, context, path)
        buildret = self.subcon._build(obj2, stream, context, path)
        return obj

    def _decode(self, obj, context, path):
        raise NotImplementedError

    def _encode(self, obj, context, path):
        raise NotImplementedError


class SymmetricAdapter(Adapter):
    r"""
    Abstract adapter class.

    Needs to implement `_decode()` only, for both parsing and building.

    :param subcon: Construct instance
    """
    def _encode(self, obj, context, path):
        return self._decode(obj, context, path)


class Validator(SymmetricAdapter):
    r"""
    Abstract class that validates a condition on the encoded/decoded object.

    Needs to implement `_validate()` that returns a bool (or a truthy value)

    :param subcon: Construct instance
    """
    def _decode(self, obj, context, path):
        if not self._validate(obj, context, path):
            raise ValidationError("object failed validation: %s" % (obj,), path=path)
        return obj

    def _validate(self, obj, context, path):
        raise NotImplementedError


class Tunnel(Subconstruct):
    r"""
    Abstract class that allows other constructs to read part of the stream as if they were reading the entire stream. See Prefixed for example.

    Needs to implement `_decode()` for parsing and `_encode()` for building.
    """
    def _parse(self, stream, context, path):
        data = stream_read_entire(stream, path)  # reads entire stream
        data = self._decode(data, context, path)
        return self.subcon.parse(data, **context)

    def _build(self, obj, stream, context, path):
        stream2 = io.BytesIO()
        buildret = self.subcon._build(obj, stream2, context, path)
        data = stream2.getvalue()
        data = self._encode(data, context, path)
        stream_write(stream, data, len(data), path)
        return obj

    def _sizeof(self, context, path):
        raise SizeofError(path=path)

    def _decode(self, data, context, path):
        raise NotImplementedError

    def _encode(self, data, context, path):
        raise NotImplementedError


class Compiled(Construct):
    """Used internally."""

    def __init__(self, parsefunc, buildfunc):
        super().__init__()
        self.source = None
        self.defersubcon = None
        self.parsefunc = parsefunc
        self.buildfunc = buildfunc

    def _parse(self, stream, context, path):
        return self.parsefunc(stream, context)

    def _build(self, obj, stream, context, path):
        return self.buildfunc(obj, stream, context)

    def _sizeof(self, context, path):
        return self.defersubcon._sizeof(context, path)

    def compile(self, filename=None):
        return self

    def benchmark(self, sampledata, filename=None):
        return self.defersubcon.benchmark(sampledata, filename)


#===============================================================================
# bytes and bits
#===============================================================================
class Bytes(Construct):
    r"""
    Field consisting of a specified number of bytes.

    Parses into a bytes (of given length). Builds into the stream directly (but checks that given object matches specified length). Can also build from an integer for convenience (although BytesInteger should be used instead). Size is the specified length.

    Can also build from a bytearray.

    :param length: integer or context lambda

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises StringError: building from non-bytes value, perhaps unicode

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Bytes(4)
        >>> d.parse(b'beef')
        b'beef'
        >>> d.build(b'beef')
        b'beef'
        >>> d.build(0)
        b'\x00\x00\x00\x00'
        >>> d.sizeof()
        4

        >>> d = Struct(
        ...     "length" / Int8ub,
        ...     "data" / Bytes(this.length),
        ... )
        >>> d.parse(b"\x04beef")
        Container(length=4, data=b'beef')
        >>> d.sizeof()
        construct.core.SizeofError: cannot calculate size, key not found in context
    """

    def __init__(self, length):
        super().__init__()
        self.length = length

    def _parse(self, stream, context, path):
        length = self.length(context) if callable(self.length) else self.length
        return stream_read(stream, length, path)

    def _build(self, obj, stream, context, path):
        length = self.length(context) if callable(self.length) else self.length
        data = integer2bytes(obj, length) if isinstance(obj, int) else obj
        data = bytes(data) if type(data) is bytearray else data
        stream_write(stream, data, length, path)
        return data

    def _sizeof(self, context, path):
        try:
            return self.length(context) if callable(self.length) else self.length
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)

    def _emitparse(self, code):
        return f"io.read({self.length})"

    def _emitbuild(self, code):
        return f"(io.write(obj), obj)[1]"

    def _emitfulltype(self, ksy, bitwise):
        return dict(size=self.length)


@singleton
class GreedyBytes(Construct):
    r"""
    Field consisting of unknown number of bytes.

    Parses the stream to the end. Builds into the stream directly (without checks). Size is undefined.

    Can also build from a bytearray.

    :raises StreamError: stream failed when reading until EOF
    :raises StringError: building from non-bytes value, perhaps unicode

    Example::

        >>> GreedyBytes.parse(b"asislight")
        b'asislight'
        >>> GreedyBytes.build(b"asislight")
        b'asislight'
    """

    def _parse(self, stream, context, path):
        return stream_read_entire(stream, path)

    def _build(self, obj, stream, context, path):
        data = bytes(obj) if type(obj) is bytearray else obj
        stream_write(stream, data, len(data), path)
        return data

    def _emitparse(self, code):
        return f"io.read()"

    def _emitbuild(self, code):
        return f"(io.write(obj), obj)[1]"

    def _emitfulltype(self, ksy, bitwise):
        return dict(size_eos=True)


def Bitwise(subcon):
    r"""
    Converts the stream from bytes to bits, and passes the bitstream to underlying subcon. Bitstream is a stream that contains 8 times as many bytes, and each byte is either \\x00 or \\x01 (in documentation those bytes are called bits).

    Parsing building and size are deferred to subcon, although size gets divided by 8 (therefore the subcon's size must be a multiple of 8).
    
    Note that by default the bit ordering is from MSB to LSB for every byte (ie. bit-level big-endian). If you need it reversed, wrap this subcon with :class:`construct.core.BitsSwapped`.

    :param subcon: Construct instance, any field that works with bits (like BitsInteger) or is bit-byte agnostic (like Struct or Flag)

    See :class:`~construct.core.Transformed` and :class:`~construct.core.Restreamed` for raisable exceptions.

    Example::

        >>> d = Bitwise(Struct(
        ...     'a' / Nibble,
        ...     'b' / Bytewise(Float32b),
        ...     'c' / Padding(4),
        ... ))
        >>> d.parse(bytes(5))
        Container(a=0, b=0.0, c=None)
        >>> d.sizeof()
        5

    Obtaining other byte or bit orderings::

        >>> d = Bitwise(Bytes(16))
        >>> d.parse(b'\x01\x03')
        b'\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x01'
        >>> d = BitsSwapped(Bitwise(Bytes(16)))
        >>> d.parse(b'\x01\x03')
        b'\x01\x00\x00\x00\x00\x00\x00\x00\x01\x01\x00\x00\x00\x00\x00\x00'
    """

    try:
        size = subcon.sizeof()
        macro = Transformed(subcon, bytes2bits, size//8, bits2bytes, size//8)
    except SizeofError:
        macro = Restreamed(subcon, bytes2bits, 1, bits2bytes, 8, lambda n: n//8)
    def _emitseq(ksy, bitwise):
        return subcon._compileseq(ksy, bitwise=True)
    def _emitprimitivetype(ksy, bitwise):
        return subcon._compileprimitivetype(ksy, bitwise=True)
    def _emitfulltype(ksy, bitwise):
        return subcon._compilefulltype(ksy, bitwise=True)
    macro._emitseq = _emitseq
    macro._emitprimitivetype = _emitprimitivetype
    macro._emitfulltype = _emitfulltype
    return macro


def Bytewise(subcon):
    r"""
    Converts the bitstream back to normal byte stream. Must be used within :class:`~construct.core.Bitwise`.

    Parsing building and size are deferred to subcon, although size gets multiplied by 8.

    :param subcon: Construct instance, any field that works with bytes or is bit-byte agnostic

    See :class:`~construct.core.Transformed` and :class:`~construct.core.Restreamed` for raisable exceptions.

    Example::

        >>> d = Bitwise(Struct(
        ...     'a' / Nibble,
        ...     'b' / Bytewise(Float32b),
        ...     'c' / Padding(4),
        ... ))
        >>> d.parse(bytes(5))
        Container(a=0, b=0.0, c=None)
        >>> d.sizeof()
        5
    """

    try:
        size = subcon.sizeof()
        macro = Transformed(subcon, bits2bytes, size*8, bytes2bits, size*8)
    except SizeofError:
        macro = Restreamed(subcon, bits2bytes, 8, bytes2bits, 1, lambda n: n*8)
    def _emitseq(ksy, bitwise):
        return subcon._compileseq(ksy, bitwise=False)
    def _emitprimitivetype(ksy, bitwise):
        return subcon._compileprimitivetype(ksy, bitwise=False)
    def _emitfulltype(ksy, bitwise):
        return subcon._compilefulltype(ksy, bitwise=False)
    macro._emitseq = _emitseq
    macro._emitprimitivetype = _emitprimitivetype
    macro._emitfulltype = _emitfulltype
    return macro


#===============================================================================
# integers and floats
#===============================================================================
class FormatField(Construct):
    r"""
    Field that uses `struct` module to pack and unpack CPU-sized integers and floats and booleans. This is used to implement most Int* Float* fields, but for example cannot pack 24-bit integers, which is left to :class:`~construct.core.BytesInteger` class. For booleans I also recommend using Flag class instead.

    See `struct module <https://docs.python.org/3/library/struct.html>`_ documentation for instructions on crafting format strings.

    Parses into an integer or float or boolean. Builds from an integer or float or boolean into specified byte count and endianness. Size is determined by `struct` module according to specified format string.

    :param endianity: string, character like: < > =
    :param format: string, character like: B H L Q b h l q e f d ?

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises FormatFieldError: wrong format string, or struct.(un)pack complained about the value

    Example::

        >>> d = FormatField(">", "H") or Int16ub
        >>> d.parse(b"\x01\x00")
        256
        >>> d.build(256)
        b"\x01\x00"
        >>> d.sizeof()
        2
    """

    def __init__(self, endianity, format):
        if endianity not in list("=<>"):
            raise FormatFieldError("endianity must be like: = < >", endianity)
        if format not in list("fdBHLQbhlqe?"):
            raise FormatFieldError("format must be like: B H L Q b h l q e f d ?", format)

        super().__init__()
        self.fmtstr = endianity+format
        self.length = struct.calcsize(endianity+format)

    def _parse(self, stream, context, path):
        data = stream_read(stream, self.length, path)
        try:
            return struct.unpack(self.fmtstr, data)[0]
        except Exception:
            raise FormatFieldError("struct %r error during parsing" % self.fmtstr, path=path)

    def _build(self, obj, stream, context, path):
        try:
            data = struct.pack(self.fmtstr, obj)
        except Exception:
            raise FormatFieldError("struct %r error during building, given value %r" % (self.fmtstr, obj), path=path)
        stream_write(stream, data, self.length, path)
        return obj

    def _sizeof(self, context, path):
        return self.length

    def _emitparse(self, code):
        fname = f"formatfield_{code.allocateId()}"
        code.append(f"{fname} = struct.Struct({repr(self.fmtstr)})")
        return f"{fname}.unpack(io.read({self.length}))[0]"

    def _emitbuild(self, code):
        fname = f"formatfield_{code.allocateId()}"
        code.append(f"{fname} = struct.Struct({repr(self.fmtstr)})")
        return f"(io.write({fname}.pack(obj)), obj)[1]"

    def _emitprimitivetype(self, ksy, bitwise):
        endianity,format = self.fmtstr
        signed = format.islower()
        swapped = (endianity == "<") or (endianity == "=" and sys.byteorder == "little")
        if format in "bhlqBHLQ":
            if bitwise:
                assert not signed
                assert not swapped
                return "b%s" % (8*self.length, )
            else:
                return "%s%s%s" % ("s" if signed else "u", self.length, "le" if swapped else "be", )
        if format in "fd":
            assert not bitwise
            return "f%s%s" % (self.length, "le" if swapped else "be", )


class BytesInteger(Construct):
    r"""
    Field that packs integers of arbitrary size. Int24* fields use this class.

    Parses into an integer. Builds from an integer into specified byte count and endianness. Size is specified in ctor.

    Analog to :class:`~construct.core.BitsInteger` which operates on bits. In fact::

        BytesInteger(n) <--> Bitwise(BitsInteger(8*n))
        BitsInteger(8*n) <--> Bytewise(BytesInteger(n))

    Byte ordering refers to bytes (chunks of 8 bits) so, for example::

        BytesInteger(n, swapped=True) <--> Bitwise(BitsInteger(8*n, swapped=True))

    :param length: integer or context lambda, number of bytes in the field
    :param signed: bool, whether the value is signed (two's complement), default is False (unsigned)
    :param swapped: bool or context lambda, whether to swap byte order (little endian), default is False (big endian)

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises IntegerError: length is negative or zero
    :raises IntegerError: value is not an integer
    :raises IntegerError: number does not fit given width and signed parameters

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = BytesInteger(4) or Int32ub
        >>> d.parse(b"abcd")
        1633837924
        >>> d.build(1)
        b'\x00\x00\x00\x01'
        >>> d.sizeof()
        4
    """

    def __init__(self, length, signed=False, swapped=False):
        super().__init__()
        self.length = length
        self.signed = signed
        self.swapped = swapped

    def _parse(self, stream, context, path):
        length = evaluate(self.length, context)
        if length <= 0:
            raise IntegerError(f"length {length} must be positive", path=path)
        data = stream_read(stream, length, path)
        if evaluate(self.swapped, context):
            data = swapbytes(data)
        try:
            return bytes2integer(data, self.signed)
        except ValueError as e:
            raise IntegerError(str(e), path=path)

    def _build(self, obj, stream, context, path):
        if not isinstance(obj, integertypes):
            raise IntegerError(f"value {obj} is not an integer", path=path)
        length = evaluate(self.length, context)
        if length <= 0:
            raise IntegerError(f"length {length} must be positive", path=path)
        try:
            data = integer2bytes(obj, length, self.signed)
        except ValueError as e:
            raise IntegerError(str(e), path=path)
        if evaluate(self.swapped, context):
            data = swapbytes(data)
        stream_write(stream, data, length, path)
        return obj

    def _sizeof(self, context, path):
        try:
            return evaluate(self.length, context)
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)

    def _emitparse(self, code):
        return f"bytes2integer(swapbytes(io.read({self.length})) if {self.swapped} else io.read({self.length}), {self.signed})"

    def _emitbuild(self, code):
        return f"((io.write(swapbytes(integer2bytes(obj, {self.length}, {self.signed})) if ({self.swapped}) else integer2bytes(obj, {self.length}, {self.signed}))), obj)[1]"

    def _emitprimitivetype(self, ksy, bitwise):
        if bitwise:
            assert not self.signed
            assert not self.swapped
            return "b%s" % (8*self.length, )
        else:
            assert not callable(self.swapped)
            return "%s%s%s" % ("s" if self.signed else "u", self.length, "le" if self.swapped else "be", )


class BitsInteger(Construct):
    r"""
    Field that packs arbitrarily large (or small) integers. Some fields (Bit Nibble Octet) use this class. Must be enclosed in :class:`~construct.core.Bitwise` context.

    Parses into an integer. Builds from an integer into specified bit count and endianness. Size (in bits) is specified in ctor.

    Analog to :class:`~construct.core.BytesInteger` which operates on bytes. In fact::

        BytesInteger(n) <--> Bitwise(BitsInteger(8*n))
        BitsInteger(8*n) <--> Bytewise(BytesInteger(n))

    Note that little-endianness is only defined for multiples of 8 bits.

    Byte ordering (i.e. `swapped` parameter) refers to bytes (chunks of 8 bits) so, for example::

        BytesInteger(n, swapped=True) <--> Bitwise(BitsInteger(8*n, swapped=True))

    Swapped argument was recently fixed. To obtain previous (faulty) behavior, you can use `ByteSwapped`, `BitsSwapped` and `Bitwise` in whatever particular order (see examples).

    :param length: integer or context lambda, number of bits in the field
    :param signed: bool, whether the value is signed (two's complement), default is False (unsigned)
    :param swapped: bool or context lambda, whether to swap byte order (little endian), default is False (big endian)

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises IntegerError: length is negative or zero
    :raises IntegerError: value is not an integer
    :raises IntegerError: number does not fit given width and signed parameters
    :raises IntegerError: little-endianness selected but length is not multiple of 8 bits

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Examples::

        >>> d = Bitwise(BitsInteger(8)) or Bitwise(Octet)
        >>> d.parse(b"\x10")
        16
        >>> d.build(255)
        b'\xff'
        >>> d.sizeof()
        1

    Obtaining other byte or bit orderings::

        >>> d = BitsInteger(2)
        >>> d.parse(b'\x01\x00') # Bit-Level Big-Endian
        2
        >>> d = ByteSwapped(BitsInteger(2))
        >>> d.parse(b'\x01\x00') # Bit-Level Little-Endian
        1
        >>> d = BitsInteger(16) # Byte-Level Big-Endian, Bit-Level Big-Endian
        >>> d.build(5 + 19*256)
        b'\x00\x00\x00\x01\x00\x00\x01\x01\x00\x00\x00\x00\x00\x01\x00\x01'
        >>> d = BitsInteger(16, swapped=True) # Byte-Level Little-Endian, Bit-Level Big-Endian
        >>> d.build(5 + 19*256)
        b'\x00\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x01\x00\x00\x01\x01'
        >>> d = ByteSwapped(BitsInteger(16)) # Byte-Level Little-Endian, Bit-Level Little-Endian
        >>> d.build(5 + 19*256)
        b'\x01\x00\x01\x00\x00\x00\x00\x00\x01\x01\x00\x00\x01\x00\x00\x00'
        >>> d = ByteSwapped(BitsInteger(16, swapped=True)) # Byte-Level Big-Endian, Bit-Level Little-Endian
        >>> d.build(5 + 19*256)
        b'\x01\x01\x00\x00\x01\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00'
    """

    def __init__(self, length, signed=False, swapped=False):
        super().__init__()
        self.length = length
        self.signed = signed
        self.swapped = swapped

    def _parse(self, stream, context, path):
        length = evaluate(self.length, context)
        if length <= 0:
            raise IntegerError(f"length {length} must be positive", path=path)
        data = stream_read(stream, length, path)
        try:
            if evaluate(self.swapped, context):
                data = swapbytesinbits(data)
            return bits2integer(data, self.signed)
        except ValueError as e:
            raise IntegerError(str(e), path=path)

    def _build(self, obj, stream, context, path):
        if not isinstance(obj, integertypes):
            raise IntegerError(f"value {obj} is not an integer", path=path)
        length = evaluate(self.length, context)
        if length <= 0:
            raise IntegerError(f"length {length} must be positive", path=path)
        try:
            data = integer2bits(obj, length, self.signed)
            if evaluate(self.swapped, context):
                data = swapbytesinbits(data)
        except ValueError as e:
            raise IntegerError(str(e), path=path)
        stream_write(stream, data, length, path)
        return obj

    def _sizeof(self, context, path):
        try:
            return evaluate(self.length, context)
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)

    def _emitparse(self, code):
        return f"bits2integer(swapbytesinbits(io.read({self.length})) if {self.swapped} else io.read({self.length}), {self.signed})"

    def _emitbuild(self, code):
        return f"((io.write(swapbytesinbits(integer2bits(obj, {self.length}, {self.signed})) if ({self.swapped}) else integer2bits(obj, {self.length}, {self.signed}))), obj)[1]"

    def _emitprimitivetype(self, ksy, bitwise):
        assert not self.signed
        assert not self.swapped
        return "b%s" % (self.length, )


@singleton
def Bit():
    """A 1-bit integer, must be enclosed in a Bitwise (eg. BitStruct)"""
    return BitsInteger(1)
@singleton
def Nibble():
    """A 4-bit integer, must be enclosed in a Bitwise (eg. BitStruct)"""
    return BitsInteger(4)
@singleton
def Octet():
    """A 8-bit integer, must be enclosed in a Bitwise (eg. BitStruct)"""
    return BitsInteger(8)

@singleton
def Int8ub():
    """Unsigned, big endian 8-bit integer"""
    return FormatField(">", "B")
@singleton
def Int16ub():
    """Unsigned, big endian 16-bit integer"""
    return FormatField(">", "H")
@singleton
def Int32ub():
    """Unsigned, big endian 32-bit integer"""
    return FormatField(">", "L")
@singleton
def Int64ub():
    """Unsigned, big endian 64-bit integer"""
    return FormatField(">", "Q")

@singleton
def Int8sb():
    """Signed, big endian 8-bit integer"""
    return FormatField(">", "b")
@singleton
def Int16sb():
    """Signed, big endian 16-bit integer"""
    return FormatField(">", "h")
@singleton
def Int32sb():
    """Signed, big endian 32-bit integer"""
    return FormatField(">", "l")
@singleton
def Int64sb():
    """Signed, big endian 64-bit integer"""
    return FormatField(">", "q")

@singleton
def Int8ul():
    """Unsigned, little endian 8-bit integer"""
    return FormatField("<", "B")
@singleton
def Int16ul():
    """Unsigned, little endian 16-bit integer"""
    return FormatField("<", "H")
@singleton
def Int32ul():
    """Unsigned, little endian 32-bit integer"""
    return FormatField("<", "L")
@singleton
def Int64ul():
    """Unsigned, little endian 64-bit integer"""
    return FormatField("<", "Q")

@singleton
def Int8sl():
    """Signed, little endian 8-bit integer"""
    return FormatField("<", "b")
@singleton
def Int16sl():
    """Signed, little endian 16-bit integer"""
    return FormatField("<", "h")
@singleton
def Int32sl():
    """Signed, little endian 32-bit integer"""
    return FormatField("<", "l")
@singleton
def Int64sl():
    """Signed, little endian 64-bit integer"""
    return FormatField("<", "q")

@singleton
def Int8un():
    """Unsigned, native endianity 8-bit integer"""
    return FormatField("=", "B")
@singleton
def Int16un():
    """Unsigned, native endianity 16-bit integer"""
    return FormatField("=", "H")
@singleton
def Int32un():
    """Unsigned, native endianity 32-bit integer"""
    return FormatField("=", "L")
@singleton
def Int64un():
    """Unsigned, native endianity 64-bit integer"""
    return FormatField("=", "Q")

@singleton
def Int8sn():
    """Signed, native endianity 8-bit integer"""
    return FormatField("=", "b")
@singleton
def Int16sn():
    """Signed, native endianity 16-bit integer"""
    return FormatField("=", "h")
@singleton
def Int32sn():
    """Signed, native endianity 32-bit integer"""
    return FormatField("=", "l")
@singleton
def Int64sn():
    """Signed, native endianity 64-bit integer"""
    return FormatField("=", "q")

Byte  = Int8ub
Short = Int16ub
Int   = Int32ub
Long  = Int64ub

@singleton
def Float16b():
    """Big endian, 16-bit IEEE 754 floating point number"""
    return FormatField(">", "e")
@singleton
def Float16l():
    """Little endian, 16-bit IEEE 754 floating point number"""
    return FormatField("<", "e")
@singleton
def Float16n():
    """Native endianity, 16-bit IEEE 754 floating point number"""
    return FormatField("=", "e")

@singleton
def Float32b():
    """Big endian, 32-bit IEEE floating point number"""
    return FormatField(">", "f")
@singleton
def Float32l():
    """Little endian, 32-bit IEEE floating point number"""
    return FormatField("<", "f")
@singleton
def Float32n():
    """Native endianity, 32-bit IEEE floating point number"""
    return FormatField("=", "f")

@singleton
def Float64b():
    """Big endian, 64-bit IEEE floating point number"""
    return FormatField(">", "d")
@singleton
def Float64l():
    """Little endian, 64-bit IEEE floating point number"""
    return FormatField("<", "d")
@singleton
def Float64n():
    """Native endianity, 64-bit IEEE floating point number"""
    return FormatField("=", "d")

Half = Float16b
Single = Float32b
Double = Float64b

native = (sys.byteorder == "little")

@singleton
def Int24ub():
    """A 3-byte big-endian unsigned integer, as used in ancient file formats."""
    return BytesInteger(3, signed=False, swapped=False)
@singleton
def Int24ul():
    """A 3-byte little-endian unsigned integer, as used in ancient file formats."""
    return BytesInteger(3, signed=False, swapped=True)
@singleton
def Int24un():
    """A 3-byte native-endian unsigned integer, as used in ancient file formats."""
    return BytesInteger(3, signed=False, swapped=native)
@singleton
def Int24sb():
    """A 3-byte big-endian signed integer, as used in ancient file formats."""
    return BytesInteger(3, signed=True, swapped=False)
@singleton
def Int24sl():
    """A 3-byte little-endian signed integer, as used in ancient file formats."""
    return BytesInteger(3, signed=True, swapped=True)
@singleton
def Int24sn():
    """A 3-byte native-endian signed integer, as used in ancient file formats."""
    return BytesInteger(3, signed=True, swapped=native)


@singleton
class VarInt(Construct):
    r"""
    VarInt encoded unsigned integer. Each 7 bits of the number are encoded in one byte of the stream, where leftmost bit (MSB) is unset when byte is terminal. Scheme is defined at Google site related to `Protocol Buffers <https://developers.google.com/protocol-buffers/docs/encoding>`_.

    Can only encode non-negative numbers.

    Parses into an integer. Builds from an integer. Size is undefined.

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises IntegerError: given a negative value, or not an integer

    Example::

        >>> VarInt.build(1)
        b'\x01'
        >>> VarInt.build(2**100)
        b'\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x04'
    """

    def _parse(self, stream, context, path):
        acc = []
        while True:
            b = byte2int(stream_read(stream, 1, path))
            acc.append(b & 0b01111111)
            if b & 0b10000000 == 0:
                break
        num = 0
        for b in reversed(acc):
            num = (num << 7) | b
        return num

    def _build(self, obj, stream, context, path):
        if not isinstance(obj, integertypes):
            raise IntegerError(f"value {obj} is not an integer", path=path)
        if obj < 0:
            raise IntegerError(f"VarInt cannot build from negative number {obj}", path=path)
        x = obj
        B = bytearray()
        while x > 0b01111111:
            B.append(0b10000000 | (x & 0b01111111))
            x >>= 7
        B.append(x)
        stream_write(stream, bytes(B), len(B), path)
        return obj

    def _emitprimitivetype(self, ksy, bitwise):
        return "vlq_base128_le"


@singleton
class ZigZag(Construct):
    r"""
    ZigZag encoded signed integer. This is a variant of VarInt encoding that also can encode negative numbers. Scheme is defined at Google site related to `Protocol Buffers <https://developers.google.com/protocol-buffers/docs/encoding>`_.

    Can also encode negative numbers.

    Parses into an integer. Builds from an integer. Size is undefined.

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises IntegerError: given not an integer

    Example::

        >>> ZigZag.build(-3)
        b'\x05'
        >>> ZigZag.build(3)
        b'\x06'
    """

    def _parse(self, stream, context, path):
        x = VarInt._parse(stream, context, path)
        if x & 1 == 0:
            x = x//2
        else:
            x = -(x//2+1)
        return x

    def _build(self, obj, stream, context, path):
        if not isinstance(obj, integertypes):
            raise IntegerError(f"value {obj} is not an integer", path=path)
        if obj >= 0:
            x = 2*obj
        else:
            x = 2*abs(obj)-1
        VarInt._build(x, stream, context, path)
        return obj


#===============================================================================
# strings
#===============================================================================

#: Explicitly supported encodings (by PaddedString and CString classes).
#:
possiblestringencodings = dict(
    ascii=1,
    utf8=1, utf_8=1, u8=1,
    utf16=2, utf_16=2, u16=2, utf_16_be=2, utf_16_le=2,
    utf32=4, utf_32=4, u32=4, utf_32_be=4, utf_32_le=4,
)


def encodingunit(encoding):
    """Used internally."""
    encoding = encoding.replace("-","_").lower()
    if encoding not in possiblestringencodings:
        raise StringError("encoding %r not found among %r" % (encoding, possiblestringencodings,))
    return bytes(possiblestringencodings[encoding])


class StringEncoded(Adapter):
    """Used internally."""

    def __init__(self, subcon, encoding):
        super().__init__(subcon)
        if not encoding:
            raise StringError("String* classes require explicit encoding")
        self.encoding = encoding

    def _decode(self, obj, context, path):
        try:
            return obj.decode(self.encoding)
        except:
            raise StringError(f"cannot use encoding {self.encoding!r} to decode {obj!r}")

    def _encode(self, obj, context, path):
        if not isinstance(obj, unicodestringtype):
            raise StringError("string encoding failed, expected unicode string", path=path)
        if obj == u"":
            return b""
        try:
            return obj.encode(self.encoding)
        except:
            raise StringError(f"cannot use encoding {self.encoding!r} to encode {obj!r}")

    def _emitparse(self, code):
        raise NotImplementedError
        # Not sure what the correct implementation would be.
        # return f"({self.subcon._compileparse(code)}).decode({repr(self.encoding)})"

    def _emitbuild(self, code):
        raise NotImplementedError
        # This is not a valid implementation. obj.encode() should be inserted into subcon
        # return f"({self.subcon._compilebuild(code)}).encode({repr(self.encoding)})"


def PaddedString(length, encoding):
    r"""
    Configurable, fixed-length or variable-length string field.

    When parsing, the byte string is stripped of null bytes (per encoding unit), then decoded. Length is an integer or context lambda. When building, the string is encoded and then padded to specified length. If encoded string is larger than the specified length, it fails with PaddingError. Size is same as length parameter.

    .. warning:: PaddedString and CString only support encodings explicitly listed in :class:`~construct.core.possiblestringencodings` .

    :param length: integer or context lambda, length in bytes (not unicode characters)
    :param encoding: string like: utf8 utf16 utf32 ascii

    :raises StringError: building a non-unicode string
    :raises StringError: selected encoding is not on supported list

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = PaddedString(10, "utf8")
        >>> d.build(u"Афон")
        b'\xd0\x90\xd1\x84\xd0\xbe\xd0\xbd\x00\x00'
        >>> d.parse(_)
        u'Афон'
    """
    macro = StringEncoded(FixedSized(length, NullStripped(GreedyBytes, pad=encodingunit(encoding))), encoding)
    def _emitfulltype(ksy, bitwise):
        return dict(size=length, type="strz", encoding=encoding)
    macro._emitfulltype = _emitfulltype
    return macro


def PascalString(lengthfield, encoding):
    r"""
    Length-prefixed string. The length field can be variable length (such as VarInt) or fixed length (such as Int64ub). :class:`~construct.core.VarInt` is recommended when designing new protocols. Stored length is in bytes, not characters. Size is not defined.

    :param lengthfield: Construct instance, field used to parse and build the length (like VarInt Int64ub)
    :param encoding: string like: utf8 utf16 utf32 ascii

    :raises StringError: building a non-unicode string

    Example::

        >>> d = PascalString(VarInt, "utf8")
        >>> d.build(u"Афон")
        b'\x08\xd0\x90\xd1\x84\xd0\xbe\xd0\xbd'
        >>> d.parse(_)
        u'Афон'
    """
    macro = StringEncoded(Prefixed(lengthfield, GreedyBytes), encoding)

    def _emitparse(code):
        return f"io.read({lengthfield._compileparse(code)}).decode({repr(encoding)})"
    macro._emitparse = _emitparse

    def _emitseq(ksy, bitwise):
        return [
            dict(id="lengthfield", type=lengthfield._compileprimitivetype(ksy, bitwise)), 
            dict(id="data", size="lengthfield", type="str", encoding=encoding),
        ]
    macro._emitseq = _emitseq

    return macro


def CString(encoding):
    r"""
    String ending in a terminating null byte (or null bytes in case of UTF16 UTF32).

    .. warning:: String and CString only support encodings explicitly listed in :class:`~construct.core.possiblestringencodings` .

    :param encoding: string like: utf8 utf16 utf32 ascii

    :raises StringError: building a non-unicode string
    :raises StringError: selected encoding is not on supported list

    Example::

        >>> d = CString("utf8")
        >>> d.build(u"Афон")
        b'\xd0\x90\xd1\x84\xd0\xbe\xd0\xbd\x00'
        >>> d.parse(_)
        u'Афон'
    """
    macro = StringEncoded(NullTerminated(GreedyBytes, term=encodingunit(encoding)), encoding)
    def _emitfulltype(ksy, bitwise):
        return dict(type="strz", encoding=encoding)
    macro._emitfulltype = _emitfulltype
    return macro


def GreedyString(encoding):
    r"""
    String that reads entire stream until EOF, and writes a given string as-is. Analog to :class:`~construct.core.GreedyBytes` but also applies unicode-to-bytes encoding.

    :param encoding: string like: utf8 utf16 utf32 ascii

    :raises StringError: building a non-unicode string
    :raises StreamError: stream failed when reading until EOF

    Example::

        >>> d = GreedyString("utf8")
        >>> d.build(u"Афон")
        b'\xd0\x90\xd1\x84\xd0\xbe\xd0\xbd'
        >>> d.parse(_)
        u'Афон'
    """
    macro = StringEncoded(GreedyBytes, encoding)
    def _emitfulltype(ksy, bitwise):
        return dict(size_eos=True, type="str", encoding=encoding)
    macro._emitfulltype = _emitfulltype
    return macro


#===============================================================================
# mappings
#===============================================================================
@singleton
class Flag(Construct):
    r"""
    One byte (or one bit) field that maps to True or False. Other non-zero bytes are also considered True. Size is defined as 1.

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes

    Example::

        >>> Flag.parse(b"\x01")
        True
        >>> Flag.build(True)
        b'\x01'
    """

    def _parse(self, stream, context, path):
        return stream_read(stream, 1, path) != b"\x00"

    def _build(self, obj, stream, context, path):
        stream_write(stream, b"\x01" if obj else b"\x00", 1, path)
        return obj

    def _sizeof(self, context, path):
        return 1

    def _emitparse(self, code):
        return f"(io.read(1) != b'\\x00')"

    def _emitbuild(self, code):
        return f"((io.write(b'\\x01') if obj else io.write(b'\\x00')), obj)[1]"

    def _emitfulltype(self, ksy, bitwise):
        return dict(type=("b1" if bitwise else "u1"), _construct_render="Flag")


class EnumInteger(int):
    """Used internally."""
    pass


class EnumIntegerString(str):
    """Used internally."""

    def __repr__(self):
        return "EnumIntegerString.new(%s, %s)" % (self.intvalue, str.__repr__(self), )

    def __int__(self):
        return self.intvalue

    @staticmethod
    def new(intvalue, stringvalue):
        ret = EnumIntegerString(stringvalue)
        ret.intvalue = intvalue
        return ret


class Enum(Adapter):
    r"""
    Translates unicode label names to subcon values, and vice versa.

    Parses integer subcon, then uses that value to lookup mapping dictionary. Returns an integer-convertible string (if mapping found) or an integer (otherwise). Building is a reversed process. Can build from an integer flag or string label. Size is same as subcon, unless it raises SizeofError.

    There is no default parameter, because if no mapping is found, it parses into an integer without error.

    This class supports enum module. See examples.

    This class supports exposing member labels as attributes, as integer-convertible strings. See examples.

    :param subcon: Construct instance, subcon to map to/from
    :param \*merge: optional, list of enum.IntEnum and enum.IntFlag instances, to merge labels and values from
    :param \*\*mapping: dict, mapping string names to values

    :raises MappingError: building from string but no mapping found

    Example::

        >>> d = Enum(Byte, one=1, two=2, four=4, eight=8)
        >>> d.parse(b"\x01")
        'one'
        >>> int(d.parse(b"\x01"))
        1
        >>> d.parse(b"\xff")
        255
        >>> int(d.parse(b"\xff"))
        255

        >>> d.build(d.one or "one" or 1)
        b'\x01'
        >>> d.one
        'one'

        import enum
        class E(enum.IntEnum or enum.IntFlag):
            one = 1
            two = 2

        Enum(Byte, E) <--> Enum(Byte, one=1, two=2)
        FlagsEnum(Byte, E) <--> FlagsEnum(Byte, one=1, two=2)
    """

    def __init__(self, subcon, *merge, **mapping):
        super().__init__(subcon)
        for enum in merge:
            for enumentry in enum:
                mapping[enumentry.name] = enumentry.value
        self.encmapping = {EnumIntegerString.new(v,k):v for k,v in mapping.items()}
        self.decmapping = {v:EnumIntegerString.new(v,k) for k,v in mapping.items()}
        self.ksymapping = {v:k for k,v in mapping.items()}

    def __getattr__(self, name):
        if name in self.encmapping:
            return self.decmapping[self.encmapping[name]]
        raise AttributeError

    def _decode(self, obj, context, path):
        try:
            return self.decmapping[obj]
        except KeyError:
            return EnumInteger(obj)

    def _encode(self, obj, context, path):
        try:
            if isinstance(obj, integertypes):
                return obj
            return self.encmapping[obj]
        except KeyError:
            raise MappingError("building failed, no mapping for %r" % (obj,), path=path)

    def _emitparse(self, code):
        fname = f"factory_{code.allocateId()}"
        code.append(f"{fname} = {repr(self.decmapping)}")
        return f"reuse(({self.subcon._compileparse(code)}), lambda x: {fname}.get(x, EnumInteger(x)))"

    def _emitbuild(self, code):
        fname = f"factory_{code.allocateId()}"
        code.append(f"{fname} = {repr(self.encmapping)}")
        return f"reuse({fname}.get(obj, obj), lambda obj: ({self.subcon._compilebuild(code)}))"

    def _emitprimitivetype(self, ksy, bitwise):
        name = "enum_%s" % ksy.allocateId()
        ksy.enums[name] = self.ksymapping
        return name


class BitwisableString(str):
    """Used internally."""

    # def __repr__(self):
    #     return "BitwisableString(%s)" % (str.__repr__(self), )

    def __or__(self, other):
        return BitwisableString("{}|{}".format(self, other))


class FlagsEnum(Adapter):
    r"""
    Translates unicode label names to subcon integer (sub)values, and vice versa.

    Parses integer subcon, then creates a Container, where flags define each key. Builds from a container by bitwise-oring of each flag if it matches a set key. Can build from an integer flag or string label directly, as well as | concatenations thereof (see examples). Size is same as subcon, unless it raises SizeofError.

    This class supports enum module. See examples.

    This class supports exposing member labels as attributes, as bitwisable strings. See examples.

    :param subcon: Construct instance, must operate on integers
    :param \*merge: optional, list of enum.IntEnum and enum.IntFlag instances, to merge labels and values from
    :param \*\*flags: dict, mapping string names to integer values

    :raises MappingError: building from object not like: integer string dict
    :raises MappingError: building from string but no mapping found

    Can raise arbitrary exceptions when computing | and & and value is non-integer.

    Example::

        >>> d = FlagsEnum(Byte, one=1, two=2, four=4, eight=8)
        >>> d.parse(b"\x03")
        Container(one=True, two=True, four=False, eight=False)
        >>> d.build(dict(one=True,two=True))
        b'\x03'

        >>> d.build(d.one|d.two or "one|two" or 1|2)
        b'\x03'

        import enum
        class E(enum.IntEnum or enum.IntFlag):
            one = 1
            two = 2

        Enum(Byte, E) <--> Enum(Byte, one=1, two=2)
        FlagsEnum(Byte, E) <--> FlagsEnum(Byte, one=1, two=2)
    """

    def __init__(self, subcon, *merge, **flags):
        super().__init__(subcon)
        for enum in merge:
            for enumentry in enum:
                flags[enumentry.name] = enumentry.value
        self.flags = flags
        self.reverseflags = {v:k for k,v in flags.items()}

    def __getattr__(self, name):
        if name in self.flags:
            return BitwisableString(name)
        raise AttributeError

    def _decode(self, obj, context, path):
        obj2 = Container()
        obj2._flagsenum = True
        for name,value in self.flags.items():
            obj2[BitwisableString(name)] = (obj & value == value)
        return obj2

    def _encode(self, obj, context, path):
        try:
            if isinstance(obj, integertypes):
                return obj
            if isinstance(obj, stringtypes):
                flags = 0
                for name in obj.split("|"):
                    name = name.strip()
                    if name:
                        flags |= self.flags[name] # KeyError
                return flags
            if isinstance(obj, dict):
                flags = 0
                for name,value in obj.items():
                    if not name.startswith("_"): # assumes key is a string
                        if value:
                            flags |= self.flags[name] # KeyError
                return flags
            raise MappingError("building failed, unknown object: %r" % (obj,), path=path)
        except KeyError:
            raise MappingError("building failed, unknown label: %r" % (obj,), path=path)

    def _emitparse(self, code):
        return f"reuse(({self.subcon._compileparse(code)}), lambda x: Container({', '.join(f'{k}=bool(x & {v} == {v})' for k,v in self.flags.items()) }))"

    def _emitseq(self, ksy, bitwise):
        bitstotal = self.subcon.sizeof() * 8
        seq = []
        for i in range(bitstotal):
            value = 1<<i
            name = self.reverseflags.get(value, "unknown_%s" % i)
            seq.append(dict(id=name, type="b1", doc=hex(value), _construct_render="Flag"))
        return seq


class Mapping(Adapter):
    r"""
    Adapter that maps objects to other objects. Translates objects after parsing and before building. Can for example, be used to translate between enum objects and strings, but Enum class supports enum module already and is recommended.

    :param subcon: Construct instance
    :param mapping: dict, for encoding (building) mapping, the reversed is used for parsing mapping

    :raises MappingError: parsing or building but no mapping found

    Example::

        >>> x = object
        >>> d = Mapping(Byte, {x:0})
        >>> d.parse(b"\x00")
        x
        >>> d.build(x)
        b'\x00'
    """

    def __init__(self, subcon, mapping):
        super().__init__(subcon)
        self.decmapping = {v:k for k,v in mapping.items()}
        self.encmapping = mapping

    def _decode(self, obj, context, path):
        try:
            return self.decmapping[obj] # KeyError
        except (KeyError, TypeError):
            raise MappingError("parsing failed, no decoding mapping for %r" % (obj,), path=path)

    def _encode(self, obj, context, path):
        try:
            return self.encmapping[obj] # KeyError
        except (KeyError, TypeError):
            raise MappingError("building failed, no encoding mapping for %r" % (obj,), path=path)

    def _emitparse(self, code):
        fname = f"factory_{code.allocateId()}"
        code.append(f"{fname} = {repr(self.decmapping)}")
        return f"{fname}[{self.subcon._compileparse(code)}]"

    def _emitbuild(self, code):
        fname = f"factory_{code.allocateId()}"
        code.append(f"{fname} = {repr(self.encmapping)}")
        return f"reuse({fname}[obj], lambda obj: ({self.subcon._compilebuild(code)}))"


#===============================================================================
# structures and sequences
#===============================================================================
class Struct(Construct):
    r"""
    Sequence of usually named constructs, similar to structs in C. The members are parsed and build in the order they are defined. If a member is anonymous (its name is None) then it gets parsed and the value discarded, or it gets build from nothing (from None).

    Some fields do not need to be named, since they are built without value anyway. See: Const Padding Check Error Pass Terminated Seek Tell for examples of such fields. 

    Operator + can also be used to make Structs (although not recommended).

    Parses into a Container (dict with attribute and key access) where keys match subcon names. Builds from a dict (not necessarily a Container) where each member gets a value from the dict matching the subcon name. If field has build-from-none flag, it gets build even when there is no matching entry in the dict. Size is the sum of all subcon sizes, unless any subcon raises SizeofError.

    This class does context nesting, meaning its members are given access to a new dictionary where the "_" entry points to the outer context. When parsing, each member gets parsed and subcon parse return value is inserted into context under matching key only if the member was named. When building, the matching entry gets inserted into context before subcon gets build, and if subcon build returns a new value (not None) that gets replaced in the context.

    This class exposes subcons as attributes. You can refer to subcons that were inlined (and therefore do not exist as variable in the namespace) by accessing the struct attributes, under same name. Also note that compiler does not support this feature. See examples.

    This class exposes subcons in the context. You can refer to subcons that were inlined (and therefore do not exist as variable in the namespace) within other inlined fields using the context. Note that you need to use a lambda (`this` expression is not supported). Also note that compiler does not support this feature. See examples.

    This class supports stopping. If :class:`~construct.core.StopIf` field is a member, and it evaluates its lambda as positive, this class ends parsing or building as successful without processing further fields.

    :param \*subcons: Construct instances, list of members, some can be anonymous
    :param \*\*subconskw: Construct instances, list of members (requires Python 3.6)

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises KeyError: building a subcon but found no corresponding key in dictionary

    Example::

        >>> d = Struct("num"/Int8ub, "data"/Bytes(this.num))
        >>> d.parse(b"\x04DATA")
        Container(num=4, data=b"DATA")
        >>> d.build(dict(num=4, data=b"DATA"))
        b"\x04DATA"

        >>> d = Struct(Const(b"MZ"), Padding(2), Pass, Terminated)
        >>> d.build({})
        b'MZ\x00\x00'
        >>> d.parse(_)
        Container()
        >>> d.sizeof()
        4

        >>> d = Struct(
        ...     "animal" / Enum(Byte, giraffe=1),
        ... )
        >>> d.animal.giraffe
        'giraffe'
        >>> d = Struct(
        ...     "count" / Byte,
        ...     "data" / Bytes(lambda this: this.count - this._subcons.count.sizeof()),
        ... )
        >>> d.build(dict(count=3, data=b"12"))
        b'\x0312'

        Alternative syntax (not recommended):
        >>> ("a"/Byte + "b"/Byte + "c"/Byte + "d"/Byte)

        Alternative syntax, but requires Python 3.6 or any PyPy:
        >>> Struct(a=Byte, b=Byte, c=Byte, d=Byte)
    """

    def __init__(self, *subcons, **subconskw):
        super().__init__()
        self.subcons = list(subcons) + list(k/v for k,v in subconskw.items())
        self._subcons = Container((sc.name,sc) for sc in self.subcons if sc.name)
        self.flagbuildnone = all(sc.flagbuildnone for sc in self.subcons)

    def __getattr__(self, name):
        if name in self._subcons:
            return self._subcons[name]
        raise AttributeError

    def _parse(self, stream, context, path):
        obj = Container()
        obj._io = stream
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        for sc in self.subcons:
            try:
                subobj = sc._parsereport(stream, context, path)
                if sc.name:
                    obj[sc.name] = subobj
                    context[sc.name] = subobj
            except StopFieldError:
                break
        return obj

    def _build(self, obj, stream, context, path):
        if obj is None:
            obj = Container()
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        context.update(obj)
        for sc in self.subcons:
            try:
                if sc.flagbuildnone:
                    subobj = obj.get(sc.name, None)
                else:
                    subobj = obj[sc.name] # raises KeyError

                if sc.name:
                    context[sc.name] = subobj

                buildret = sc._build(subobj, stream, context, path)
                if sc.name:
                    context[sc.name] = buildret
            except StopFieldError:
                break
        return context

    def _sizeof(self, context, path):
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = None, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        try:
            return sum(sc._sizeof(context, path) for sc in self.subcons)
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)

    def _emitparse(self, code):
        fname = f"parse_struct_{code.allocateId()}"
        block = f"""
            def {fname}(io, this):
                result = Container()
                this = Container(_ = this, _params = this['_params'], _root = None, _parsing = True, _building = False, _sizing = False, _subcons = None, _io = io, _index = this.get('_index', None))
                this['_root'] = this['_'].get('_root', this)
                try:
        """
        for sc in self.subcons:
            block += f"""
                    {f'result[{repr(sc.name)}] = this[{repr(sc.name)}] = ' if sc.name else ''}{sc._compileparse(code)}
            """
        block += f"""
                    pass
                except StopFieldError:
                    pass
                return result
        """
        code.append(block)
        return f"{fname}(io, this)"

    def _emitbuild(self, code):
        fname = f"build_struct_{code.allocateId()}"
        block = f"""
            def {fname}(obj, io, this):
                this = Container(_ = this, _params = this['_params'], _root = None, _parsing = False, _building = True, _sizing = False, _subcons = None, _io = io, _index = this.get('_index', None))
                this['_root'] = this['_'].get('_root', this)
                this.update(obj)
                try:
                    objdict = obj
        """
        for sc in self.subcons:
            block += f"""
                    {f'obj = objdict.get({repr(sc.name)}, None)' if sc.flagbuildnone else f'obj = objdict[{repr(sc.name)}]'}
                    {f'this[{repr(sc.name)}] = obj' if sc.name else ''}
                    {f'this[{repr(sc.name)}] = ' if sc.name else ''}{sc._compilebuild(code)}
            """
        block += f"""
                    pass
                except StopFieldError:
                    pass
                return this
        """
        code.append(block)
        return f"{fname}(obj, io, this)"

    def _emitseq(self, ksy, bitwise):
        return [sc._compilefulltype(ksy, bitwise) for sc in self.subcons]


class Sequence(Construct):
    r"""
    Sequence of usually un-named constructs. The members are parsed and build in the order they are defined. If a member is named, its parsed value gets inserted into the context. This allows using members that refer to previous members.

    Operator >> can also be used to make Sequences (although not recommended).

    Parses into a ListContainer (list with pretty-printing) where values are in same order as subcons. Builds from a list (not necessarily a ListContainer) where each subcon is given the element at respective position. Size is the sum of all subcon sizes, unless any subcon raises SizeofError.

    This class does context nesting, meaning its members are given access to a new dictionary where the "_" entry points to the outer context. When parsing, each member gets parsed and subcon parse return value is inserted into context under matching key only if the member was named. When building, the matching entry gets inserted into context before subcon gets build, and if subcon build returns a new value (not None) that gets replaced in the context.

    This class exposes subcons as attributes. You can refer to subcons that were inlined (and therefore do not exist as variable in the namespace) by accessing the struct attributes, under same name. Also note that compiler does not support this feature. See examples.

    This class exposes subcons in the context. You can refer to subcons that were inlined (and therefore do not exist as variable in the namespace) within other inlined fields using the context. Note that you need to use a lambda (`this` expression is not supported). Also note that compiler does not support this feature. See examples.

    This class supports stopping. If :class:`~construct.core.StopIf` field is a member, and it evaluates its lambda as positive, this class ends parsing or building as successful without processing further fields.

    :param \*subcons: Construct instances, list of members, some can be named
    :param \*\*subconskw: Construct instances, list of members (requires Python 3.6)

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises KeyError: building a subcon but found no corresponding key in dictionary

    Example::

        >>> d = Sequence(Byte, Float32b)
        >>> d.build([0, 1.23])
        b'\x00?\x9dp\xa4'
        >>> d.parse(_)
        [0, 1.2300000190734863] # a ListContainer

        >>> d = Sequence(
        ...     "animal" / Enum(Byte, giraffe=1),
        ... )
        >>> d.animal.giraffe
        'giraffe'
        >>> d = Sequence(
        ...     "count" / Byte,
        ...     "data" / Bytes(lambda this: this.count - this._subcons.count.sizeof()),
        ... )
        >>> d.build([3, b"12"])
        b'\x0312'

        Alternative syntax (not recommended):
        >>> (Byte >> "Byte >> "c"/Byte >> "d"/Byte)

        Alternative syntax, but requires Python 3.6 or any PyPy:
        >>> Sequence(a=Byte, b=Byte, c=Byte, d=Byte)
    """

    def __init__(self, *subcons, **subconskw):
        super().__init__()
        self.subcons = list(subcons) + list(k/v for k,v in subconskw.items())
        self._subcons = Container((sc.name,sc) for sc in self.subcons if sc.name)
        self.flagbuildnone = all(sc.flagbuildnone for sc in self.subcons)

    def __getattr__(self, name):
        if name in self._subcons:
            return self._subcons[name]
        raise AttributeError

    def _parse(self, stream, context, path):
        obj = ListContainer()
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        for sc in self.subcons:
            try:
                subobj = sc._parsereport(stream, context, path)
                obj.append(subobj)
                if sc.name:
                    context[sc.name] = subobj
            except StopFieldError:
                break
        return obj

    def _build(self, obj, stream, context, path):
        if obj is None:
            obj = ListContainer([None for sc in self.subcons])
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        objiter = iter(obj)
        retlist = ListContainer()
        for i,sc in enumerate(self.subcons):
            try:
                subobj = next(objiter)
                if sc.name:
                    context[sc.name] = subobj

                buildret = sc._build(subobj, stream, context, path)
                retlist.append(buildret)

                if sc.name:
                    context[sc.name] = buildret
            except StopFieldError:
                break
        return retlist

    def _sizeof(self, context, path):
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = None, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        try:
            return sum(sc._sizeof(context, path) for sc in self.subcons)
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)

    def _emitparse(self, code):
        fname = f"parse_sequence_{code.allocateId()}"
        block = f"""
            def {fname}(io, this):
                result = ListContainer()
                this = Container(_ = this, _params = this['_params'], _root = None, _parsing = True, _building = False, _sizing = False, _subcons = None, _io = io, _index = this.get('_index', None))
                this['_root'] = this['_'].get('_root', this)
                try:
        """
        for sc in self.subcons:
            block += f"""
                    result.append({sc._compileparse(code)})
            """
            if sc.name:
                block += f"""
                    this[{repr(sc.name)}] = result[-1]
                """
        block += f"""
                    pass
                except StopFieldError:
                    pass
                return result
        """
        code.append(block)
        return f"{fname}(io, this)"

    def _emitbuild(self, code):
        fname = f"build_sequence_{code.allocateId()}"
        block = f"""
            def {fname}(obj, io, this):
                this = Container(_ = this, _params = this['_params'], _root = None, _parsing = False, _building = True, _sizing = False, _subcons = None, _io = io, _index = this.get('_index', None))
                this['_root'] = this['_'].get('_root', this)
                try:
                    objiter = iter(obj)
                    retlist = ListContainer()
        """
        for sc in self.subcons:
            block += f"""
                    {f'obj = next(objiter)'}
                    {f'this[{repr(sc.name)}] = obj' if sc.name else ''}
                    {f'x = '}{sc._compilebuild(code)}
                    {f'retlist.append(x)'}
                    {f'this[{repr(sc.name)}] = x' if sc.name else ''}
            """
        block += f"""
                    pass
                except StopFieldError:
                    pass
                return retlist
        """
        code.append(block)
        return f"{fname}(obj, io, this)"

    def _emitseq(self, ksy, bitwise):
        return [sc._compilefulltype(ksy, bitwise) for sc in self.subcons]


#===============================================================================
# arrays ranges and repeaters
#===============================================================================
class Array(Subconstruct):
    r"""
    Homogenous array of elements, similar to C# generic T[].

    Parses into a ListContainer (a list). Parsing and building processes an exact amount of elements. If given list has more or less than count elements, raises RangeError. Size is defined as count multiplied by subcon size, but only if subcon is fixed size.

    Operator [] can be used to make Array instances (recommended syntax).

    :param count: integer or context lambda, strict amount of elements
    :param subcon: Construct instance, subcon to process individual elements
    :param discard: optional, bool, if set then parsing returns empty list

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises RangeError: specified count is not valid
    :raises RangeError: given object has different length than specified count

    Can propagate any exception from the lambdas, possibly non-ConstructError.

    Example::

        >>> d = Array(5, Byte) or Byte[5]
        >>> d.build(range(5))
        b'\x00\x01\x02\x03\x04'
        >>> d.parse(_)
        [0, 1, 2, 3, 4]
    """

    def __init__(self, count, subcon, discard=False):
        super().__init__(subcon)
        self.count = count
        self.discard = discard

    def _parse(self, stream, context, path):
        count = evaluate(self.count, context)
        if not 0 <= count:
            raise RangeError("invalid count %s" % (count,), path=path)
        discard = self.discard
        obj = ListContainer()
        for i in range(count):
            context._index = i
            e = self.subcon._parsereport(stream, context, path)
            if not discard:
                obj.append(e)
        return obj

    def _build(self, obj, stream, context, path):
        count = evaluate(self.count, context)
        if not 0 <= count:
            raise RangeError("invalid count %s" % (count,), path=path)
        if not len(obj) == count:
            raise RangeError("expected %d elements, found %d" % (count, len(obj)), path=path)
        discard = self.discard
        retlist = ListContainer()
        for i,e in enumerate(obj):
            context._index = i
            buildret = self.subcon._build(e, stream, context, path)
            if not discard:
                retlist.append(buildret)
        return retlist

    def _sizeof(self, context, path):
        try:
            count = evaluate(self.count, context)
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)
        return count * self.subcon._sizeof(context, path)

    def _emitparse(self, code):
        return f"ListContainer(({self.subcon._compileparse(code)}) for i in range({self.count}))"

    def _emitbuild(self, code):
        return f"ListContainer(reuse(obj[i], lambda obj: ({self.subcon._compilebuild(code)})) for i in range({self.count}))"

    def _emitfulltype(self, ksy, bitwise):
        return dict(type=self.subcon._compileprimitivetype(ksy, bitwise), repeat="expr", repeat_expr=self.count)


class GreedyRange(Subconstruct):
    r"""
    Homogenous array of elements, similar to C# generic IEnumerable<T>, but works with unknown count of elements by parsing until end of stream.

    Parses into a ListContainer (a list). Parsing stops when an exception occured when parsing the subcon, either due to EOF or subcon format not being able to parse the data. Either way, when GreedyRange encounters either failure it seeks the stream back to a position after last successful subcon parsing. Builds from enumerable, each element as-is. Size is undefined.

    This class supports stopping. If :class:`~construct.core.StopIf` field is a member, and it evaluates its lambda as positive, this class ends parsing or building as successful without processing further fields.

    :param subcon: Construct instance, subcon to process individual elements
    :param discard: optional, bool, if set then parsing returns empty list

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises StreamError: stream is not seekable and tellable

    Can propagate any exception from the lambdas, possibly non-ConstructError.

    Example::

        >>> d = GreedyRange(Byte)
        >>> d.build(range(8))
        b'\x00\x01\x02\x03\x04\x05\x06\x07'
        >>> d.parse(_)
        [0, 1, 2, 3, 4, 5, 6, 7]
    """

    def __init__(self, subcon, discard=False):
        super().__init__(subcon)
        self.discard = discard

    def _parse(self, stream, context, path):
        discard = self.discard
        obj = ListContainer()
        try:
            for i in itertools.count():
                context._index = i
                fallback = stream_tell(stream, path)
                e = self.subcon._parsereport(stream, context, path)
                if not discard:
                    obj.append(e)
        except StopFieldError:
            pass
        except ExplicitError:
            raise
        except Exception:
            stream_seek(stream, fallback, 0, path)
        return obj

    def _build(self, obj, stream, context, path):
        discard = self.discard
        try:
            retlist = ListContainer()
            for i,e in enumerate(obj):
                context._index = i
                buildret = self.subcon._build(e, stream, context, path)
                if not discard:
                    retlist.append(buildret)
            return retlist
        except StopFieldError:
            pass

    def _sizeof(self, context, path):
        raise SizeofError(path=path)

    def _emitfulltype(self, ksy, bitwise):
        return dict(type=self.subcon._compileprimitivetype(ksy, bitwise), repeat="eos")


class RepeatUntil(Subconstruct):
    r"""
    Homogenous array of elements, similar to C# generic IEnumerable<T>, that repeats until the predicate indicates it to stop. Note that the last element (that predicate indicated as True) is included in the return list.

    Parse iterates indefinately until last element passed the predicate. Build iterates indefinately over given list, until an element passed the precicate (or raises RepeatError if no element passed it). Size is undefined.

    :param predicate: lambda that takes (obj, list, context) and returns True to break or False to continue (or a truthy value)
    :param subcon: Construct instance, subcon used to parse and build each element
    :param discard: optional, bool, if set then parsing returns empty list

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises RepeatError: consumed all elements in the stream but neither passed the predicate

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = RepeatUntil(lambda x,lst,ctx: x > 7, Byte)
        >>> d.build(range(20))
        b'\x00\x01\x02\x03\x04\x05\x06\x07\x08'
        >>> d.parse(b"\x01\xff\x02")
        [1, 255]

        >>> d = RepeatUntil(lambda x,lst,ctx: lst[-2:] == [0,0], Byte)
        >>> d.parse(b"\x01\x00\x00\xff")
        [1, 0, 0]
    """

    def __init__(self, predicate, subcon, discard=False):
        super().__init__(subcon)
        self.predicate = predicate
        self.discard = discard

    def _parse(self, stream, context, path):
        predicate = self.predicate
        discard = self.discard
        if not callable(predicate):
            predicate = lambda _1,_2,_3: predicate
        obj = ListContainer()
        for i in itertools.count():
            context._index = i
            e = self.subcon._parsereport(stream, context, path)
            if not discard:
                obj.append(e)
            if predicate(e, obj, context):
                return obj

    def _build(self, obj, stream, context, path):
        predicate = self.predicate
        discard = self.discard
        if not callable(predicate):
            predicate = lambda _1,_2,_3: predicate
        partiallist = ListContainer()
        retlist = ListContainer()
        for i,e in enumerate(obj):
            context._index = i
            buildret = self.subcon._build(e, stream, context, path)
            if not discard:
                retlist.append(buildret)
                partiallist.append(buildret)
            if predicate(e, partiallist, context):
                break
        else:
            raise RepeatError("expected any item to match predicate, when building", path=path)
        return retlist

    def _sizeof(self, context, path):
        raise SizeofError("cannot calculate size, amount depends on actual data", path=path)

    def _emitparse(self, code):
        fname = f"parse_repeatuntil_{code.allocateId()}"
        block = f"""
            def {fname}(io, this):
                list_ = ListContainer()
                while True:
                    obj_ = {self.subcon._compileparse(code)}
                    if not ({self.discard}):
                        list_.append(obj_)
                    if ({self.predicate}):
                        return list_
        """
        code.append(block)
        return f"{fname}(io, this)"

    def _emitbuild(self, code):
        fname = f"build_repeatuntil_{code.allocateId()}"
        block = f"""
            def {fname}(obj, io, this):
                objiter = iter(obj)
                list_ = ListContainer()
                while True:
                    obj_ = reuse(next(objiter), lambda obj: {self.subcon._compilebuild(code)})
                    list_.append(obj_)
                    if ({self.predicate}):
                        return list_
        """
        code.append(block)
        return f"{fname}(obj, io, this)"

    def _emitfulltype(self, ksy, bitwise):
        return dict(type=self.subcon._compileprimitivetype(ksy, bitwise), repeat="until", repeat_until=repr(self.predicate).replace("obj_","_"))


#===============================================================================
# specials
#===============================================================================
class Renamed(Subconstruct):
    r"""
    Special wrapper that allows a Struct (or other similar class) to see a field as having a name (or a different name) or having a parsed hook. Library classes do not have names (its None). Renamed does not change a field, only wraps it like a candy with a label. Used internally by / and * operators.

    Also this wrapper is responsible for building a path info (a chain of names) that gets attached to error message when parsing, building, or sizeof fails. Fields that are not named do not appear in the path string.

    Parsing building and size are deferred to subcon.

    :param subcon: Construct instance
    :param newname: optional, string
    :param newdocs: optional, string
    :param newparsed: optional, lambda

    Example::

        >>> "number" / Int32ub
        <Renamed: number>
    """

    def __init__(self, subcon, newname=None, newdocs=None, newparsed=None):
        super().__init__(subcon)
        self.name = newname if newname else subcon.name
        self.docs = newdocs if newdocs else subcon.docs
        self.parsed = newparsed if newparsed else subcon.parsed

    def __getattr__(self, name):
        return getattr(self.subcon, name)

    def _parse(self, stream, context, path):
        path += " -> %s" % (self.name,)
        return self.subcon._parsereport(stream, context, path)

    def _build(self, obj, stream, context, path):
        path += " -> %s" % (self.name,)
        return self.subcon._build(obj, stream, context, path)

    def _sizeof(self, context, path):
        path += " -> %s" % (self.name,)
        return self.subcon._sizeof(context, path)

    def _emitparse(self, code):
        return self.subcon._compileparse(code)

    def _emitbuild(self, code):
        return self.subcon._compilebuild(code)

    def _emitseq(self, ksy, bitwise):
        return self.subcon._compileseq(ksy, bitwise)

    def _emitprimitivetype(self, ksy, bitwise):
        return self.subcon._compileprimitivetype(ksy, bitwise)

    def _emitfulltype(self, ksy, bitwise):
        r = dict()
        if self.name:
            r.update(id=self.name)
        r.update(self.subcon._compilefulltype(ksy, bitwise))
        if self.docs:
            r.update(doc=self.docs)
        return r


#===============================================================================
# miscellaneous
#===============================================================================
class Const(Subconstruct):
    r"""
    Field enforcing a constant. It is used for file signatures, to validate that the given pattern exists. Data in the stream must strictly match the specified value.

    Note that a variable sized subcon may still provide positive verification. Const does not consume a precomputed amount of bytes, but depends on the subcon to read the appropriate amount (eg. VarInt is acceptable). Whatever subcon parses into, gets compared against the specified value.

    Parses using subcon and return its value (after checking). Builds using subcon from nothing (or given object, if not None). Size is the same as subcon, unless it raises SizeofError.

    :param value: expected value, usually a bytes literal
    :param subcon: optional, Construct instance, subcon used to build value from, assumed to be Bytes if value parameter was a bytes literal

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises ConstError: parsed data does not match specified value, or building from wrong value
    :raises StringError: building from non-bytes value, perhaps unicode

    Example::

        >>> d = Const(b"IHDR")
        >>> d.build(None)
        b'IHDR'
        >>> d.parse(b"JPEG")
        construct.core.ConstError: expected b'IHDR' but parsed b'JPEG'

        >>> d = Const(255, Int32ul)
        >>> d.build(None)
        b'\xff\x00\x00\x00'
    """

    def __init__(self, value, subcon=None):
        if subcon is None:
            if not isinstance(value, bytestringtype):
                raise StringError(f"given non-bytes value {repr(value)}, perhaps unicode?")
            subcon = Bytes(len(value))
        super().__init__(subcon)
        self.value = value
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        obj = self.subcon._parsereport(stream, context, path)
        if not obj == self.value:
            raise ConstError(f"parsing expected {repr(self.value)} but parsed {repr(obj)}", path=path)
        return obj

    def _build(self, obj, stream, context, path):
        if obj not in (None, self.value):
            raise ConstError(f"building expected None or {repr(self.value)} but got {repr(obj)}", path=path)
        return self.subcon._build(self.value, stream, context, path)

    def _sizeof(self, context, path):
        return self.subcon._sizeof(context, path)

    def _emitparse(self, code):
        code.append(f"""
            def parse_const(value, expected):
                if not value == expected: raise ConstError
                return value
        """)
        return f"parse_const({self.subcon._compileparse(code)}, {repr(self.value)})"

    def _emitbuild(self, code):
        if isinstance(self.value, bytes):
            return f"(io.write({repr(self.value)}), {repr(self.value)})[1]"
        else:
            return f"reuse({repr(self.value)}, lambda obj: {self.subcon._compilebuild(code)})"

    def _emitfulltype(self, ksy, bitwise):
        data = self.subcon.build(self.value)
        return dict(contents=list(data))


class Computed(Construct):
    r"""
    Field computing a value from the context dictionary or some outer source like os.urandom or random module. Underlying byte stream is unaffected. The source can be non-deterministic.

    Parsing and Building return the value returned by the context lambda (although a constant value can also be used). Size is defined as 0 because parsing and building does not consume or produce bytes into the stream.

    :param func: context lambda or constant value

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::
        >>> d = Struct(
        ...     "width" / Byte,
        ...     "height" / Byte,
        ...     "total" / Computed(this.width * this.height),
        ... )
        >>> d.build(dict(width=4,height=5))
        b'\x04\x05'
        >>> d.parse(b"12")
        Container(width=49, height=50, total=2450)

        >>> d = Computed(7)
        >>> d.parse(b"")
        7
        >>> d = Computed(lambda ctx: 7)
        >>> d.parse(b"")
        7

        >>> import os
        >>> d = Computed(lambda ctx: os.urandom(10))
        >>> d.parse(b"")
        b'\x98\xc2\xec\x10\x07\xf5\x8e\x98\xc2\xec'
    """

    def __init__(self, func):
        super().__init__()
        self.func = func
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        return self.func(context) if callable(self.func) else self.func

    def _build(self, obj, stream, context, path):
        return self.func(context) if callable(self.func) else self.func

    def _sizeof(self, context, path):
        return 0

    def _emitparse(self, code):
        return repr(self.func)

    def _emitbuild(self, code):
        return repr(self.func)


@singleton
class Index(Construct):
    r"""
    Indexes a field inside outer :class:`~construct.core.Array` :class:`~construct.core.GreedyRange` :class:`~construct.core.RepeatUntil` context.

    Note that you can use this class, or use `this._index` expression instead, depending on how its used. See the examples.

    Parsing and building pulls _index key from the context. Size is 0 because stream is unaffected.

    :raises IndexFieldError: did not find either key in context

    Example::

        >>> d = Array(3, Index)
        >>> d.parse(b"")
        [0, 1, 2]
        >>> d = Array(3, Struct("i" / Index))
        >>> d.parse(b"")
        [Container(i=0), Container(i=1), Container(i=2)]

        >>> d = Array(3, Computed(this._index+1))
        >>> d.parse(b"")
        [1, 2, 3]
        >>> d = Array(3, Struct("i" / Computed(this._._index+1)))
        >>> d.parse(b"")
        [Container(i=1), Container(i=2), Container(i=3)]
    """

    def __init__(self):
        super().__init__()
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        return context.get("_index", None)

    def _build(self, obj, stream, context, path):
        return context.get("_index", None)

    def _sizeof(self, context, path):
        return 0


class Rebuild(Subconstruct):
    r"""
    Field where building does not require a value, because the value gets recomputed when needed. Comes handy when building a Struct from a dict with missing keys. Useful for length and count fields when :class:`~construct.core.Prefixed` and :class:`~construct.core.PrefixedArray` cannot be used.

    Parsing defers to subcon. Building is defered to subcon, but it builds from a value provided by the context lambda (or constant). Size is the same as subcon, unless it raises SizeofError.

    Difference between Default and Rebuild, is that in first the build value is optional and in second the build value is ignored.

    :param subcon: Construct instance
    :param func: context lambda or constant value

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Struct(
        ...     "count" / Rebuild(Byte, len_(this.items)),
        ...     "items" / Byte[this.count],
        ... )
        >>> d.build(dict(items=[1,2,3]))
        b'\x03\x01\x02\x03'
    """

    def __init__(self, subcon, func):
        super().__init__(subcon)
        self.func = func
        self.flagbuildnone = True

    def _build(self, obj, stream, context, path):
        obj = evaluate(self.func, context)
        return self.subcon._build(obj, stream, context, path)

    def _emitparse(self, code):
        return self.subcon._compileparse(code)

    def _emitbuild(self, code):
        return f"reuse({repr(self.func)}, lambda obj: ({self.subcon._compilebuild(code)}))"

    def _emitseq(self, ksy, bitwise):
        return self.subcon._compileseq(ksy, bitwise)

    def _emitprimitivetype(self, ksy, bitwise):
        return self.subcon._compileprimitivetype(ksy, bitwise)

    def _emitfulltype(self, ksy, bitwise):
        return self.subcon._compilefulltype(ksy, bitwise)


class Default(Subconstruct):
    r"""
    Field where building does not require a value, because the value gets taken from default. Comes handy when building a Struct from a dict with missing keys.

    Parsing defers to subcon. Building is defered to subcon, but it builds from a default (if given object is None) or from given object. Building does not require a value, but can accept one. Size is the same as subcon, unless it raises SizeofError.

    Difference between Default and Rebuild, is that in first the build value is optional and in second the build value is ignored.

    :param subcon: Construct instance
    :param value: context lambda or constant value

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Struct(
        ...     "a" / Default(Byte, 0),
        ... )
        >>> d.build(dict(a=1))
        b'\x01'
        >>> d.build(dict())
        b'\x00'
    """

    def __init__(self, subcon, value):
        super().__init__(subcon)
        self.value = value
        self.flagbuildnone = True

    def _build(self, obj, stream, context, path):
        obj = evaluate(self.value, context) if obj is None else obj
        return self.subcon._build(obj, stream, context, path)

    def _emitparse(self, code):
        return self.subcon._compileparse(code)

    def _emitbuild(self, code):
        return f"reuse({repr(self.value)} if obj is None else obj, lambda obj: ({self.subcon._compilebuild(code)}))"

    def _emitseq(self, ksy, bitwise):
        return self.subcon._compileseq(ksy, bitwise)

    def _emitprimitivetype(self, ksy, bitwise):
        return self.subcon._compileprimitivetype(ksy, bitwise)

    def _emitfulltype(self, ksy, bitwise):
        return self.subcon._compilefulltype(ksy, bitwise)


class Check(Construct):
    r"""
    Checks for a condition, and raises CheckError if the check fails.

    Parsing and building return nothing (but check the condition). Size is 0 because stream is unaffected.

    :param func: bool or context lambda, that gets run on parsing and building

    :raises CheckError: lambda returned false

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        Check(lambda ctx: len(ctx.payload.data) == ctx.payload_len)
        Check(len_(this.payload.data) == this.payload_len)
    """

    def __init__(self, func):
        super().__init__()
        self.func = func
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        passed = evaluate(self.func, context)
        if not passed:
            raise CheckError("check failed during parsing", path=path)

    def _build(self, obj, stream, context, path):
        passed = evaluate(self.func, context)
        if not passed:
            raise CheckError("check failed during building", path=path)

    def _sizeof(self, context, path):
        return 0

    def _emitparse(self, code):
        code.append(f"""
            def parse_check(condition):
                if not condition: raise CheckError
        """)
        return f"parse_check({repr(self.func)})"

    def _emitbuild(self, code):
        code.append(f"""
            def build_check(condition):
                if not condition: raise CheckError
        """)
        return f"build_check({repr(self.func)})"


@singleton
class Error(Construct):
    r"""
    Raises ExplicitError, unconditionally.

    Parsing and building always raise ExplicitError. Size is undefined.

    :raises ExplicitError: unconditionally, on parsing and building

    Example::

        >>> d = Struct("num"/Byte, Error)
        >>> d.parse(b"data...")
        construct.core.ExplicitError: Error field was activated during parsing
    """

    def __init__(self):
        super().__init__()
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        raise ExplicitError("Error field was activated during parsing", path=path)

    def _build(self, obj, stream, context, path):
        raise ExplicitError("Error field was activated during building", path=path)

    def _sizeof(self, context, path):
        raise SizeofError("Error does not have size, because it interrupts parsing and building", path=path)

    def _emitparse(self, code):
        code.append("""
            def parse_error():
                raise ExplicitError
        """)
        return "parse_error()"

    def _emitbuild(self, code):
        code.append("""
            def build_error():
                raise ExplicitError
        """)
        return "build_error()"


class FocusedSeq(Construct):
    r"""
    Allows constructing more elaborate "adapters" than Adapter class.

    Parse does parse all subcons in sequence, but returns only the element that was selected (discards other values). Build does build all subcons in sequence, where each gets build from nothing (except the selected subcon which is given the object). Size is the sum of all subcon sizes, unless any subcon raises SizeofError.

    This class does context nesting, meaning its members are given access to a new dictionary where the "_" entry points to the outer context. When parsing, each member gets parsed and subcon parse return value is inserted into context under matching key only if the member was named. When building, the matching entry gets inserted into context before subcon gets build, and if subcon build returns a new value (not None) that gets replaced in the context.

    This class exposes subcons as attributes. You can refer to subcons that were inlined (and therefore do not exist as variable in the namespace) by accessing the struct attributes, under same name. Also note that compiler does not support this feature. See examples.

    This class exposes subcons in the context. You can refer to subcons that were inlined (and therefore do not exist as variable in the namespace) within other inlined fields using the context. Note that you need to use a lambda (`this` expression is not supported). Also note that compiler does not support this feature. See examples.

    This class is used internally to implement :class:`~construct.core.PrefixedArray`.

    :param parsebuildfrom: string name or context lambda, selects a subcon
    :param \*subcons: Construct instances, list of members, some can be named
    :param \*\*subconskw: Construct instances, list of members (requires Python 3.6)

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises UnboundLocalError: selector does not match any subcon

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Excample::

        >>> d = FocusedSeq("num", Const(b"SIG"), "num"/Byte, Terminated)
        >>> d.parse(b"SIG\xff")
        255
        >>> d.build(255)
        b'SIG\xff'

        >>> d = FocusedSeq("animal",
        ...     "animal" / Enum(Byte, giraffe=1),
        ... )
        >>> d.animal.giraffe
        'giraffe'
        >>> d = FocusedSeq("count",
        ...     "count" / Byte,
        ...     "data" / Padding(lambda this: this.count - this._subcons.count.sizeof()),
        ... )
        >>> d.build(4)
        b'\x04\x00\x00\x00'

        PrefixedArray <--> FocusedSeq("items",
            "count" / Rebuild(lengthfield, len_(this.items)),
            "items" / subcon[this.count],
        )
    """

    def __init__(self, parsebuildfrom, *subcons, **subconskw):
        super().__init__()
        self.parsebuildfrom = parsebuildfrom
        self.subcons = list(subcons) + list(k/v for k,v in subconskw.items())
        self._subcons = Container((sc.name,sc) for sc in self.subcons if sc.name)

    def __getattr__(self, name):
        if name in self._subcons:
            return self._subcons[name]
        raise AttributeError

    def _parse(self, stream, context, path):
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        parsebuildfrom = evaluate(self.parsebuildfrom, context)
        for i,sc in enumerate(self.subcons):
            parseret = sc._parsereport(stream, context, path)
            if sc.name:
                context[sc.name] = parseret
            if sc.name == parsebuildfrom:
                finalret = parseret
        return finalret

    def _build(self, obj, stream, context, path):
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        parsebuildfrom = evaluate(self.parsebuildfrom, context)
        context[parsebuildfrom] = obj
        for i,sc in enumerate(self.subcons):
            buildret = sc._build(obj if sc.name == parsebuildfrom else None, stream, context, path)
            if sc.name:
                context[sc.name] = buildret
            if sc.name == parsebuildfrom:
                finalret = buildret
        return finalret

    def _sizeof(self, context, path):
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = None, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        try:
            return sum(sc._sizeof(context, path) for sc in self.subcons)
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)

    def _emitparse(self, code):
        fname = f"parse_focusedseq_{code.allocateId()}"
        block = f"""
            def {fname}(io, this):
                result = []
                this = Container(_ = this, _params = this['_params'], _root = None, _parsing = True, _building = False, _sizing = False, _subcons = None, _io = io, _index = this.get('_index', None))
                this['_root'] = this['_'].get('_root', this)
        """
        for sc in self.subcons:
            block += f"""
                result.append({sc._compileparse(code)})
            """
            if sc.name:
                block += f"""
                this[{repr(sc.name)}] = result[-1]
                """
        block += f"""
                return this[{repr(self.parsebuildfrom)}]
        """
        code.append(block)
        return f"{fname}(io, this)"

    def _emitbuild(self, code):
        fname = f"build_focusedseq_{code.allocateId()}"
        block = f"""
            def {fname}(obj, io, this):
                this = Container(_ = this, _params = this['_params'], _root = None, _parsing = False, _building = True, _sizing = False, _subcons = None, _io = io, _index = this.get('_index', None))
                this['_root'] = this['_'].get('_root', this)
                try:
                    this[{repr(self.parsebuildfrom)}] = obj
                    finalobj = obj
        """
        for sc in self.subcons:
            block += f"""
                    {f'obj = {"finalobj" if sc.name == self.parsebuildfrom else "None"}'}
                    {f'buildret = '}{sc._compilebuild(code)}
                    {f'this[{repr(sc.name)}] = buildret' if sc.name else ''}
                    {f'{"finalret = buildret" if sc.name == self.parsebuildfrom else ""}'}
            """
        block += f"""
                    pass
                except StopFieldError:
                    pass
                return finalret
        """
        code.append(block)
        return f"{fname}(obj, io, this)"

    def _emitseq(self, ksy, bitwise):
        return [sc._compilefulltype(ksy, bitwise) for sc in self.subcons]


@singleton
class Pickled(Construct):
    r"""
    Preserves arbitrary Python objects.

    Parses using `pickle.load() <https://docs.python.org/3/library/pickle.html#pickle.load>`_ and builds using `pickle.dump() <https://docs.python.org/3/library/pickle.html#pickle.dump>`_ functions, using default Pickle binary protocol. Size is undefined.

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes

    Can propagate pickle.load() and pickle.dump() exceptions.

    Example::

        >>> x = [1, 2.3, {}]
        >>> Pickled.build(x)
        b'\x80\x03]q\x00(K\x01G@\x02ffffff}q\x01e.'
        >>> Pickled.parse(_)
        [1, 2.3, {}]
    """

    def _parse(self, stream, context, path):
        return pickle.load(stream)

    def _build(self, obj, stream, context, path):
        pickle.dump(obj, stream)
        return obj


@singleton
class Numpy(Construct):
    r"""
    Preserves numpy arrays (both shape, dtype and values).

    Parses using `numpy.load() <https://docs.scipy.org/doc/numpy/reference/generated/numpy.load.html#numpy.load>`_ and builds using `numpy.save() <https://docs.scipy.org/doc/numpy/reference/generated/numpy.save.html#numpy.save>`_ functions, using Numpy binary protocol. Size is undefined.

    :raises ImportError: numpy could not be imported during parsing or building
    :raises ValueError: could not read enough bytes, or so

    Can propagate numpy.load() and numpy.save() exceptions.

    Example::

        >>> import numpy
        >>> a = numpy.asarray([1,2,3])
        >>> Numpy.build(a)
        b"\x93NUMPY\x01\x00F\x00{'descr': '<i8', 'fortran_order': False, 'shape': (3,), }            \n\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00"
        >>> Numpy.parse(_)
        array([1, 2, 3])
    """

    def _parse(self, stream, context, path):
        import numpy
        return numpy.load(stream)

    def _build(self, obj, stream, context, path):
        import numpy
        numpy.save(stream, obj)
        return obj


class NamedTuple(Adapter):
    r"""
    Both arrays, structs, and sequences can be mapped to a namedtuple from `collections module <https://docs.python.org/3/library/collections.html#collections.namedtuple>`_. To create a named tuple, you need to provide a name and a sequence of fields, either a string with space-separated names or a list of string names, like the standard namedtuple.

    Parses into a collections.namedtuple instance, and builds from such instance (although it also builds from lists and dicts). Size is undefined.

    :param tuplename: string
    :param tuplefields: string or list of strings
    :param subcon: Construct instance, either Struct Sequence Array GreedyRange

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises NamedTupleError: subcon is neither Struct Sequence Array GreedyRange

    Can propagate collections exceptions.

    Example::

        >>> d = NamedTuple("coord", "x y z", Byte[3])
        >>> d = NamedTuple("coord", "x y z", Byte >> Byte >> Byte)
        >>> d = NamedTuple("coord", "x y z", "x"/Byte + "y"/Byte + "z"/Byte)
        >>> d.parse(b"123")
        coord(x=49, y=50, z=51)
    """

    def __init__(self, tuplename, tuplefields, subcon):
        if not isinstance(subcon, (Struct,Sequence,Array,GreedyRange)):
            raise NamedTupleError("subcon is neither Struct Sequence Array GreedyRange")
        super().__init__(subcon)
        self.tuplename = tuplename
        self.tuplefields = tuplefields
        self.factory = collections.namedtuple(tuplename, tuplefields)

    def _decode(self, obj, context, path):
        if isinstance(self.subcon, Struct):
            del obj["_io"]
            return self.factory(**obj)
        if isinstance(self.subcon, (Sequence,Array,GreedyRange)):
            return self.factory(*obj)
        raise NamedTupleError("subcon is neither Struct Sequence Array GreedyRangeGreedyRange", path=path)

    def _encode(self, obj, context, path):
        if isinstance(self.subcon, Struct):
            return Container({sc.name:getattr(obj,sc.name) for sc in self.subcon.subcons if sc.name})
        if isinstance(self.subcon, (Sequence,Array,GreedyRange)):
            return list(obj)
        raise NamedTupleError("subcon is neither Struct Sequence Array GreedyRange", path=path)

    def _emitparse(self, code):
        fname = "factory_%s" % code.allocateId()
        code.append("""
            %s = collections.namedtuple(%r, %r)
        """ % (fname, self.tuplename, self.tuplefields, ))
        if isinstance(self.subcon, Struct):
            return "%s(**(%s))" % (fname, self.subcon._compileparse(code), )
        if isinstance(self.subcon, (Sequence,Array,GreedyRange)):
            return "%s(*(%s))" % (fname, self.subcon._compileparse(code), )
        raise NamedTupleError("subcon is neither Struct Sequence Array GreedyRange")

    def _emitseq(self, ksy, bitwise):
        return self.subcon._compileseq(ksy, bitwise)

    def _emitprimitivetype(self, ksy, bitwise):
        return self.subcon._compileprimitivetype(ksy, bitwise)

    def _emitfulltype(self, ksy, bitwise):
        return self.subcon._compilefulltype(ksy, bitwise)


class TimestampAdapter(Adapter):
    """Used internally."""


def Timestamp(subcon, unit, epoch):
    r"""
    Datetime, represented as `Arrow <https://pypi.org/project/arrow/>`_ object.

    Note that accuracy is not guaranteed, because building rounds the value to integer (even when Float subcon is used), due to floating-point errors in general, and because MSDOS scheme has only 5-bit (32 values) seconds field (seconds are rounded to multiple of 2).

    Unit is a fraction of a second. 1 is second resolution, 10**-3 is milliseconds resolution, 10**-6 is microseconds resolution, etc. Usually its 1 on Unix and MacOSX, 10**-7 on Windows. Epoch is a year (if integer) or a specific day (if Arrow object). Usually its 1970 on Unix, 1904 on MacOSX, 1600 on Windows. MSDOS format doesnt support custom unit or epoch, it uses 2-seconds resolution and 1980 epoch.

    :param subcon: Construct instance like Int* Float*, or Int32ub with msdos format
    :param unit: integer or float, or msdos string
    :param epoch: integer, or Arrow instance, or msdos string

    :raises ImportError: arrow could not be imported during ctor
    :raises TimestampError: subcon is not a Construct instance
    :raises TimestampError: unit or epoch is a wrong type

    Example::

        >>> d = Timestamp(Int64ub, 1., 1970)
        >>> d.parse(b'\x00\x00\x00\x00ZIz\x00')
        <Arrow [2018-01-01T00:00:00+00:00]>
        >>> d = Timestamp(Int32ub, "msdos", "msdos")
        >>> d.parse(b'H9\x8c"')
        <Arrow [2016-01-25T17:33:04+00:00]>
    """
    import arrow

    if not isinstance(subcon, Construct):
        raise TimestampError("subcon should be Int*, experimentally Float*, or Int32ub when using msdos format")
    if not isinstance(unit, (integertypes, float, stringtypes)):
        raise TimestampError("unit must be one of: int float string")
    if not isinstance(epoch, (integertypes, arrow.Arrow, stringtypes)):
        raise TimestampError("epoch must be one of: int Arrow string")

    if unit == "msdos" or epoch == "msdos":
        st = BitStruct(
            "year" / BitsInteger(7),
            "month" / BitsInteger(4),
            "day" / BitsInteger(5),
            "hour" / BitsInteger(5),
            "minute" / BitsInteger(6),
            "second" / BitsInteger(5),
        )
        class MsdosTimestampAdapter(TimestampAdapter):
            def _decode(self, obj, context, path):
                return arrow.Arrow(1980,1,1).shift(years=obj.year, months=obj.month-1, days=obj.day-1, hours=obj.hour, minutes=obj.minute, seconds=obj.second*2)
            def _encode(self, obj, context, path):
                t = obj.timetuple()
                return Container(year=t.tm_year-1980, month=t.tm_mon, day=t.tm_mday, hour=t.tm_hour, minute=t.tm_min, second=t.tm_sec//2)
        macro = MsdosTimestampAdapter(st)

    else:
        if isinstance(epoch, integertypes):
            epoch = arrow.Arrow(epoch, 1, 1)
        class EpochTimestampAdapter(TimestampAdapter):
            def _decode(self, obj, context, path):
                return epoch.shift(seconds=obj*unit)
            def _encode(self, obj, context, path):
                return int((obj-epoch).total_seconds()/unit)
        macro = EpochTimestampAdapter(subcon)

    def _emitfulltype(ksy, bitwise):
        return subcon._compilefulltype(ksy, bitwise)
    def _emitprimitivetype(ksy, bitwise):
        return subcon._compileprimitivetype(ksy, bitwise)
    macro._emitfulltype = _emitfulltype
    macro._emitprimitivetype = _emitprimitivetype
    return macro


class Hex(Adapter):
    r"""
    Adapter for displaying hexadecimal/hexlified representation of integers/bytes/RawCopy dictionaries.

    Parsing results in int-alike bytes-alike or dict-alike object, whose only difference from original is pretty-printing. If you look at the result, you will be presented with its `repr` which remains as-is. If you print it, then you will see its `str` whic is a hexlified representation. Building and sizeof defer to subcon.

    To obtain a hexlified string (like before Hex HexDump changed semantics) use binascii.(un)hexlify on parsed results.

    Example::

        >>> d = Hex(Int32ub)
        >>> obj = d.parse(b"\x00\x00\x01\x02")
        >>> obj
        258
        >>> print(obj)
        0x00000102

        >>> d = Hex(GreedyBytes)
        >>> obj = d.parse(b"\x00\x00\x01\x02")
        >>> obj
        b'\x00\x00\x01\x02'
        >>> print(obj)
        unhexlify('00000102')

        >>> d = Hex(RawCopy(Int32ub))
        >>> obj = d.parse(b"\x00\x00\x01\x02")
        >>> obj
        {'data': b'\x00\x00\x01\x02',
         'length': 4,
         'offset1': 0,
         'offset2': 4,
         'value': 258}
        >>> print(obj)
        unhexlify('00000102')
    """
    def _decode(self, obj, context, path):
        if isinstance(obj, integertypes):
            return HexDisplayedInteger.new(obj, "0%sX" % (2 * self.subcon._sizeof(context, path)))
        if isinstance(obj, bytestringtype):
            return HexDisplayedBytes(obj)
        if isinstance(obj, dict):
            return HexDisplayedDict(obj)
        return obj

    def _encode(self, obj, context, path):
        return obj

    def _emitparse(self, code):
        return self.subcon._compileparse(code)

    def _emitseq(self, ksy, bitwise):
        return self.subcon._compileseq(ksy, bitwise)

    def _emitprimitivetype(self, ksy, bitwise):
        return self.subcon._compileprimitivetype(ksy, bitwise)

    def _emitfulltype(self, ksy, bitwise):
        return self.subcon._compilefulltype(ksy, bitwise)


class HexDump(Adapter):
    r"""
    Adapter for displaying hexlified representation of bytes/RawCopy dictionaries.

    Parsing results in bytes-alike or dict-alike object, whose only difference from original is pretty-printing. If you look at the result, you will be presented with its `repr` which remains as-is. If you print it, then you will see its `str` whic is a hexlified representation. Building and sizeof defer to subcon.

    To obtain a hexlified string (like before Hex HexDump changed semantics) use construct.lib.hexdump on parsed results.

    Example::

        >>> d = HexDump(GreedyBytes)
        >>> obj = d.parse(b"\x00\x00\x01\x02")
        >>> obj
        b'\x00\x00\x01\x02'
        >>> print(obj)
        hexundump('''
        0000   00 00 01 02                                       ....
        ''')

        >>> d = HexDump(RawCopy(Int32ub))
        >>> obj = d.parse(b"\x00\x00\x01\x02")
        >>> obj
        {'data': b'\x00\x00\x01\x02',
         'length': 4,
         'offset1': 0,
         'offset2': 4,
         'value': 258}
        >>> print(obj)
        hexundump('''
        0000   00 00 01 02                                       ....
        ''')
    """
    def _decode(self, obj, context, path):
        if isinstance(obj, bytestringtype):
            return HexDumpDisplayedBytes(obj)
        if isinstance(obj, dict):
            return HexDumpDisplayedDict(obj)
        return obj

    def _encode(self, obj, context, path):
        return obj

    def _emitparse(self, code):
        return self.subcon._compileparse(code)

    def _emitseq(self, ksy, bitwise):
        return self.subcon._compileseq(ksy, bitwise)

    def _emitprimitivetype(self, ksy, bitwise):
        return self.subcon._compileprimitivetype(ksy, bitwise)

    def _emitfulltype(self, ksy, bitwise):
        return self.subcon._compilefulltype(ksy, bitwise)


#===============================================================================
# conditional
#===============================================================================
class Union(Construct):
    r"""
    Treats the same data as multiple constructs (similar to C union) so you can look at the data in multiple views. Fields are usually named (so parsed values are inserted into dictionary under same name).

    Parses subcons in sequence, and reverts the stream back to original position after each subcon. Afterwards, advances the stream by selected subcon. Builds from first subcon that has a matching key in given dict. Size is undefined (because parsefrom is not used for building).

    This class does context nesting, meaning its members are given access to a new dictionary where the "_" entry points to the outer context. When parsing, each member gets parsed and subcon parse return value is inserted into context under matching key only if the member was named. When building, the matching entry gets inserted into context before subcon gets build, and if subcon build returns a new value (not None) that gets replaced in the context.

    This class exposes subcons as attributes. You can refer to subcons that were inlined (and therefore do not exist as variable in the namespace) by accessing the struct attributes, under same name. Also note that compiler does not support this feature. See examples.

    This class exposes subcons in the context. You can refer to subcons that were inlined (and therefore do not exist as variable in the namespace) within other inlined fields using the context. Note that you need to use a lambda (`this` expression is not supported). Also note that compiler does not support this feature. See examples.

    .. warning:: If you skip `parsefrom` parameter then stream will be left back at starting offset, not seeked to any common denominator.

    :param parsefrom: how to leave stream after parsing, can be integer index or string name selecting a subcon, or None (leaves stream at initial offset, the default), or context lambda
    :param \*subcons: Construct instances, list of members, some can be anonymous
    :param \*\*subconskw: Construct instances, list of members (requires Python 3.6)

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises StreamError: stream is not seekable and tellable
    :raises UnionError: selector does not match any subcon, or dict given to build does not contain any keys matching any subcon
    :raises IndexError: selector does not match any subcon
    :raises KeyError: selector does not match any subcon

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Union(0, 
        ...     "raw" / Bytes(8),
        ...     "ints" / Int32ub[2],
        ...     "shorts" / Int16ub[4],
        ...     "chars" / Byte[8],
        ... )
        >>> d.parse(b"12345678")
        Container(raw=b'12345678', ints=[825373492, 892745528], shorts=[12594, 13108, 13622, 14136], chars=[49, 50, 51, 52, 53, 54, 55, 56])
        >>> d.build(dict(chars=range(8)))
        b'\x00\x01\x02\x03\x04\x05\x06\x07'

        >>> d = Union(None,
        ...     "animal" / Enum(Byte, giraffe=1),
        ... )
        >>> d.animal.giraffe
        'giraffe'
        >>> d = Union(None,
        ...     "chars" / Byte[4],
        ...     "data" / Bytes(lambda this: this._subcons.chars.sizeof()),
        ... )
        >>> d.parse(b"\x01\x02\x03\x04")
        Container(chars=[1, 2, 3, 4], data=b'\x01\x02\x03\x04')

        Alternative syntax, but requires Python 3.6 or any PyPy:
        >>> Union(0, raw=Bytes(8), ints=Int32ub[2], shorts=Int16ub[4], chars=Byte[8])
    """

    def __init__(self, parsefrom, *subcons, **subconskw):
        if isinstance(parsefrom, Construct):
            raise UnionError("parsefrom should be either: None int str context-function")
        super().__init__()
        self.parsefrom = parsefrom
        self.subcons = list(subcons) + list(k/v for k,v in subconskw.items())
        self._subcons = Container((sc.name,sc) for sc in self.subcons if sc.name)

    def __getattr__(self, name):
        if name in self._subcons:
            return self._subcons[name]
        raise AttributeError

    def _parse(self, stream, context, path):
        obj = Container()
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        fallback = stream_tell(stream, path)
        forwards = {}
        for i,sc in enumerate(self.subcons):
            subobj = sc._parsereport(stream, context, path)
            if sc.name:
                obj[sc.name] = subobj
                context[sc.name] = subobj
            forwards[i] = stream_tell(stream, path)
            if sc.name:
                forwards[sc.name] = stream_tell(stream, path)
            stream_seek(stream, fallback, 0, path)
        parsefrom = evaluate(self.parsefrom, context)
        if parsefrom is not None:
            stream_seek(stream, forwards[parsefrom], 0, path) # raises KeyError
        return obj

    def _build(self, obj, stream, context, path):
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        context.update(obj)
        for sc in self.subcons:
            if sc.flagbuildnone:
                subobj = obj.get(sc.name, None)
            elif sc.name in obj:
                subobj = obj[sc.name]
            else:
                continue

            if sc.name:
                context[sc.name] = subobj

            buildret = sc._build(subobj, stream, context, path)
            if sc.name:
                context[sc.name] = buildret
            return Container({sc.name:buildret})
        else:
            raise UnionError("cannot build, none of subcons were found in the dictionary %r" % (obj, ), path=path)

    def _sizeof(self, context, path):
        raise SizeofError("Union builds depending on actual object dict, size is unknown", path=path)

    def _emitparse(self, code):
        if callable(self.parsefrom):
            raise NotImplementedError("Union does not compile non-constant parsefrom")
        fname = "parse_union_%s" % code.allocateId()
        block = """
            def %s(io, this):
                this = Container(_ = this, _params = this['_params'], _root = None, _parsing = True, _building = False, _sizing = False, _subcons = None, _io = io, _index = this.get('_index', None))
                this['_root'] = this['_'].get('_root', this)
                fallback = io.tell()
        """ % (fname, )
        if isinstance(self.parsefrom, type(None)):
            index = -1
            skipfallback = False
            skipforward = True
        if isinstance(self.parsefrom, int):
            index = self.parsefrom
            self.subcons[index] # raises IndexError
            skipfallback = True
            skipforward = self.subcons[index].sizeof() == self.subcons[-1].sizeof()
        if isinstance(self.parsefrom, str):
            index = {sc.name:i for i,sc in enumerate(self.subcons) if sc.name}[self.parsefrom] # raises KeyError
            skipfallback = True
            skipforward = self.subcons[index].sizeof() == self.subcons[-1].sizeof()

        for i,sc in enumerate(self.subcons):
            block += """
                %s%s
            """ % ("this[%r] = " % sc.name if sc.name else "", sc._compileparse(code))
            if i == index and not skipforward:
                block += """
                forward = io.tell()
                """
            if i < len(self.subcons)-1:
                block += """
                io.seek(fallback)
                """
        if not skipfallback:
            block += """
                io.seek(fallback)
            """
        if not skipforward:
            block += """
                io.seek(forward)
            """
        block += """
                del this['_']
                del this['_index']
                return this
        """
        code.append(block)
        return "%s(io, this)" % (fname,)

    def _emitbuild(self, code):
        fname = f"build_union_{code.allocateId()}"
        block = f"""
            def {fname}(obj, io, this):
                this = Container(_ = this, _params = this['_params'], _root = None, _parsing = False, _building = True, _sizing = False, _subcons = None, _io = io, _index = this.get('_index', None))
                this['_root'] = this['_'].get('_root', this)
                this.update(obj)
                objdict = obj
        """
        for sc in self.subcons:
            block += f"""
                if {'True' if sc.flagbuildnone else f'{repr(sc.name)} in objdict'}:
                    {f'obj = objdict.get({repr(sc.name)}, None)' if sc.flagbuildnone else f'obj = objdict[{repr(sc.name)}]'}
                    {f'this[{repr(sc.name)}] = obj' if sc.name else ''}
                    {f'buildret = this[{repr(sc.name)}] = ' if sc.name else ''}{sc._compilebuild(code)}
                    {f'return Container({{ {repr(sc.name)}:buildret }})'}
            """
        block += f"""
                raise UnionError('cannot build, none of subcons were found in the dictionary')
        """
        code.append(block)
        return f"{fname}(obj, io, this)"


class Select(Construct):
    r"""
    Selects the first matching subconstruct.

    Parses and builds by literally trying each subcon in sequence until one of them parses or builds without exception. Stream gets reverted back to original position after each failed attempt, but not if parsing succeeds. Size is not defined.

    :param \*subcons: Construct instances, list of members, some can be anonymous
    :param \*\*subconskw: Construct instances, list of members (requires Python 3.6)

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises StreamError: stream is not seekable and tellable
    :raises SelectError: neither subcon succeded when parsing or building

    Example::

        >>> d = Select(Int32ub, CString("utf8"))
        >>> d.build(1)
        b'\x00\x00\x00\x01'
        >>> d.build(u"Афон")
        b'\xd0\x90\xd1\x84\xd0\xbe\xd0\xbd\x00'

        Alternative syntax, but requires Python 3.6 or any PyPy:
        >>> Select(num=Int32ub, text=CString("utf8"))
    """

    def __init__(self, *subcons, **subconskw):
        super().__init__()
        self.subcons = list(subcons) + list(k/v for k,v in subconskw.items())
        self.flagbuildnone = any(sc.flagbuildnone for sc in self.subcons)

    def _parse(self, stream, context, path):
        for sc in self.subcons:
            fallback = stream_tell(stream, path)
            try:
                obj = sc._parsereport(stream, context, path)
            except ExplicitError:
                raise
            except Exception:
                stream_seek(stream, fallback, 0, path)
            else:
                return obj
        raise SelectError("no subconstruct matched", path=path)

    def _build(self, obj, stream, context, path):
        for sc in self.subcons:
            try:
                data = sc.build(obj, **context)
            except ExplicitError:
                raise
            except Exception:
                pass
            else:
                stream_write(stream, data, len(data), path)
                return obj
        raise SelectError("no subconstruct matched: %s" % (obj,), path=path)


def Optional(subcon):
    r"""
    Makes an optional field.

    Parsing attempts to parse subcon. If sub-parsing fails, returns None and reports success. Building attempts to build subcon. If sub-building fails, writes nothing and reports success. Size is undefined, because whether bytes would be consumed or produced depends on actual data and actual context.

    :param subcon: Construct instance

    Example::

        Optional  <-->  Select(subcon, Pass)

        >>> d = Optional(Int64ul)
        >>> d.parse(b"12345678")
        4050765991979987505
        >>> d.parse(b"")
        None
        >>> d.build(1)
        b'\x01\x00\x00\x00\x00\x00\x00\x00'
        >>> d.build(None)
        b''
    """
    return Select(subcon, Pass)


def If(condfunc, subcon):
    r"""
    If-then conditional construct.

    Parsing evaluates condition, if True then subcon is parsed, otherwise just returns None. Building also evaluates condition, if True then subcon gets build from, otherwise does nothing. Size is either same as subcon or 0, depending how condfunc evaluates.

    :param condfunc: bool or context lambda (or a truthy value)
    :param subcon: Construct instance, used if condition indicates True

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        If <--> IfThenElse(condfunc, subcon, Pass)

        >>> d = If(this.x > 0, Byte)
        >>> d.build(255, x=1)
        b'\xff'
        >>> d.build(255, x=0)
        b''
    """
    macro = IfThenElse(condfunc, subcon, Pass)

    def _emitfulltype(ksy, bitwise):
        return dict(type=subcon._compileprimitivetype(ksy, bitwise), if_=repr(condfunc).replace("this.",""))
    macro._emitfulltype = _emitfulltype

    return macro


class IfThenElse(Construct):
    r"""
    If-then-else conditional construct, similar to ternary operator.

    Parsing and building evaluates condition, and defers to either subcon depending on the value. Size is computed the same way.

    :param condfunc: bool or context lambda (or a truthy value)
    :param thensubcon: Construct instance, used if condition indicates True
    :param elsesubcon: Construct instance, used if condition indicates False

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = IfThenElse(this.x > 0, VarInt, Byte)
        >>> d.build(255, dict(x=1))
        b'\xff\x01'
        >>> d.build(255, dict(x=0))
        b'\xff'
    """

    def __init__(self, condfunc, thensubcon, elsesubcon):
        super().__init__()
        self.condfunc = condfunc
        self.thensubcon = thensubcon
        self.elsesubcon = elsesubcon
        self.flagbuildnone = thensubcon.flagbuildnone and elsesubcon.flagbuildnone

    def _parse(self, stream, context, path):
        condfunc = evaluate(self.condfunc, context)
        sc = self.thensubcon if condfunc else self.elsesubcon
        return sc._parsereport(stream, context, path)

    def _build(self, obj, stream, context, path):
        condfunc = evaluate(self.condfunc, context)
        sc = self.thensubcon if condfunc else self.elsesubcon
        return sc._build(obj, stream, context, path)

    def _sizeof(self, context, path):
        condfunc = evaluate(self.condfunc, context)
        sc = self.thensubcon if condfunc else self.elsesubcon
        return sc._sizeof(context, path)

    def _emitparse(self, code):
        return "((%s) if (%s) else (%s))" % (self.thensubcon._compileparse(code), self.condfunc, self.elsesubcon._compileparse(code), )

    def _emitbuild(self, code):
        return f"(({self.thensubcon._compilebuild(code)}) if ({repr(self.condfunc)}) else ({self.elsesubcon._compilebuild(code)}))"

    def _emitseq(self, ksy, bitwise):
        return [
        dict(id="thenvalue", type=self.thensubcon._compileprimitivetype(ksy, bitwise), if_=repr(self.condfunc).replace("this.","")),
        dict(id="elsesubcon", type=self.elsesubcon._compileprimitivetype(ksy, bitwise), if_=repr(~self.condfunc).replace("this.","")),
        ]


class Switch(Construct):
    r"""
    A conditional branch.

    Parsing and building evaluate keyfunc and select a subcon based on the value and dictionary entries. Dictionary (cases) maps values into subcons. If no case matches then `default` is used (that is Pass by default). Note that `default` is a Construct instance, not a dictionary key. Size is evaluated in same way as parsing and building, by evaluating keyfunc and selecting a field accordingly.

    :param keyfunc: context lambda or constant, that matches some key in cases
    :param cases: dict mapping keys to Construct instances
    :param default: optional, Construct instance, used when keyfunc is not found in cases, Pass is default value for this parameter, Error is a possible value for this parameter

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Switch(this.n, { 1:Int8ub, 2:Int16ub, 4:Int32ub })
        >>> d.build(5, n=1)
        b'\x05'
        >>> d.build(5, n=4)
        b'\x00\x00\x00\x05'

        >>> d = Switch(this.n, {}, default=Byte)
        >>> d.parse(b"\x01", n=255)
        1
        >>> d.build(1, n=255)
        b"\x01"
    """

    def __init__(self, keyfunc, cases, default=None):
        if default is None:
            default = Pass
        super().__init__()
        self.keyfunc = keyfunc
        self.cases = cases
        self.default = default
        allcases = list(cases.values()) + [default]
        self.flagbuildnone = all(sc.flagbuildnone for sc in allcases)

    def _parse(self, stream, context, path):
        keyfunc = evaluate(self.keyfunc, context)
        sc = self.cases.get(keyfunc, self.default)
        return sc._parsereport(stream, context, path)

    def _build(self, obj, stream, context, path):
        keyfunc = evaluate(self.keyfunc, context)
        sc = self.cases.get(keyfunc, self.default)
        return sc._build(obj, stream, context, path)

    def _sizeof(self, context, path):
        try:
            keyfunc = evaluate(self.keyfunc, context)
            sc = self.cases.get(keyfunc, self.default)
            return sc._sizeof(context, path)

        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)

    def _emitparse(self, code):
        fname = f"switch_cases_{code.allocateId()}"
        code.append(f"{fname} = {{}}")
        for key,sc in self.cases.items():
            code.append(f"{fname}[{repr(key)}] = lambda io,this: {sc._compileparse(code)}")
        defaultfname = f"switch_defaultcase_{code.allocateId()}"
        code.append(f"{defaultfname} = lambda io,this: {self.default._compileparse(code)}")
        return f"{fname}.get({repr(self.keyfunc)}, {defaultfname})(io, this)"

    def _emitbuild(self, code):
        fname = f"switch_cases_{code.allocateId()}"
        code.append(f"{fname} = {{}}")
        for key,sc in self.cases.items():
            code.append(f"{fname}[{repr(key)}] = lambda obj,io,this: {sc._compilebuild(code)}")
        defaultfname = f"switch_defaultcase_{code.allocateId()}"
        code.append(f"{defaultfname} = lambda obj,io,this: {self.default._compilebuild(code)}")
        return f"{fname}.get({repr(self.keyfunc)}, {defaultfname})(obj, io, this)"


class StopIf(Construct):
    r"""
    Checks for a condition, and stops certain classes (:class:`~construct.core.Struct` :class:`~construct.core.Sequence` :class:`~construct.core.GreedyRange`) from parsing or building further.

    Parsing and building check the condition, and raise StopFieldError if indicated. Size is undefined.

    :param condfunc: bool or context lambda (or truthy value)

    :raises StopFieldError: used internally

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> Struct('x'/Byte, StopIf(this.x == 0), 'y'/Byte)
        >>> Sequence('x'/Byte, StopIf(this.x == 0), 'y'/Byte)
        >>> GreedyRange(FocusedSeq(0, 'x'/Byte, StopIf(this.x == 0)))
    """

    def __init__(self, condfunc):
        super().__init__()
        self.condfunc = condfunc
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        condfunc = evaluate(self.condfunc, context)
        if condfunc:
            raise StopFieldError(path=path)

    def _build(self, obj, stream, context, path):
        condfunc = evaluate(self.condfunc, context)
        if condfunc:
            raise StopFieldError(path=path)

    def _sizeof(self, context, path):
        raise SizeofError("StopIf cannot determine size because it depends on actual context which then depends on actual data and outer constructs", path=path)

    def _emitparse(self, code):
        code.append(f"""
            def parse_stopif(condition):
                if condition:
                    raise StopFieldError
        """)
        return f"parse_stopif({repr(self.condfunc)})"

    def _emitbuild(self, code):
        code.append(f"""
            def build_stopif(condition):
                if condition:
                    raise StopFieldError
        """)
        return f"build_stopif({repr(self.condfunc)})"


#===============================================================================
# alignment and padding
#===============================================================================
def Padding(length, pattern=b"\x00"):
    r"""
    Appends null bytes.

    Parsing consumes specified amount of bytes and discards it. Building writes specified pattern byte multiplied into specified length. Size is same as specified.

    :param length: integer or context lambda, length of the padding
    :param pattern: b-character, padding pattern, default is \\x00

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises PaddingError: length was negative
    :raises PaddingError: pattern was not bytes (b-character)

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Padding(4) or Padded(4, Pass)
        >>> d.build(None)
        b'\x00\x00\x00\x00'
        >>> d.parse(b"****")
        None
        >>> d.sizeof()
        4
    """
    macro = Padded(length, Pass, pattern=pattern)
    def _emitprimitivetype(ksy, bitwise):
        if not bitwise:
            raise NotImplementedError
        return "b%s" % (length, )
    def _emitfulltype(ksy, bitwise):
        if bitwise:
            raise NotImplementedError
        return dict(size=length)
    macro._emitprimitivetype = _emitprimitivetype
    macro._emitfulltype = _emitfulltype
    return macro


class Padded(Subconstruct):
    r"""
    Appends additional null bytes to achieve a length.

    Parsing first parses the subcon, then uses stream.tell() to measure how many bytes were read and consumes additional bytes accordingly. Building first builds the subcon, then uses stream.tell() to measure how many bytes were written and produces additional bytes accordingly. Size is same as `length`, but negative amount results in error. Note that subcon can actually be variable size, it is the eventual amount of bytes that is read or written during parsing or building that determines actual padding.

    :param length: integer or context lambda, length of the padding
    :param subcon: Construct instance
    :param pattern: optional, b-character, padding pattern, default is \\x00

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises PaddingError: length is negative
    :raises PaddingError: subcon read or written more than the length (would cause negative pad)
    :raises PaddingError: pattern is not bytes of length 1

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Padded(4, Byte)
        >>> d.build(255)
        b'\xff\x00\x00\x00'
        >>> d.parse(_)
        255
        >>> d.sizeof()
        4

        >>> d = Padded(4, VarInt)
        >>> d.build(1)
        b'\x01\x00\x00\x00'
        >>> d.build(70000)
        b'\xf0\xa2\x04\x00'
    """

    def __init__(self, length, subcon, pattern=b"\x00"):
        if not isinstance(pattern, bytestringtype) or len(pattern) != 1:
            raise PaddingError("pattern expected to be bytes of length 1")
        super().__init__(subcon)
        self.length = length
        self.pattern = pattern

    def _parse(self, stream, context, path):
        length = evaluate(self.length, context)
        if length < 0:
            raise PaddingError("length cannot be negative", path=path)
        position1 = stream_tell(stream, path)
        obj = self.subcon._parsereport(stream, context, path)
        position2 = stream_tell(stream, path)
        pad = length - (position2 - position1)
        if pad < 0:
            raise PaddingError("subcon parsed %d bytes but was allowed only %d" % (position2-position1, length), path=path)
        stream_read(stream, pad, path)
        return obj

    def _build(self, obj, stream, context, path):
        length = evaluate(self.length, context)
        if length < 0:
            raise PaddingError("length cannot be negative", path=path)
        position1 = stream_tell(stream, path)
        buildret = self.subcon._build(obj, stream, context, path)
        position2 = stream_tell(stream, path)
        pad = length - (position2 - position1)
        if pad < 0:
            raise PaddingError("subcon build %d bytes but was allowed only %d" % (position2-position1, length), path=path)
        stream_write(stream, self.pattern * pad, pad, path)
        return buildret

    def _sizeof(self, context, path):
        try:
            length = evaluate(self.length, context)
            if length < 0:
                raise PaddingError("length cannot be negative", path=path)
            return length
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)

    def _emitparse(self, code):
        return f"({self.subcon._compileparse(code)}, io.read(({self.length})-({self.subcon.sizeof()}) ))[0]"

    def _emitbuild(self, code):
        return f"({self.subcon._compilebuild(code)}, io.write({repr(self.pattern)}*(({self.length})-({self.subcon.sizeof()})) ))[0]"

    def _emitfulltype(self, ksy, bitwise):
        return dict(size=self.length, type=self.subcon._compileprimitivetype(ksy, bitwise))


class Aligned(Subconstruct):
    r"""
    Appends additional null bytes to achieve a length that is shortest multiple of a modulus.

    Note that subcon can actually be variable size, it is the eventual amount of bytes that is read or written during parsing or building that determines actual padding.

    Parsing first parses subcon, then consumes an amount of bytes to sum up to specified length, and discards it. Building first builds subcon, then writes specified pattern byte to sum up to specified length. Size is subcon size plus modulo remainder, unless SizeofError was raised.

    :param modulus: integer or context lambda, modulus to final length
    :param subcon: Construct instance
    :param pattern: optional, b-character, padding pattern, default is \\x00

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises PaddingError: modulus was less than 2
    :raises PaddingError: pattern was not bytes (b-character)

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Aligned(4, Int16ub)
        >>> d.parse(b'\x00\x01\x00\x00')
        1
        >>> d.sizeof()
        4
    """

    def __init__(self, modulus, subcon, pattern=b"\x00"):
        if not isinstance(pattern, bytestringtype) or len(pattern) != 1:
            raise PaddingError("pattern expected to be bytes character")
        super().__init__(subcon)
        self.modulus = modulus
        self.pattern = pattern

    def _parse(self, stream, context, path):
        modulus = evaluate(self.modulus, context)
        if modulus < 2:
            raise PaddingError("expected modulo 2 or greater", path=path)
        position1 = stream_tell(stream, path)
        obj = self.subcon._parsereport(stream, context, path)
        position2 = stream_tell(stream, path)
        pad = -(position2 - position1) % modulus
        stream_read(stream, pad, path)
        return obj

    def _build(self, obj, stream, context, path):
        modulus = evaluate(self.modulus, context)
        if modulus < 2:
            raise PaddingError("expected modulo 2 or greater", path=path)
        position1 = stream_tell(stream, path)
        buildret = self.subcon._build(obj, stream, context, path)
        position2 = stream_tell(stream, path)
        pad = -(position2 - position1) % modulus
        stream_write(stream, self.pattern * pad, pad, path)
        return buildret

    def _sizeof(self, context, path):
        try:
            modulus = evaluate(self.modulus, context)
            if modulus < 2:
                raise PaddingError("expected modulo 2 or greater", path=path)
            subconlen = self.subcon._sizeof(context, path)
            return subconlen + (-subconlen % modulus)
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)

    def _emitparse(self, code):
        return f"({self.subcon._compileparse(code)}, io.read(-({self.subcon.sizeof()}) % ({self.modulus}) ))[0]"

    def _emitbuild(self, code):
        return f"({self.subcon._compilebuild(code)}, io.write({repr(self.pattern)}*(-({self.subcon.sizeof()}) % ({self.modulus}))) )[0]"


def AlignedStruct(modulus, *subcons, **subconskw):
    r"""
    Makes a structure where each field is aligned to the same modulus (it is a struct of aligned fields, NOT an aligned struct).

    See :class:`~construct.core.Aligned` and :class:`~construct.core.Struct` for semantics and raisable exceptions.

    :param modulus: integer or context lambda, passed to each member
    :param \*subcons: Construct instances, list of members, some can be anonymous
    :param \*\*subconskw: Construct instances, list of members (requires Python 3.6)

    Example::

        >>> d = AlignedStruct(4, "a"/Int8ub, "b"/Int16ub)
        >>> d.build(dict(a=0xFF,b=0xFFFF))
        b'\xff\x00\x00\x00\xff\xff\x00\x00'
    """
    subcons = list(subcons) + list(k/v for k,v in subconskw.items())
    return Struct(*[sc.name / Aligned(modulus, sc) for sc in subcons])


def BitStruct(*subcons, **subconskw):
    r"""
    Makes a structure inside a Bitwise.

    See :class:`~construct.core.Bitwise` and :class:`~construct.core.Struct` for semantics and raisable exceptions.

    :param \*subcons: Construct instances, list of members, some can be anonymous
    :param \*\*subconskw: Construct instances, list of members (requires Python 3.6)

    Example::

        BitStruct  <-->  Bitwise(Struct(...))

        >>> d = BitStruct(
        ...     "a" / Flag,
        ...     "b" / Nibble,
        ...     "c" / BitsInteger(10),
        ...     "d" / Padding(1),
        ... )
        >>> d.parse(b"\xbe\xef")
        Container(a=True, b=7, c=887, d=None)
        >>> d.sizeof()
        2
    """
    return Bitwise(Struct(*subcons, **subconskw))


#===============================================================================
# stream manipulation
#===============================================================================
class Pointer(Subconstruct):
    r"""
    Jumps in the stream forth and back for one field.

    Parsing and building seeks the stream to new location, processes subcon, and seeks back to original location. Size is defined as 0 but that does not mean no bytes are written into the stream.

    Offset can be positive, indicating a position from stream beginning forward, or negative, indicating a position from EOF backwards.

    :param offset: integer or context lambda, positive or negative
    :param subcon: Construct instance
    :param stream: None to use original stream (default), or context lambda to provide a different stream

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises StreamError: stream is not seekable and tellable

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Pointer(8, Bytes(1))
        >>> d.parse(b"abcdefghijkl")
        b'i'
        >>> d.build(b"Z")
        b'\x00\x00\x00\x00\x00\x00\x00\x00Z'
    """

    def __init__(self, offset, subcon, stream=None):
        super().__init__(subcon)
        self.offset = offset
        self.stream = stream

    def _parse(self, stream, context, path):
        offset = evaluate(self.offset, context)
        stream = evaluate(self.stream, context) or stream
        fallback = stream_tell(stream, path)
        stream_seek(stream, offset, 2 if offset < 0 else 0, path)
        obj = self.subcon._parsereport(stream, context, path)
        stream_seek(stream, fallback, 0, path)
        return obj

    def _build(self, obj, stream, context, path):
        offset = evaluate(self.offset, context)
        stream = evaluate(self.stream, context) or stream
        fallback = stream_tell(stream, path)
        stream_seek(stream, offset, 2 if offset < 0 else 0, path)
        buildret = self.subcon._build(obj, stream, context, path)
        stream_seek(stream, fallback, 0, path)
        return buildret

    def _sizeof(self, context, path):
        return 0

    def _emitparse(self, code):
        code.append(f"""
            def parse_pointer(io, offset, func):
                fallback = io.tell()
                io.seek(offset, 2 if offset < 0 else 0)
                obj = func()
                io.seek(fallback)
                return obj
        """)
        return f"parse_pointer(io, {self.offset}, lambda: {self.subcon._compileparse(code)})"

    def _emitbuild(self, code):
        code.append(f"""
            def build_pointer(obj, io, offset, func):
                fallback = io.tell()
                io.seek(offset, 2 if offset < 0 else 0)
                ret = func()
                io.seek(fallback)
                return ret
        """)
        return f"build_pointer(obj, io, {self.offset}, lambda: {self.subcon._compilebuild(code)})"

    def _emitprimitivetype(self, ksy, bitwise):
        offset = self.offset.__getfield__() if callable(self.offset) else self.offset
        name = "instance_%s" % ksy.allocateId()
        ksy.instances[name] = dict(pos=offset, **self.subcon._compilefulltype(ksy, bitwise))
        return name


class Peek(Subconstruct):
    r"""
    Peeks at the stream.

    Parsing sub-parses (and returns None if failed), then reverts stream to original position. Building does nothing (its NOT deferred). Size is defined as 0 because there is no building.

    This class is used in :class:`~construct.core.Union` class to parse each member.

    :param subcon: Construct instance

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises StreamError: stream is not seekable and tellable

    Example::

        >>> d = Sequence(Peek(Int8ub), Peek(Int16ub))
        >>> d.parse(b"\x01\x02")
        [1, 258]
        >>> d.sizeof()
        0
    """

    def __init__(self, subcon):
        super().__init__(subcon)
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        fallback = stream_tell(stream, path)
        try:
            return self.subcon._parsereport(stream, context, path)
        except ExplicitError:
            raise
        except ConstructError:
            pass
        finally:
            stream_seek(stream, fallback, 0, path)

    def _build(self, obj, stream, context, path):
        return obj

    def _sizeof(self, context, path):
        return 0

    def _emitparse(self, code):
        code.append("""
            def parse_peek(io, func):
                fallback = io.tell()
                try:
                    return func()
                except ExplicitError:
                    raise
                except ConstructError:
                    pass
                finally:
                    io.seek(fallback)
        """)
        return "parse_peek(io, lambda: %s)" % (self.subcon._compileparse(code),)

    def _emitbuild(self, code):
        return "obj"


class OffsettedEnd(Subconstruct):
    r"""
    Parses all bytes in the stream till `EOF plus a negative endoffset` is reached.

    This is useful when GreedyBytes (or any other greedy construct) is followed by a fixed-size footer.

    Parsing determines the length of the stream and reads all bytes till EOF plus `endoffset` is reached, then defers to subcon using new BytesIO with said bytes. Building defers to subcon as-is. Size is undefined.

    :param endoffset: integer or context lambda, only negative offsets or zero are allowed
    :param subcon: Construct instance

    :raises StreamError: could not read enough bytes
    :raises StreamError: reads behind the stream (if endoffset is positive)

    Example::

        >>> d = Struct(
        ...     "header" / Bytes(2),
        ...     "data" / OffsettedEnd(-2, GreedyBytes),
        ...     "footer" / Bytes(2),
        ... )
        >>> d.parse(b"\x01\x02\x03\x04\x05\x06\x07")
        Container(header=b'\x01\x02', data=b'\x03\x04\x05', footer=b'\x06\x07')
    """

    def __init__(self, endoffset, subcon):
        super().__init__(subcon)
        self.endoffset = endoffset

    def _parse(self, stream, context, path):
        endoffset = evaluate(self.endoffset, context)
        curpos = stream_tell(stream, path)
        stream_seek(stream, 0, 2, path)
        endpos = stream_tell(stream, path)
        stream_seek(stream, curpos, 0, path)
        length = endpos + endoffset - curpos
        substream = BytesIOWithOffsets.from_reading(stream, length, path)
        return self.subcon._parsereport(substream, context, path)

    def _build(self, obj, stream, context, path):
        return self.subcon._build(obj, stream, context, path)

    def _sizeof(self, context, path):
        raise SizeofError(path=path)


class Seek(Construct):
    r"""
    Seeks the stream.

    Parsing and building seek the stream to given location (and whence), and return stream.seek() return value. Size is not defined.

    .. seealso:: Analog :class:`~construct.core.Pointer` wrapper that has same side effect but also processes a subcon, and also seeks back.

    :param at: integer or context lambda, where to jump to
    :param whence: optional, integer or context lambda, is the offset from beginning (0) or from current position (1) or from EOF (2), default is 0

    :raises StreamError: stream is not seekable

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = (Seek(5) >> Byte)
        >>> d.parse(b"01234x")
        [5, 120]

        >>> d = (Bytes(10) >> Seek(5) >> Byte)
        >>> d.build([b"0123456789", None, 255])
        b'01234\xff6789'
    """

    def __init__(self, at, whence=0):
        super().__init__()
        self.at = at
        self.whence = whence
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        at = evaluate(self.at, context)
        whence = evaluate(self.whence, context)
        return stream_seek(stream, at, whence, path)

    def _build(self, obj, stream, context, path):
        at = evaluate(self.at, context)
        whence = evaluate(self.whence, context)
        return stream_seek(stream, at, whence, path)

    def _sizeof(self, context, path):
        raise SizeofError("Seek only moves the stream, size is not meaningful", path=path)

    def _emitparse(self, code):
        return f"io.seek({self.at}, {self.whence})"

    def _emitbuild(self, code):
        return f"io.seek({self.at}, {self.whence})"


@singleton
class Tell(Construct):
    r"""
    Tells the stream.

    Parsing and building return current stream offset using using stream.tell(). Size is defined as 0 because parsing and building does not consume or add into the stream.

    Tell is useful for adjusting relative offsets to absolute positions, or to measure sizes of Constructs. To get an absolute pointer, use a Tell plus a relative offset. To get a size, place two Tells and measure their difference using a Compute field. However, its recommended to use :class:`~construct.core.RawCopy` instead of manually extracting two positions and computing difference.

    :raises StreamError: stream is not tellable

    Example::

        >>> d = Struct("num"/VarInt, "offset"/Tell)
        >>> d.parse(b"X")
        Container(num=88, offset=1)
        >>> d.build(dict(num=88))
        b'X'
    """

    def __init__(self):
        super().__init__()
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        return stream_tell(stream, path)

    def _build(self, obj, stream, context, path):
        return stream_tell(stream, path)

    def _sizeof(self, context, path):
        return 0

    def _emitparse(self, code):
        return "io.tell()"

    def _emitbuild(self, code):
        return "io.tell()"


@singleton
class Pass(Construct):
    r"""
    No-op construct, useful as default cases for Switch and Enum.

    Parsing returns None. Building does nothing. Size is 0 by definition.

    Example::

        >>> Pass.parse(b"")
        None
        >>> Pass.build(None)
        b''
        >>> Pass.sizeof()
        0
    """

    def __init__(self):
        super().__init__()
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        return None

    def _build(self, obj, stream, context, path):
        return obj

    def _sizeof(self, context, path):
        return 0

    def _emitparse(self, code):
        return "None"

    def _emitbuild(self, code):
        return "None"

    def _emitfulltype(self, ksy, bitwise):
        return dict(size=0)


@singleton
class Terminated(Construct):
    r"""
    Asserts end of stream (EOF). You can use it to ensure no more unparsed data follows in the stream.

    Parsing checks if stream reached EOF, and raises TerminatedError if not. Building does nothing. Size is defined as 0 because parsing and building does not consume or add into the stream, as far as other constructs see it.

    :raises TerminatedError: stream not at EOF when parsing

    Example::

        >>> Terminated.parse(b"")
        None
        >>> Terminated.parse(b"remaining")
        construct.core.TerminatedError: expected end of stream
    """

    def __init__(self):
        super().__init__()
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        if stream.read(1):
            raise TerminatedError("expected end of stream", path=path)

    def _build(self, obj, stream, context, path):
        return obj

    def _sizeof(self, context, path):
        raise SizeofError(path=path)


#===============================================================================
# tunneling and byte/bit swapping
#===============================================================================
class RawCopy(Subconstruct):
    r"""
    Used to obtain byte representation of a field (aside of object value).

    Returns a dict containing both parsed subcon value, the raw bytes that were consumed by subcon, starting and ending offset in the stream, and amount in bytes. Builds either from raw bytes representation or a value used by subcon. Size is same as subcon.

    Object is a dictionary with either "data" or "value" keys, or both.

    When building, if both the "value" and "data" keys are present, then the "data" key is used and the "value" key is ignored. This is undesirable in the case that you parse some data for the purpose of modifying it and writing it back; in this case, delete the "data" key when modifying the "value" key to correctly rebuild the former.

    :param subcon: Construct instance

    :raises StreamError: stream is not seekable and tellable
    :raises RawCopyError: building and neither data or value was given
    :raises StringError: building from non-bytes value, perhaps unicode

    Example::

        >>> d = RawCopy(Byte)
        >>> d.parse(b"\xff")
        Container(data=b'\xff', value=255, offset1=0, offset2=1, length=1)
        >>> d.build(dict(data=b"\xff"))
        '\xff'
        >>> d.build(dict(value=255))
        '\xff'
    """

    def _parse(self, stream, context, path):
        offset1 = stream_tell(stream, path)
        obj = self.subcon._parsereport(stream, context, path)
        offset2 = stream_tell(stream, path)
        stream_seek(stream, offset1, 0, path)
        data = stream_read(stream, offset2-offset1, path)
        return Container(data=data, value=obj, offset1=offset1, offset2=offset2, length=(offset2-offset1))

    def _build(self, obj, stream, context, path):
        if obj is None and self.subcon.flagbuildnone:
            obj = dict(value=None)
        if 'data' in obj:
            data = obj['data']
            offset1 = stream_tell(stream, path)
            stream_write(stream, data, len(data), path)
            offset2 = stream_tell(stream, path)
            return Container(obj, data=data, offset1=offset1, offset2=offset2, length=(offset2-offset1))
        if 'value' in obj:
            value = obj['value']
            offset1 = stream_tell(stream, path)
            buildret = self.subcon._build(value, stream, context, path)
            value = value if buildret is None else buildret
            offset2 = stream_tell(stream, path)
            stream_seek(stream, offset1, 0, path)
            data = stream_read(stream, offset2-offset1, path)
            return Container(obj, data=data, value=value, offset1=offset1, offset2=offset2, length=(offset2-offset1))
        raise RawCopyError('RawCopy cannot build, both data and value keys are missing', path=path)


def ByteSwapped(subcon):
    r"""
    Swaps the byte order within boundaries of given subcon. Requires a fixed sized subcon.

    :param subcon: Construct instance, subcon on top of byte swapped bytes

    :raises SizeofError: ctor or compiler could not compute subcon size

    See :class:`~construct.core.Transformed` and :class:`~construct.core.Restreamed` for raisable exceptions.

    Example::

        Int24ul <--> ByteSwapped(Int24ub) <--> BytesInteger(3, swapped=True) <--> ByteSwapped(BytesInteger(3))
    """

    size = subcon.sizeof()
    return Transformed(subcon, swapbytes, size, swapbytes, size)


def BitsSwapped(subcon):
    r"""
    Swaps the bit order within each byte within boundaries of given subcon. Does NOT require a fixed sized subcon.

    :param subcon: Construct instance, subcon on top of bit swapped bytes

    :raises SizeofError: compiler could not compute subcon size

    See :class:`~construct.core.Transformed` and :class:`~construct.core.Restreamed` for raisable exceptions.

    Example::

        >>> d = Bitwise(Bytes(8))
        >>> d.parse(b"\x01")
        '\x00\x00\x00\x00\x00\x00\x00\x01'
        >>>> BitsSwapped(d).parse(b"\x01")
        '\x01\x00\x00\x00\x00\x00\x00\x00'
    """

    try:
        size = subcon.sizeof()
        return Transformed(subcon, swapbitsinbytes, size, swapbitsinbytes, size)
    except SizeofError:
        return Restreamed(subcon, swapbitsinbytes, 1, swapbitsinbytes, 1, lambda n: n)


class Prefixed(Subconstruct):
    r"""
    Prefixes a field with byte count.

    Parses the length field. Then reads that amount of bytes, and parses subcon using only those bytes. Constructs that consume entire remaining stream are constrained to consuming only the specified amount of bytes (a substream). When building, data gets prefixed by its length. Optionally, length field can include its own size. Size is the sum of both fields sizes, unless either raises SizeofError.

    Analog to :class:`~construct.core.PrefixedArray` which prefixes with an element count, instead of byte count. Semantics is similar but implementation is different.

    :class:`~construct.core.VarInt` is recommended for new protocols, as it is more compact and never overflows.

    :param lengthfield: Construct instance, field used for storing the length
    :param subcon: Construct instance, subcon used for storing the value
    :param includelength: optional, bool, whether length field should include its own size, default is False

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes

    Example::

        >>> d = Prefixed(VarInt, GreedyRange(Int32ul))
        >>> d.parse(b"\x08abcdefgh")
        [1684234849, 1751606885]

        >>> d = PrefixedArray(VarInt, Int32ul)
        >>> d.parse(b"\x02abcdefgh")
        [1684234849, 1751606885]
    """

    def __init__(self, lengthfield, subcon, includelength=False):
        super().__init__(subcon)
        self.lengthfield = lengthfield
        self.includelength = includelength

    def _parse(self, stream, context, path):
        length = self.lengthfield._parsereport(stream, context, path)
        if self.includelength:
            length -= self.lengthfield._sizeof(context, path)
        substream = BytesIOWithOffsets.from_reading(stream, length, path)
        return self.subcon._parsereport(substream, context, path)

    def _build(self, obj, stream, context, path):
        stream2 = io.BytesIO()
        buildret = self.subcon._build(obj, stream2, context, path)
        data = stream2.getvalue()
        length = len(data)
        if self.includelength:
            length += self.lengthfield._sizeof(context, path)
        self.lengthfield._build(length, stream, context, path)
        stream_write(stream, data, len(data), path)
        return buildret

    def _sizeof(self, context, path):
        return self.lengthfield._sizeof(context, path) + self.subcon._sizeof(context, path)

    def _actualsize(self, stream, context, path):
        position1 = stream_tell(stream, path)
        length = self.lengthfield._parse(stream, context, path)
        if self.includelength:
            length -= self.lengthfield._sizeof(context, path)
        position2 = stream_tell(stream, path)
        return (position2-position1) + length

    def _emitparse(self, code):
        sub = self.lengthfield.sizeof() if self.includelength else 0
        return f"restream(io.read(({self.lengthfield._compileparse(code)})-({sub})), lambda io: ({self.subcon._compileparse(code)}))"

    def _emitseq(self, ksy, bitwise):
        return [
            dict(id="lengthfield", type=self.lengthfield._compileprimitivetype(ksy, bitwise)), 
            dict(id="data", size="lengthfield", type=self.subcon._compileprimitivetype(ksy, bitwise)),
        ]


def PrefixedArray(countfield, subcon):
    r"""
    Prefixes an array with item count (as opposed to prefixed by byte count, see :class:`~construct.core.Prefixed`).

    :class:`~construct.core.VarInt` is recommended for new protocols, as it is more compact and never overflows.

    :param countfield: Construct instance, field used for storing the element count
    :param subcon: Construct instance, subcon used for storing each element

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises RangeError: consumed or produced too little elements

    Example::

        >>> d = Prefixed(VarInt, GreedyRange(Int32ul))
        >>> d.parse(b"\x08abcdefgh")
        [1684234849, 1751606885]

        >>> d = PrefixedArray(VarInt, Int32ul)
        >>> d.parse(b"\x02abcdefgh")
        [1684234849, 1751606885]
    """
    macro = FocusedSeq("items",
        "count" / Rebuild(countfield, len_(this.items)),
        "items" / subcon[this.count],
    )

    def _emitparse(code):
        return "ListContainer((%s) for i in range(%s))" % (subcon._compileparse(code), countfield._compileparse(code), )
    macro._emitparse = _emitparse

    def _emitbuild(code):
        return f"(reuse(len(obj), lambda obj: {countfield._compilebuild(code)}), list({subcon._compilebuild(code)} for obj in obj), obj)[2]"
    macro._emitbuild = _emitbuild

    def _actualsize(self, stream, context, path):
        position1 = stream_tell(stream, path)
        count = countfield._parse(stream, context, path)
        position2 = stream_tell(stream, path)
        return (position2-position1) + count * subcon._sizeof(context, path)
    macro._actualsize = _actualsize

    def _emitseq(ksy, bitwise):
        return [
            dict(id="countfield", type=countfield._compileprimitivetype(ksy, bitwise)), 
            dict(id="data", type=subcon._compileprimitivetype(ksy, bitwise), repeat="expr", repeat_expr="countfield"),
        ]
    macro._emitseq = _emitseq

    return macro


class FixedSized(Subconstruct):
    r"""
    Restricts parsing to specified amount of bytes.

    Parsing reads `length` bytes, then defers to subcon using new BytesIO with said bytes. Building builds the subcon using new BytesIO, then writes said data and additional null bytes accordingly. Size is same as `length`, although negative amount raises an error.

    :param length: integer or context lambda, total amount of bytes (both data and padding)
    :param subcon: Construct instance

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises PaddingError: length is negative
    :raises PaddingError: subcon written more bytes than entire length (negative padding)

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = FixedSized(10, Byte)
        >>> d.parse(b'\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00')
        255
        >>> d.build(255)
        b'\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        >>> d.sizeof()
        10
    """

    def __init__(self, length, subcon):
        super().__init__(subcon)
        self.length = length

    def _parse(self, stream, context, path):
        length = evaluate(self.length, context)
        if length < 0:
            raise PaddingError("length cannot be negative", path=path)
        substream = BytesIOWithOffsets.from_reading(stream, length, path)
        return self.subcon._parsereport(substream, context, path)

    def _build(self, obj, stream, context, path):
        length = evaluate(self.length, context)
        if length < 0:
            raise PaddingError("length cannot be negative", path=path)
        stream2 = io.BytesIO()
        buildret = self.subcon._build(obj, stream2, context, path)
        data = stream2.getvalue()
        pad = length - len(data)
        if pad < 0:
            raise PaddingError("subcon build %d bytes but was allowed only %d" % (len(data), length), path=path)
        stream_write(stream, data, len(data), path)
        stream_write(stream, bytes(pad), pad, path)
        return buildret

    def _sizeof(self, context, path):
        length = evaluate(self.length, context)
        if length < 0:
            raise PaddingError("length cannot be negative", path=path)
        return length

    def _emitparse(self, code):
        return f"restream(io.read({self.length}), lambda io: ({self.subcon._compileparse(code)}))"

    def _emitfulltype(self, ksy, bitwise):
        return dict(size=repr(self.length).replace("this.",""), **self.subcon._compilefulltype(ksy, bitwise))


class NullTerminated(Subconstruct):
    r"""
    Restricts parsing to bytes preceding a null byte.

    Parsing reads one byte at a time and accumulates it with previous bytes. When term was found, (by default) consumes but discards the term. When EOF was found, (by default) raises same StreamError exception. Then subcon is parsed using new BytesIO made with said data. Building builds the subcon and then writes the term. Size is undefined.

    The term can be multiple bytes, to support string classes with UTF16/32 encodings for example. Be warned however: as reported in Issue 1046, the data read must be a multiple of the term length and the term must start at a unit boundary, otherwise strange things happen when parsing.

    :param subcon: Construct instance
    :param term: optional, bytes, terminator byte-string, default is \x00 single null byte
    :param include: optional, bool, if to include terminator in resulting data, default is False
    :param consume: optional, bool, if to consume terminator or leave it in the stream, default is True
    :param require: optional, bool, if EOF results in failure or not, default is True

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises StreamError: encountered EOF but require is not disabled
    :raises PaddingError: terminator is less than 1 bytes in length

    Example::

        >>> d = NullTerminated(Byte)
        >>> d.parse(b'\xff\x00')
        255
        >>> d.build(255)
        b'\xff\x00'
    """

    def __init__(self, subcon, term=b"\x00", include=False, consume=True, require=True):
        super().__init__(subcon)
        self.term = term
        self.include = include
        self.consume = consume
        self.require = require

    def _parse(self, stream, context, path):
        term = self.term
        unit = len(term)
        if unit < 1:
            raise PaddingError("NullTerminated term must be at least 1 byte", path=path)
        data = b''
        offset = stream_tell(stream, path)
        while True:
            try:
                b = stream_read(stream, unit, path)
            except StreamError:
                if self.require:
                    raise
                else:
                    break
            if b == term:
                if self.include:
                    data += b
                if not self.consume:
                    stream_seek(stream, -unit, 1, path)
                break
            data += b
        substream = BytesIOWithOffsets(data, stream, offset)
        return self.subcon._parsereport(substream, context, path)

    def _build(self, obj, stream, context, path):
        buildret = self.subcon._build(obj, stream, context, path)
        stream_write(stream, self.term, len(self.term), path)
        return buildret

    def _sizeof(self, context, path):
        raise SizeofError(path=path)

    def _emitfulltype(self, ksy, bitwise):
        if len(self.term) > 1:
            raise NotImplementedError
        return dict(terminator=byte2int(self.term), include=self.include, consume=self.consume, eos_error=self.require, **self.subcon._compilefulltype(ksy, bitwise))


class NullStripped(Subconstruct):
    r"""
    Restricts parsing to bytes except padding left of EOF.

    Parsing reads entire stream, then strips the data from right to left of null bytes, then parses subcon using new BytesIO made of said data. Building defers to subcon as-is. Size is undefined, because it reads till EOF.

    The pad can be multiple bytes, to support string classes with UTF16/32 encodings.

    :param subcon: Construct instance
    :param pad: optional, bytes, padding byte-string, default is \x00 single null byte

    :raises PaddingError: pad is less than 1 bytes in length

    Example::

        >>> d = NullStripped(Byte)
        >>> d.parse(b'\xff\x00\x00')
        255
        >>> d.build(255)
        b'\xff'
    """

    def __init__(self, subcon, pad=b"\x00"):
        super().__init__(subcon)
        self.pad = pad

    def _parse(self, stream, context, path):
        pad = self.pad
        unit = len(pad)
        if unit < 1:
            raise PaddingError("NullStripped pad must be at least 1 byte", path=path)
        offset = stream_tell(stream, path)
        data = stream_read_entire(stream, path)
        if unit == 1:
            data = data.rstrip(pad)
        else:
            tailunit = len(data) % unit
            end = len(data)
            if tailunit and data[-tailunit:] == pad[:tailunit]:
                end -= tailunit
            while end-unit >= 0 and data[end-unit:end] == pad:
                end -= unit
            data = data[:end]
        substream = BytesIOWithOffsets(data, stream, offset)
        return self.subcon._parsereport(substream, context, path)

    def _build(self, obj, stream, context, path):
        return self.subcon._build(obj, stream, context, path)

    def _sizeof(self, context, path):
        raise SizeofError(path=path)

    def _emitfulltype(self, ksy, bitwise):
        if len(self.pad) > 1:
            raise NotImplementedError
        return dict(pad_right=byte2int(self.pad), **self.subcon._compilefulltype(ksy, bitwise))


class RestreamData(Subconstruct):
    r"""
    Parses a field on external data (but does not build).

    Parsing defers to subcon, but provides it a separate BytesIO stream based on data provided by datafunc (a bytes literal or another BytesIO stream or Construct instances that returns bytes or context lambda). Building does nothing. Size is 0 because as far as other fields see it, this field does not produce or consume any bytes from the stream.

    :param datafunc: bytes or BytesIO or Construct instance (that parses into bytes) or context lambda, provides data for subcon to parse from
    :param subcon: Construct instance

    Can propagate any exception from the lambdas, possibly non-ConstructError.

    Example::

        >>> d = RestreamData(b"\x01", Int8ub)
        >>> d.parse(b"")
        1
        >>> d.build(0)
        b''

        >>> d = RestreamData(NullTerminated(GreedyBytes), Int16ub)
        >>> d.parse(b"\x01\x02\x00")
        0x0102
        >>> d = RestreamData(FixedSized(2, GreedyBytes), Int16ub)
        >>> d.parse(b"\x01\x02\x00")
        0x0102
    """

    def __init__(self, datafunc, subcon):
        super().__init__(subcon)
        self.datafunc = datafunc
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        data = evaluate(self.datafunc, context)
        if isinstance(data, bytestringtype):
            stream2 = io.BytesIO(data)
        if isinstance(data, io.BytesIO):
            stream2 = data
        if isinstance(data, Construct):
            stream2 = io.BytesIO(data._parsereport(stream, context, path))
        return self.subcon._parsereport(stream2, context, path)

    def _build(self, obj, stream, context, path):
        return obj

    def _sizeof(self, context, path):
        return 0

    def _emitparse(self, code):
        return "restream(%r, lambda io: %s)" % (self.datafunc, self.subcon._compileparse(code), )


class Transformed(Subconstruct):
    r"""
    Transforms bytes between the underlying stream and the (fixed-sized) subcon.

    Parsing reads a specified amount (or till EOF), processes data using a bytes-to-bytes decoding function, then parses subcon using those data. Building does build subcon into separate bytes, then processes it using encoding bytes-to-bytes function, then writes those data into main stream. Size is reported as `decodeamount` or `encodeamount` if those are equal, otherwise its SizeofError.

    Used internally to implement :class:`~construct.core.Bitwise` :class:`~construct.core.Bytewise` :class:`~construct.core.ByteSwapped` :class:`~construct.core.BitsSwapped` .

    Possible use-cases include encryption, obfuscation, byte-level encoding.

    .. warning:: Remember that subcon must consume (or produce) an amount of bytes that is same as `decodeamount` (or `encodeamount`).

    .. warning:: Do NOT use seeking/telling classes inside Transformed context.

    :param subcon: Construct instance
    :param decodefunc: bytes-to-bytes function, applied before parsing subcon
    :param decodeamount: integer, amount of bytes to read
    :param encodefunc: bytes-to-bytes function, applied after building subcon
    :param encodeamount: integer, amount of bytes to write

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises StreamError: subcon build and encoder transformed more or less than `encodeamount` bytes, if amount is specified
    :raises StringError: building from non-bytes value, perhaps unicode

    Can propagate any exception from the lambdas, possibly non-ConstructError.

    Example::

        >>> d = Transformed(Bytes(16), bytes2bits, 2, bits2bytes, 2)
        >>> d.parse(b"\x00\x00")
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'

        >>> d = Transformed(GreedyBytes, bytes2bits, None, bits2bytes, None)
        >>> d.parse(b"\x00\x00")
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    """

    def __init__(self, subcon, decodefunc, decodeamount, encodefunc, encodeamount):
        super().__init__(subcon)
        self.decodefunc = decodefunc
        self.decodeamount = decodeamount
        self.encodefunc = encodefunc
        self.encodeamount = encodeamount

    def _parse(self, stream, context, path):
        if isinstance(self.decodeamount, type(None)):
            data = stream_read_entire(stream, path)
        if isinstance(self.decodeamount, integertypes):
            data = stream_read(stream, self.decodeamount, path)
        data = self.decodefunc(data)
        return self.subcon._parsereport(io.BytesIO(data), context, path)

    def _build(self, obj, stream, context, path):
        stream2 = io.BytesIO()
        buildret = self.subcon._build(obj, stream2, context, path)
        data = stream2.getvalue()
        data = self.encodefunc(data)
        if isinstance(self.encodeamount, integertypes):
            if len(data) != self.encodeamount:
                raise StreamError("encoding transformation produced wrong amount of bytes, %s instead of expected %s" % (len(data), self.encodeamount, ), path=path)
        stream_write(stream, data, len(data), path)
        return buildret

    def _sizeof(self, context, path):
        if self.decodeamount is None or self.encodeamount is None:
            raise SizeofError(path=path)
        if self.decodeamount == self.encodeamount:
            return self.encodeamount
        raise SizeofError(path=path)


class Restreamed(Subconstruct):
    r"""
    Transforms bytes between the underlying stream and the (variable-sized) subcon.

    Used internally to implement :class:`~construct.core.Bitwise` :class:`~construct.core.Bytewise` :class:`~construct.core.ByteSwapped` :class:`~construct.core.BitsSwapped` .

    .. warning:: Remember that subcon must consume or produce an amount of bytes that is a multiple of encoding or decoding units. For example, in a Bitwise context you should process a multiple of 8 bits or the stream will fail during parsing/building.

    .. warning:: Do NOT use seeking/telling classes inside Restreamed context.

    :param subcon: Construct instance
    :param decoder: bytes-to-bytes function, used on data chunks when parsing
    :param decoderunit: integer, decoder takes chunks of this size
    :param encoder: bytes-to-bytes function, used on data chunks when building
    :param encoderunit: integer, encoder takes chunks of this size
    :param sizecomputer: function that computes amount of bytes outputed

    Can propagate any exception from the lambda, possibly non-ConstructError.
    Can also raise arbitrary exceptions in RestreamedBytesIO implementation.

    Example::

        Bitwise  <--> Restreamed(subcon, bits2bytes, 8, bytes2bits, 1, lambda n: n//8)
        Bytewise <--> Restreamed(subcon, bytes2bits, 1, bits2bytes, 8, lambda n: n*8)
    """

    def __init__(self, subcon, decoder, decoderunit, encoder, encoderunit, sizecomputer):
        super().__init__(subcon)
        self.decoder = decoder
        self.decoderunit = decoderunit
        self.encoder = encoder
        self.encoderunit = encoderunit
        self.sizecomputer = sizecomputer

    def _parse(self, stream, context, path):
        stream2 = RestreamedBytesIO(stream, self.decoder, self.decoderunit, self.encoder, self.encoderunit)
        obj = self.subcon._parsereport(stream2, context, path)
        stream2.close()
        return obj

    def _build(self, obj, stream, context, path):
        stream2 = RestreamedBytesIO(stream, self.decoder, self.decoderunit, self.encoder, self.encoderunit)
        buildret = self.subcon._build(obj, stream2, context, path)
        stream2.close()
        return obj

    def _sizeof(self, context, path):
        if self.sizecomputer is None:
            raise SizeofError("Restreamed cannot calculate size without a sizecomputer", path=path)
        else:
            return self.sizecomputer(self.subcon._sizeof(context, path))


class ProcessXor(Subconstruct):
    r"""
    Transforms bytes between the underlying stream and the subcon.

    Used internally by KaitaiStruct compiler, when translating `process: xor` tags.

    Parsing reads till EOF, xors data with the pad, then feeds that data into subcon. Building first builds the subcon into separate BytesIO stream, xors data with the pad, then writes that data into the main stream. Size is the same as subcon, unless it raises SizeofError.

    :param padfunc: integer or bytes or context lambda, single or multiple bytes to xor data with
    :param subcon: Construct instance

    :raises StringError: pad is not integer or bytes

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = ProcessXor(0xf0 or b'\xf0', Int16ub)
        >>> d.parse(b"\x00\xff")
        0xf00f
        >>> d.sizeof()
        2
    """

    def __init__(self, padfunc, subcon):
        super().__init__(subcon)
        self.padfunc = padfunc

    def _parse(self, stream, context, path):
        pad = evaluate(self.padfunc, context)
        if not isinstance(pad, (integertypes, bytestringtype)):
            raise StringError("ProcessXor needs integer or bytes pad", path=path)
        if isinstance(pad, bytestringtype) and len(pad) == 1:
            pad = byte2int(pad)
        offset = stream_tell(stream, path)
        data = stream_read_entire(stream, path)
        if isinstance(pad, integertypes):
            if not (pad == 0):
                data = integers2bytes( (b ^ pad) for b in data )
        if isinstance(pad, bytestringtype):
            if not (len(pad) <= 64 and pad == bytes(len(pad))):
                data = integers2bytes( (b ^ p) for b,p in zip(data, itertools.cycle(pad)) )
        substream = BytesIOWithOffsets(data, stream, offset)
        return self.subcon._parsereport(substream, context, path)

    def _build(self, obj, stream, context, path):
        pad = evaluate(self.padfunc, context)
        if not isinstance(pad, (integertypes, bytestringtype)):
            raise StringError("ProcessXor needs integer or bytes pad", path=path)
        if isinstance(pad, bytestringtype) and len(pad) == 1:
            pad = byte2int(pad)
        stream2 = io.BytesIO()
        buildret = self.subcon._build(obj, stream2, context, path)
        data = stream2.getvalue()
        if isinstance(pad, integertypes):
            if not (pad == 0):
                data = integers2bytes( (b ^ pad) for b in data )
        if isinstance(pad, bytestringtype):
            if not (len(pad) <= 64 and pad == bytes(len(pad))):
                data = integers2bytes( (b ^ p) for b,p in zip(data, itertools.cycle(pad)) )
        stream_write(stream, data, len(data), path)
        return buildret

    def _sizeof(self, context, path):
        return self.subcon._sizeof(context, path)


class ProcessRotateLeft(Subconstruct):
    r"""
    Transforms bytes between the underlying stream and the subcon.

    Used internally by KaitaiStruct compiler, when translating `process: rol/ror` tags.

    Parsing reads till EOF, rotates (shifts) the data *left* by amount in bits, then feeds that data into subcon. Building first builds the subcon into separate BytesIO stream, rotates *right* by negating amount, then writes that data into the main stream. Size is the same as subcon, unless it raises SizeofError.

    :param amount: integer or context lambda, shift by this amount in bits, treated modulo (group x 8)
    :param group: integer or context lambda, shifting is applied to chunks of this size in bytes
    :param subcon: Construct instance

    :raises RotationError: group is less than 1
    :raises RotationError: data length is not a multiple of group size

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = ProcessRotateLeft(4, 1, Int16ub)
        >>> d.parse(b'\x0f\xf0')
        0xf00f
        >>> d = ProcessRotateLeft(4, 2, Int16ub)
        >>> d.parse(b'\x0f\xf0')
        0xff00
        >>> d.sizeof()
        2
    """

    # formula taken from: http://stackoverflow.com/a/812039
    precomputed_single_rotations = {amount: [(i << amount) & 0xff | (i >> (8-amount)) for i in range(256)] for amount in range(1,8)}

    def __init__(self, amount, group, subcon):
        super().__init__(subcon)
        self.amount = amount
        self.group = group

    def _parse(self, stream, context, path):
        amount = evaluate(self.amount, context)
        group = evaluate(self.group, context)
        if group < 1:
            raise RotationError("group size must be at least 1 to be valid", path=path)

        amount = amount % (group * 8)
        amount_bytes = amount // 8
        data = stream_read_entire(stream, path)
        data_ints = bytes2integers(data)

        if len(data) % group != 0:
            raise RotationError("data length must be a multiple of group size", path=path)

        if amount == 0:
            pass

        elif group == 1:
            translate = ProcessRotateLeft.precomputed_single_rotations[amount]
            data = integers2bytes( translate[a] for a in data_ints )

        elif amount % 8 == 0:
            indices = [(i + amount_bytes) % group for i in range(group)]
            data = integers2bytes( data_ints[i+k] for i in range(0,len(data),group) for k in indices )

        else:
            amount1 = amount % 8
            amount2 = 8 - amount1
            indices_pairs = [ ((i+amount_bytes) % group, (i+1+amount_bytes) % group) for i in range(group)]
            data = integers2bytes( (data_ints[i+k1] << amount1) & 0xff | (data_ints[i+k2] >> amount2) for i in range(0,len(data),group) for k1,k2 in indices_pairs )

        return self.subcon._parsereport(io.BytesIO(data), context, path)

    def _build(self, obj, stream, context, path):
        amount = evaluate(self.amount, context)
        group = evaluate(self.group, context)
        if group < 1:
            raise RotationError("group size must be at least 1 to be valid", path=path)

        amount = -amount % (group * 8)
        amount_bytes = amount // 8
        stream2 = io.BytesIO()
        buildret = self.subcon._build(obj, stream2, context, path)
        data = stream2.getvalue()
        data_ints = bytes2integers(data)

        if len(data) % group != 0:
            raise RotationError("data length must be a multiple of group size", path=path)

        if amount == 0:
            pass

        elif group == 1:
            translate = ProcessRotateLeft.precomputed_single_rotations[amount]
            data = integers2bytes( translate[a] for a in data_ints )

        elif amount % 8 == 0:
            indices = [(i + amount_bytes) % group for i in range(group)]
            data = integers2bytes( data_ints[i+k] for i in range(0,len(data),group) for k in indices )

        else:
            amount1 = amount % 8
            amount2 = 8 - amount1
            indices_pairs = [ ((i+amount_bytes) % group, (i+1+amount_bytes) % group) for i in range(group)]
            data = integers2bytes( (data_ints[i+k1] << amount1) & 0xff | (data_ints[i+k2] >> amount2) for i in range(0,len(data),group) for k1,k2 in indices_pairs )

        stream_write(stream, data, len(data), path)
        return buildret

    def _sizeof(self, context, path):
        return self.subcon._sizeof(context, path)


class Checksum(Construct):
    r"""
    Field that is build or validated by a hash of a given byte range. Usually used with :class:`~construct.core.RawCopy` .

    Parsing compares parsed subcon `checksumfield` with a context entry provided by `bytesfunc` and transformed by `hashfunc`. Building fetches the contect entry, transforms it, then writes is using subcon. Size is same as subcon.

    :param checksumfield: a subcon field that reads the checksum, usually Bytes(int)
    :param hashfunc: function that takes bytes and returns whatever checksumfield takes when building, usually from hashlib module
    :param bytesfunc: context lambda that returns bytes (or object) to be hashed, usually like this.rawcopy1.data

    :raises ChecksumError: parsing and actual checksum does not match actual data

    Can propagate any exception from the lambdas, possibly non-ConstructError.

    Example::

        import hashlib
        d = Struct(
            "fields" / RawCopy(Struct(
                Padding(1000),
            )),
            "checksum" / Checksum(Bytes(64),
                lambda data: hashlib.sha512(data).digest(),
                this.fields.data),
        )
        d.build(dict(fields=dict(value={})))

    ::

        import hashlib
        d = Struct(
            "offset" / Tell,
            "checksum" / Padding(64),
            "fields" / RawCopy(Struct(
                Padding(1000),
            )),
            "checksum" / Pointer(this.offset, Checksum(Bytes(64),
                lambda data: hashlib.sha512(data).digest(),
                this.fields.data)),
        )
        d.build(dict(fields=dict(value={})))
    """

    def __init__(self, checksumfield, hashfunc, bytesfunc):
        super().__init__()
        self.checksumfield = checksumfield
        self.hashfunc = hashfunc
        self.bytesfunc = bytesfunc
        self.flagbuildnone = True

    def _parse(self, stream, context, path):
        hash1 = self.checksumfield._parsereport(stream, context, path)
        hash2 = self.hashfunc(self.bytesfunc(context))
        if hash1 != hash2:
            raise ChecksumError(
                "wrong checksum, read %r, computed %r" % (
                    hash1 if not isinstance(hash1,bytestringtype) else binascii.hexlify(hash1),
                    hash2 if not isinstance(hash2,bytestringtype) else binascii.hexlify(hash2), ),
                path=path
            )
        return hash1

    def _build(self, obj, stream, context, path):
        hash2 = self.hashfunc(self.bytesfunc(context))
        self.checksumfield._build(hash2, stream, context, path)
        return hash2

    def _sizeof(self, context, path):
        return self.checksumfield._sizeof(context, path)


class Compressed(Tunnel):
    r"""
    Compresses and decompresses underlying stream when processing subcon. When parsing, entire stream is consumed. When building, it puts compressed bytes without marking the end. This construct should be used with :class:`~construct.core.Prefixed` .

    Parsing and building transforms all bytes using a specified codec. Since data is processed until EOF, it behaves similar to `GreedyBytes`. Size is undefined.

    :param subcon: Construct instance, subcon used for storing the value
    :param encoding: string, any of module names like zlib/gzip/bzip2/lzma, otherwise any of codecs module bytes<->bytes encodings, each codec usually requires some Python version
    :param level: optional, integer between 0..9, although lzma discards it, some encoders allow different compression levels

    :raises ImportError: needed module could not be imported by ctor
    :raises StreamError: stream failed when reading until EOF

    Example::

        >>> d = Prefixed(VarInt, Compressed(GreedyBytes, "zlib"))
        >>> d.build(bytes(100))
        b'\x0cx\x9cc`\xa0=\x00\x00\x00d\x00\x01'
        >>> len(_)
        13
   """

    def __init__(self, subcon, encoding, level=None):
        super().__init__(subcon)
        self.encoding = encoding
        self.level = level
        if self.encoding == "zlib":
            import zlib
            self.lib = zlib
        elif self.encoding == "gzip":
            import gzip
            self.lib = gzip
        elif self.encoding == "bzip2":
            import bz2
            self.lib = bz2
        elif self.encoding == "lzma":
            import lzma
            self.lib = lzma
        else:
            import codecs
            self.lib = codecs

    def _decode(self, data, context, path):
        if self.encoding in ("zlib", "gzip", "bzip2", "lzma"):
            return self.lib.decompress(data)
        return self.lib.decode(data, self.encoding)

    def _encode(self, data, context, path):
        if self.encoding in ("zlib", "gzip", "bzip2", "lzma"):
            if self.level is None or self.encoding == "lzma":
                return self.lib.compress(data)
            else:
                return self.lib.compress(data, self.level)
        return self.lib.encode(data, self.encoding)


class CompressedLZ4(Tunnel):
    r"""
    Compresses and decompresses underlying stream before processing subcon. When parsing, entire stream is consumed. When building, it puts compressed bytes without marking the end. This construct should be used with :class:`~construct.core.Prefixed` .

    Parsing and building transforms all bytes using LZ4 library. Since data is processed until EOF, it behaves similar to `GreedyBytes`. Size is undefined.

    :param subcon: Construct instance, subcon used for storing the value

    :raises ImportError: needed module could not be imported by ctor
    :raises StreamError: stream failed when reading until EOF

    Can propagate lz4.frame exceptions.

    Example::

        >>> d = Prefixed(VarInt, CompressedLZ4(GreedyBytes))
        >>> d.build(bytes(100))
        b'"\x04"M\x18h@d\x00\x00\x00\x00\x00\x00\x00#\x0b\x00\x00\x00\x1f\x00\x01\x00KP\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        >>> len(_)
        35
   """

    def __init__(self, subcon):
        super().__init__(subcon)
        import lz4.frame
        self.lib = lz4.frame

    def _decode(self, data, context, path):
        return self.lib.decompress(data)

    def _encode(self, data, context, path):
        return self.lib.compress(data)


class EncryptedSym(Tunnel):
    r"""
    Perform symmetrical encryption and decryption of the underlying stream before processing subcon. When parsing, entire stream is consumed. When building, it puts encrypted bytes without marking the end.

    Parsing and building transforms all bytes using the selected cipher. Since data is processed until EOF, it behaves similar to `GreedyBytes`. Size is undefined.

    The key for encryption and decryption should be passed via `contextkw` to `build` and `parse` methods.

    This construct is heavily based on the `cryptography` library, which supports the following algorithms and modes. For more details please see the documentation of that library.
    
    Algorithms:
    - AES
    - Camellia
    - ChaCha20
    - TripleDES
    - CAST5
    - SEED
    - SM4
    - Blowfish (weak cipher)
    - ARC4 (weak cipher)
    - IDEA (weak cipher)

    Modes:
    - CBC
    - CTR
    - OFB
    - CFB
    - CFB8
    - XTS
    - ECB (insecure)

    .. note:: Keep in mind that some of the algorithms require padding of the data. This can be done e.g. with :class:`~construct.core.Aligned`.
    .. note:: For GCM mode use :class:`~construct.core.EncryptedSymAead`.

    :param subcon: Construct instance, subcon used for storing the value
    :param cipher: Cipher object or context lambda from cryptography.hazmat.primitives.ciphers

    :raises ImportError: needed module could not be imported
    :raises StreamError: stream failed when reading until EOF
    :raises CipherError: no cipher object is provided
    :raises CipherError: an AEAD cipher is used

    Can propagate cryptography.exceptions exceptions.

    Example::

        >>> from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
        >>> d = Struct(
        ...     "iv" / Default(Bytes(16), os.urandom(16)),
        ...     "enc_data" / EncryptedSym(
        ...         Aligned(16,
        ...             Struct(
        ...                 "width" / Int16ul,
        ...                 "height" / Int16ul,
        ...             )
        ...         ),
        ...         lambda ctx: Cipher(algorithms.AES(ctx._.key), modes.CBC(ctx.iv))
        ...     )
        ... )
        >>> key128 = b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
        >>> d.build({"enc_data": {"width": 5, "height": 4}}, key=key128)
        b"o\x11i\x98~H\xc9\x1c\x17\x83\xf6|U:\x1a\x86+\x00\x89\xf7\x8e\xc3L\x04\t\xca\x8a\xc8\xc2\xfb'\xc8"
        >>> d.parse(b"o\x11i\x98~H\xc9\x1c\x17\x83\xf6|U:\x1a\x86+\x00\x89\xf7\x8e\xc3L\x04\t\xca\x8a\xc8\xc2\xfb'\xc8", key=key128)
        Container: 
            iv = b'o\x11i\x98~H\xc9\x1c\x17\x83\xf6|U:\x1a\x86' (total 16)
            enc_data = Container: 
                width = 5
                height = 4
   """

    def __init__(self, subcon, cipher):
        import cryptography
        super().__init__(subcon)
        self.cipher = cipher

    def _evaluate_cipher(self, context, path):
        from cryptography.hazmat.primitives.ciphers import Cipher, modes
        cipher = evaluate(self.cipher, context)
        if not isinstance(cipher, Cipher):
            raise CipherError(f"cipher {repr(cipher)} is not a cryptography.hazmat.primitives.ciphers.Cipher object", path=path)
        if isinstance(cipher.mode, modes.GCM):
            raise CipherError(f"AEAD cipher is not supported in this class, use EncryptedSymAead", path=path)
        return cipher

    def _decode(self, data, context, path):
        cipher = self._evaluate_cipher(context, path)
        decryptor = cipher.decryptor()
        return decryptor.update(data) + decryptor.finalize()

    def _encode(self, data, context, path):
        cipher = self._evaluate_cipher(context, path)
        encryptor = cipher.encryptor()
        return encryptor.update(data) + encryptor.finalize()


class EncryptedSymAead(Tunnel):
    r"""
    Perform symmetrical AEAD encryption and decryption of the underlying stream before processing subcon. When parsing, entire stream is consumed. When building, it puts encrypted bytes and tag without marking the end.

    Parsing and building transforms all bytes using the selected cipher and also authenticates the `associated_data`. Since data is processed until EOF, it behaves similar to `GreedyBytes`. Size is undefined.

    The key for encryption and decryption should be passed via `contextkw` to `build` and `parse` methods.

    This construct is heavily based on the `cryptography` library, which supports the following AEAD ciphers. For more details please see the documentation of that library.
    
    AEAD ciphers:
    - AESGCM
    - AESCCM
    - ChaCha20Poly1305

    :param subcon: Construct instance, subcon used for storing the value
    :param cipher: Cipher object or context lambda from cryptography.hazmat.primitives.ciphers

    :raises ImportError: needed module could not be imported
    :raises StreamError: stream failed when reading until EOF
    :raises CipherError: unsupported cipher object is provided

    Can propagate cryptography.exceptions exceptions.

    Example::

        >>> from cryptography.hazmat.primitives.ciphers import aead
        >>> d = Struct(
        ...     "nonce" / Default(Bytes(16), os.urandom(16)),
        ...     "associated_data" / Bytes(21),
        ...     "enc_data" / EncryptedSymAead(
        ...         GreedyBytes,
        ...         lambda ctx: aead.AESGCM(ctx._.key),
        ...         this.nonce,
        ...         this.associated_data
        ...     )
        ... )
        >>> key128 = b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
        >>> d.build({"associated_data": b"This is authenticated", "enc_data": b"The secret message"}, key=key128)
        b'\xe3\xb0"\xbaQ\x18\xd3|\x14\xb0q\x11\xb5XZ\xeeThis is authenticated\x88~\xe5Vh\x00\x01m\xacn\xad k\x02\x13\xf4\xb4[\xbe\x12$\xa0\x7f\xfb\xbf\x82Ar\xb0\x97C\x0b\xe3\x85'
        >>> d.parse(b'\xe3\xb0"\xbaQ\x18\xd3|\x14\xb0q\x11\xb5XZ\xeeThis is authenticated\x88~\xe5Vh\x00\x01m\xacn\xad k\x02\x13\xf4\xb4[\xbe\x12$\xa0\x7f\xfb\xbf\x82Ar\xb0\x97C\x0b\xe3\x85', key=key128)
        Container: 
            nonce = b'\xe3\xb0"\xbaQ\x18\xd3|\x14\xb0q\x11\xb5XZ\xee' (total 16)
            associated_data = b'This is authenti'... (truncated, total 21)
            enc_data = b'The secret messa'... (truncated, total 18)
   """

    def __init__(self, subcon, cipher, nonce, associated_data=b""):
        super().__init__(subcon)
        self.cipher = cipher
        self.nonce = nonce
        self.associated_data = associated_data

    def _evaluate_cipher(self, context, path):
        from cryptography.hazmat.primitives.ciphers.aead import AESGCM, AESCCM, ChaCha20Poly1305
        cipher = evaluate(self.cipher, context)
        if not isinstance(cipher, (AESGCM, AESCCM, ChaCha20Poly1305)):
            raise CipherError(f"cipher object {repr(cipher)} is not supported", path=path)
        return cipher

    def _decode(self, data, context, path):
        cipher = self._evaluate_cipher(context, path)
        nonce = evaluate(self.nonce, context)
        associated_data = evaluate(self.associated_data, context)
        return cipher.decrypt(nonce, data, associated_data)

    def _encode(self, data, context, path):
        cipher = self._evaluate_cipher(context, path)
        nonce = evaluate(self.nonce, context)
        associated_data = evaluate(self.associated_data, context)
        return cipher.encrypt(nonce, data, associated_data)


class Rebuffered(Subconstruct):
    r"""
    Caches bytes from underlying stream, so it becomes seekable and tellable, and also becomes blocking on reading. Useful for processing non-file streams like pipes, sockets, etc.

    .. warning:: Experimental implementation. May not be mature enough.

    :param subcon: Construct instance, subcon which will operate on the buffered stream
    :param tailcutoff: optional, integer, amount of bytes kept in buffer, by default buffers everything

    Can also raise arbitrary exceptions in its implementation.

    Example::

        Rebuffered(..., tailcutoff=1024).parse_stream(nonseekable_stream)
    """

    def __init__(self, subcon, tailcutoff=None):
        super().__init__(subcon)
        self.stream2 = RebufferedBytesIO(None, tailcutoff=tailcutoff)

    def _parse(self, stream, context, path):
        self.stream2.substream = stream
        return self.subcon._parsereport(self.stream2, context, path)

    def _build(self, obj, stream, context, path):
        self.stream2.substream = stream
        return self.subcon._build(obj, self.stream2, context, path)


#===============================================================================
# lazy equivalents
#===============================================================================
class Lazy(Subconstruct):
    r"""
    Lazyfies a field.

    This wrapper allows you to do lazy parsing of individual fields inside a normal Struct (without using LazyStruct which may not work in every scenario). It is also used by KaitaiStruct compiler to emit `instances` because those are not processed greedily, and they may refer to other not yet parsed fields. Those are 2 entirely different applications but semantics are the same.

    Parsing saves the current stream offset and returns a lambda. If and when that lambda gets evaluated, it seeks the stream to then-current position, parses the subcon, and seeks the stream back to previous position. Building evaluates that lambda into an object (if needed), then defers to subcon. Size also defers to subcon.

    :param subcon: Construct instance

    :raises StreamError: requested reading negative amount, could not read enough bytes, requested writing different amount than actual data, or could not write all bytes
    :raises StreamError: stream is not seekable and tellable

    Example::

        >>> d = Lazy(Byte)
        >>> x = d.parse(b'\x00')
        >>> x
        <function construct.core.Lazy._parse.<locals>.execute>
        >>> x()
        0
        >>> d.build(0)
        b'\x00'
        >>> d.build(x)
        b'\x00'
        >>> d.sizeof()
        1
    """

    def __init__(self, subcon):
        super().__init__(subcon)

    def _parse(self, stream, context, path):
        offset = stream_tell(stream, path)
        def execute():
            fallback = stream_tell(stream, path)
            stream_seek(stream, offset, 0, path)
            obj = self.subcon._parsereport(stream, context, path)
            stream_seek(stream, fallback, 0, path)
            return obj
        len = self.subcon._actualsize(stream, context, path)
        stream_seek(stream, len, 1, path)
        return execute

    def _build(self, obj, stream, context, path):
        if callable(obj):
            obj = obj()
        return self.subcon._build(obj, stream, context, path)


class LazyContainer(dict):
    """Used internally."""

    def __init__(self, struct, stream, offsets, values, context, path):
        self._struct = struct
        self._stream = stream
        self._offsets = offsets
        self._values = values
        self._context = context
        self._path = path

    def __getattr__(self, name):
        if name in self._struct._subconsindexes:
            return self[name]
        raise AttributeError

    def __getitem__(self, index):
        if isinstance(index, stringtypes):
            index = self._struct._subconsindexes[index] # KeyError
        if index in self._values:
            return self._values[index]
        stream_seek(self._stream, self._offsets[index], 0, self._path) # KeyError
        parseret = self._struct.subcons[index]._parsereport(self._stream, self._context, self._path)
        self._values[index] = parseret
        return parseret

    def __len__(self):
        return len(self._struct.subcons)

    def keys(self):
        return iter(self._struct._subcons)

    def values(self):
        return (self[k] for k in self._struct._subcons)

    def items(self):
        return ((k, self[k]) for k in self._struct._subcons)

    __iter__ = keys

    def __eq__(self, other):
        return Container.__eq__(self, other)

    def __repr__(self):
        return "<LazyContainer: %s items cached, %s subcons>" % (len(self._values), len(self._struct.subcons), )


class LazyStruct(Construct):
    r"""
    Equivalent to :class:`~construct.core.Struct`, but when this class is parsed, most fields are not parsed (they are skipped if their size can be measured by _actualsize or _sizeof method). See its docstring for details.

    Fields are parsed depending on some factors:

    * Some fields like Int* Float* Bytes(5) Array(5,Byte) Pointer are fixed-size and are therefore skipped. Stream is not read.
    * Some fields like Bytes(this.field) are variable-size but their size is known during parsing when there is a corresponding context entry. Those fields are also skipped. Stream is not read.
    * Some fields like Prefixed PrefixedArray PascalString are variable-size but their size can be computed by partially reading the stream. Only first few bytes are read (the lengthfield).
    * Other fields like VarInt need to be parsed. Stream position that is left after the field was parsed is used.
    * Some fields may not work properly, due to the fact that this class attempts to skip fields, and parses them only out of necessity. Miscellaneous fields often have size defined as 0, and fixed sized fields are skippable.

    Note there are restrictions:

    * If a field like Bytes(this.field) references another field in the same struct, you need to access the referenced field first (to trigger its parsing) and then you can access the Bytes field. Otherwise it would fail due to missing context entry.
    * If a field references another field within inner (nested) or outer (super) struct, things may break. Context is nested, but this class was not rigorously tested in that manner.

    Building and sizeof are greedy, like in Struct.

    :param \*subcons: Construct instances, list of members, some can be anonymous
    :param \*\*subconskw: Construct instances, list of members (requires Python 3.6)
    """

    def __init__(self, *subcons, **subconskw):
        super().__init__()
        self.subcons = list(subcons) + list(k/v for k,v in subconskw.items())
        self._subcons = Container((sc.name,sc) for sc in self.subcons if sc.name)
        self._subconsindexes = Container((sc.name,i) for i,sc in enumerate(self.subcons) if sc.name)
        self.flagbuildnone = all(sc.flagbuildnone for sc in self.subcons)

    def __getattr__(self, name):
        if name in self._subcons:
            return self._subcons[name]
        raise AttributeError

    def _parse(self, stream, context, path):
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        offset = stream_tell(stream, path)
        offsets = {0: offset}
        values = {}
        for i,sc in enumerate(self.subcons):
            try:
                offset += sc._actualsize(stream, context, path)
                stream_seek(stream, offset, 0, path)
            except SizeofError:
                parseret = sc._parsereport(stream, context, path)
                values[i] = parseret
                if sc.name:
                    context[sc.name] = parseret
                offset = stream_tell(stream, path)
            offsets[i+1] = offset
        return LazyContainer(self, stream, offsets, values, context, path)

    def _build(self, obj, stream, context, path):
        # exact copy from Struct class
        if obj is None:
            obj = Container()
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = stream, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        context.update(obj)
        for sc in self.subcons:
            try:
                if sc.flagbuildnone:
                    subobj = obj.get(sc.name, None)
                else:
                    subobj = obj[sc.name] # raises KeyError

                if sc.name:
                    context[sc.name] = subobj

                buildret = sc._build(subobj, stream, context, path)
                if sc.name:
                    context[sc.name] = buildret
            except StopFieldError:
                break
        return context

    def _sizeof(self, context, path):
        # exact copy from Struct class
        context = Container(_ = context, _params = context._params, _root = None, _parsing = context._parsing, _building = context._building, _sizing = context._sizing, _subcons = self._subcons, _io = None, _index = context.get("_index", None))
        context._root = context._.get("_root", context)
        try:
            return sum(sc._sizeof(context, path) for sc in self.subcons)
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)


class LazyListContainer(list):
    """Used internally."""

    def __init__(self, subcon, stream, count, offsets, values, context, path):
        self._subcon = subcon
        self._stream = stream
        self._count = count
        self._offsets = offsets
        self._values = values
        self._context = context
        self._path = path

    def __getitem__(self, index):
        if isinstance(index, slice):
            return [self[i] for i in range(*index.indices(self._count))]
        if index in self._values:
            return self._values[index]
        stream_seek(self._stream, self._offsets[index], 0, self._path) # KeyError
        parseret = self._subcon._parsereport(self._stream, self._context, self._path)
        self._values[index] = parseret
        return parseret

    def __getslice__(self, start, stop):
        if stop == sys.maxsize:
            stop = self._count
        return self.__getitem__(slice(start, stop))

    def __len__(self):
        return self._count

    def __iter__(self):
        return (self[i] for i in range(self._count))

    def __eq__(self, other):
        return len(self) == len(other) and all(self[i] == other[i] for i in range(self._count))

    def __repr__(self):
        return "<LazyListContainer: %s of %s items cached>" % (len(self._values), self._count, )


class LazyArray(Subconstruct):
    r"""
    Equivalent to :class:`~construct.core.Array`, but the subcon is not parsed when possible (it gets skipped if the size can be measured by _actualsize or _sizeof method). See its docstring for details.

    Fields are parsed depending on some factors:

    * Some fields like Int* Float* Bytes(5) Array(5,Byte) Pointer are fixed-size and are therefore skipped. Stream is not read.
    * Some fields like Bytes(this.field) are variable-size but their size is known during parsing when there is a corresponding context entry. Those fields are also skipped. Stream is not read.
    * Some fields like Prefixed PrefixedArray PascalString are variable-size but their size can be computed by partially reading the stream. Only first few bytes are read (the lengthfield).
    * Other fields like VarInt need to be parsed. Stream position that is left after the field was parsed is used.
    * Some fields may not work properly, due to the fact that this class attempts to skip fields, and parses them only out of necessity. Miscellaneous fields often have size defined as 0, and fixed sized fields are skippable.

    Note there are restrictions:

    * If a field references another field within inner (nested) or outer (super) struct, things may break. Context is nested, but this class was not rigorously tested in that manner.

    Building and sizeof are greedy, like in Array.

    :param count: integer or context lambda, strict amount of elements
    :param subcon: Construct instance, subcon to process individual elements
    """

    def __init__(self, count, subcon):
        super().__init__(subcon)
        self.count = count

    def _parse(self, stream, context, path):
        sc = self.subcon
        count = self.count
        if callable(count):
            count = count(context)
        if not 0 <= count:
            raise RangeError("invalid count %s" % (count,), path=path)
        offset = stream_tell(stream, path)
        offsets = {0: offset}
        values = {}
        for i in range(count):
            try:
                offset += sc._actualsize(stream, context, path)
                stream_seek(stream, offset, 0, path)
            except SizeofError:
                parseret = sc._parsereport(stream, context, path)
                values[i] = parseret
                offset = stream_tell(stream, path)
            offsets[i+1] = offset
        return LazyListContainer(sc, stream, count, offsets, values, context, path)

    def _build(self, obj, stream, context, path):
        # exact copy from Array class
        count = self.count
        if callable(count):
            count = count(context)
        if not 0 <= count:
            raise RangeError("invalid count %s" % (count,), path=path)
        if not len(obj) == count:
            raise RangeError("expected %d elements, found %d" % (count, len(obj)), path=path)
        retlist = ListContainer()
        for i,e in enumerate(obj):
            context._index = i
            buildret = self.subcon._build(e, stream, context, path)
            retlist.append(buildret)
        return retlist

    def _sizeof(self, context, path):
        # exact copy from Array class
        try:
            count = self.count
            if callable(count):
                count = count(context)
        except (KeyError, AttributeError):
            raise SizeofError("cannot calculate size, key not found in context", path=path)
        return count * self.subcon._sizeof(context, path)


class LazyBound(Construct):
    r"""
    Field that binds to the subcon only at runtime (during parsing and building, not ctor). Useful for recursive data structures, like linked-lists and trees, where a construct needs to refer to itself (while it does not exist yet in the namespace).

    Note that it is possible to obtain same effect without using this class, using a loop. However there are usecases where that is not possible (if remaining nodes cannot be sized-up, and there is data following the recursive structure). There is also a significant difference, namely that LazyBound actually does greedy parsing while the loop does lazy parsing. See examples.

    To break recursion, use `If` field. See examples.

    :param subconfunc: parameter-less lambda returning Construct instance, can also return itself

    Example::

        d = Struct(
            "value" / Byte,
            "next" / If(this.value > 0, LazyBound(lambda: d)),
        )
        >>> print(d.parse(b"\x05\x09\x00"))
        Container:
            value = 5
            next = Container:
                value = 9
                next = Container:
                    value = 0
                    next = None

    ::

        d = Struct(
            "value" / Byte,
            "next" / GreedyBytes,
        )
        data = b"\x05\x09\x00"
        while data:
            x = d.parse(data)
            data = x.next
            print(x)
        # print outputs
        Container:
            value = 5
            next = \t\x00 (total 2)
        # print outputs
        Container:
            value = 9
            next = \x00 (total 1)
        # print outputs
        Container:
            value = 0
            next =  (total 0)
    """

    def __init__(self, subconfunc):
        super().__init__()
        self.subconfunc = subconfunc

    def _parse(self, stream, context, path):
        sc = self.subconfunc()
        return sc._parsereport(stream, context, path)

    def _build(self, obj, stream, context, path):
        sc = self.subconfunc()
        return sc._build(obj, stream, context, path)


#===============================================================================
# adapters and validators
#===============================================================================
class ExprAdapter(Adapter):
    r"""
    Generic adapter that takes `decoder` and `encoder` lambdas as parameters. You can use ExprAdapter instead of writing a full-blown class deriving from Adapter when only a simple lambda is needed.

    :param subcon: Construct instance, subcon to adapt
    :param decoder: lambda that takes (obj, context) and returns an decoded version of obj
    :param encoder: lambda that takes (obj, context) and returns an encoded version of obj

    Example::

        >>> d = ExprAdapter(Byte, obj_+1, obj_-1)
        >>> d.parse(b'\x04')
        5
        >>> d.build(5)
        b'\x04'
    """
    def __init__(self, subcon, decoder, encoder):
        super().__init__(subcon)
        self._decode = lambda obj,ctx,path: decoder(obj,ctx)
        self._encode = lambda obj,ctx,path: encoder(obj,ctx)


class ExprSymmetricAdapter(ExprAdapter):
    """
    Macro around :class:`~construct.core.ExprAdapter`.

    :param subcon: Construct instance, subcon to adapt
    :param encoder: lambda that takes (obj, context) and returns both encoded version and decoded version of obj

    Example::

        >>> d = ExprSymmetricAdapter(Byte, obj_ & 0b00001111)
        >>> d.parse(b"\xff")
        15
        >>> d.build(255)
        b'\x0f'
    """
    def __init__(self, subcon, encoder):
        super().__init__(subcon, encoder, encoder)


class ExprValidator(Validator):
    r"""
    Generic adapter that takes `validator` lambda as parameter. You can use ExprValidator instead of writing a full-blown class deriving from Validator when only a simple lambda is needed.

    :param subcon: Construct instance, subcon to adapt
    :param validator: lambda that takes (obj, context) and returns a bool

    Example::

        >>> d = ExprValidator(Byte, obj_ & 0b11111110 == 0)
        >>> d.build(1)
        b'\x01'
        >>> d.build(88)
        ValidationError: object failed validation: 88

    """
    def __init__(self, subcon, validator):
        super().__init__(subcon)
        self._validate = lambda obj,ctx,path: validator(obj,ctx)


def OneOf(subcon, valids):
    r"""
    Validates that the object is one of the listed values, both during parsing and building.

    .. note:: For performance, `valids` should be a set or frozenset.

    :param subcon: Construct instance, subcon to validate
    :param valids: collection implementing __contains__, usually a list or set

    :raises ValidationError: parsed or build value is not among valids

    Example::

        >>> d = OneOf(Byte, [1,2,3])
        >>> d.parse(b"\x01")
        1
        >>> d.parse(b"\xff")
        construct.core.ValidationError: object failed validation: 255
    """
    return ExprValidator(subcon, lambda obj,ctx: obj in valids)


def NoneOf(subcon, invalids):
    r"""
    Validates that the object is none of the listed values, both during parsing and building.

    .. note:: For performance, `valids` should be a set or frozenset.

    :param subcon: Construct instance, subcon to validate
    :param invalids: collection implementing __contains__, usually a list or set

    :raises ValidationError: parsed or build value is among invalids

    """
    return ExprValidator(subcon, lambda obj,ctx: obj not in invalids)


def Filter(predicate, subcon):
    r"""
    Filters a list leaving only the elements that passed through the predicate.

    :param subcon: Construct instance, usually Array GreedyRange Sequence
    :param predicate: lambda that takes (obj, context) and returns a bool

    Can propagate any exception from the lambda, possibly non-ConstructError.

    Example::

        >>> d = Filter(obj_ != 0, Byte[:])
        >>> d.parse(b"\x00\x02\x00")
        [2]
        >>> d.build([0,1,0,2,0])
        b'\x01\x02'
    """
    return ExprSymmetricAdapter(subcon, lambda obj,ctx: [x for x in obj if predicate(x,ctx)])


class Slicing(Adapter):
    r"""
    Adapter for slicing a list. Works with GreedyRange and Sequence.

    :param subcon: Construct instance, subcon to slice
    :param count: integer, expected number of elements, needed during building
    :param start: integer for start index (or None for entire list)
    :param stop: integer for stop index (or None for up-to-end)
    :param step: integer, step (or 1 for every element)
    :param empty: object, value to fill the list with, during building

    Example::

        d = Slicing(Array(4,Byte), 4, 1, 3, empty=0)
        assert d.parse(b"\x01\x02\x03\x04") == [2,3]
        assert d.build([2,3]) == b"\x00\x02\x03\x00"
        assert d.sizeof() == 4
    """
    def __init__(self, subcon, count, start, stop, step=1, empty=None):
        super().__init__(subcon)
        self.count = count
        self.start = start
        self.stop = stop
        self.step = step
        self.empty = empty
    def _decode(self, obj, context, path):
        return obj[self.start:self.stop:self.step]
    def _encode(self, obj, context, path):
        if self.start is None:
            return obj
        elif self.stop is None:
            output = [self.empty] * self.count
            output[self.start::self.step] = obj
        else:
            output = [self.empty] * self.count
            output[self.start:self.stop:self.step] = obj
        return output


class Indexing(Adapter):
    r"""
    Adapter for indexing a list (getting a single item from that list). Works with Range and Sequence and their lazy equivalents.

    :param subcon: Construct instance, subcon to index
    :param count: integer, expected number of elements, needed during building
    :param index: integer, index of the list to get
    :param empty: object, value to fill the list with, during building

    Example::

        d = Indexing(Array(4,Byte), 4, 2, empty=0)
        assert d.parse(b"\x01\x02\x03\x04") == 3
        assert d.build(3) == b"\x00\x00\x03\x00"
        assert d.sizeof() == 4
    """
    def __init__(self, subcon, count, index, empty=None):
        super().__init__(subcon)
        self.count = count
        self.index = index
        self.empty = empty
    def _decode(self, obj, context, path):
        return obj[self.index]
    def _encode(self, obj, context, path):
        output = [self.empty] * self.count
        output[self.index] = obj
        return output


#===============================================================================
# end of file
#===============================================================================
