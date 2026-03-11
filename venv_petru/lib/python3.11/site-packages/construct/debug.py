from construct import *
from construct.lib import *
import sys, traceback, pdb, inspect


class Probe(Construct):
    r"""
    Probe that dumps the context, and some stream content (peeks into it) to the screen to aid the debugging process. It can optionally limit itself to a single context entry, instead of printing entire context.

    :param into: optional, None by default, or context lambda
    :param lookahead: optional, integer, number of bytes to dump from the stream

    Example::

        >>> d = Struct(
        ...     "count" / Byte,
        ...     "items" / Byte[this.count],
        ...     Probe(lookahead=32),
        ... )
        >>> d.parse(b"\x05abcde\x01\x02\x03")

        --------------------------------------------------
        Probe, path is (parsing), into is None
        Stream peek: (hexlified) b'010203'...
        Container: 
            count = 5
            items = ListContainer: 
                97
                98
                99
                100
                101
        --------------------------------------------------

    ::

        >>> d = Struct(
        ...     "count" / Byte,
        ...     "items" / Byte[this.count],
        ...     Probe(this.count),
        ... )
        >>> d.parse(b"\x05abcde\x01\x02\x03")

        --------------------------------------------------
        Probe, path is (parsing), into is this.count
        5
        --------------------------------------------------

    """

    def __init__(self, into=None, lookahead=None):
        super(Probe, self).__init__()
        self.flagbuildnone = True
        self.into = into
        self.lookahead = lookahead

    def _parse(self, stream, context, path):
        self.printout(stream, context, path)

    def _build(self, obj, stream, context, path):
        self.printout(stream, context, path)

    def _sizeof(self, context, path):
        self.printout(None, context, path)
        return 0

    def _emitparse(self, code):
        return f"print({self.into})" if self.into else "print(this)"

    def _emitbuild(self, code):
        return f"print({self.into})" if self.into else "print(this)"

    def printout(self, stream, context, path):
        print("--------------------------------------------------")
        print("Probe, path is %s, into is %r" % (path, self.into, ))

        if self.lookahead and stream is not None:
            fallback = stream.tell()
            datafollows = stream.read(self.lookahead)
            stream.seek(fallback)
            if datafollows:
                print("Stream peek: (hexlified) %s..." % (hexlify(datafollows), ))
            else:
                print("Stream peek: EOF reached")

        if context is not None:
            if self.into:
                try:
                    subcontext = self.into(context)
                    print(subcontext)
                except Exception:
                    print("Failed to compute %r on the context %r" % (self.into, context, ))
            else:
                print(context)
        print("--------------------------------------------------")


class Debugger(Subconstruct):
    r"""
    PDB-based debugger. When an exception occurs in the subcon, a debugger will appear and allow you to debug the error (and even fix it on-the-fly).
    
    :param subcon: Construct instance, subcon to debug
    
    Example::
    
        >>> Debugger(Byte[3]).build([])

        --------------------------------------------------
        Debugging exception of <Array: None>
        path is (building)
          File "/media/arkadiusz/MAIN/GitHub/construct/construct/debug.py", line 192, in _build
            return self.subcon._build(obj, stream, context, path)
          File "/media/arkadiusz/MAIN/GitHub/construct/construct/core.py", line 2149, in _build
            raise RangeError("expected %d elements, found %d" % (count, len(obj)))
        construct.core.RangeError: expected 3 elements, found 0

        > /media/arkadiusz/MAIN/GitHub/construct/construct/core.py(2149)_build()
        -> raise RangeError("expected %d elements, found %d" % (count, len(obj)))
        (Pdb) q
        --------------------------------------------------
    """

    def _parse(self, stream, context, path):
        try:
            return self.subcon._parse(stream, context, path)
        except Exception:
            self.retval = NotImplemented
            self.handle_exc(path, msg="(you can set self.retval, which will be returned from method)")
            if self.retval is NotImplemented:
                raise
            else:
                return self.retval

    def _build(self, obj, stream, context, path):
        try:
            return self.subcon._build(obj, stream, context, path)
        except Exception:
            self.handle_exc(path)

    def _sizeof(self, context, path):
        try:
            return self.subcon._sizeof(context, path)
        except Exception:
            self.handle_exc(path)

    def _emitparse(self, code):
        return self.subcon._compileparse(code)

    def _emitbuild(self, code):
        return self.subcon._compilebuild(code)

    def handle_exc(self, path, msg=None):
        print("--------------------------------------------------")
        print("Debugging exception of %r" % (self.subcon, ))
        print("path is %s" % (path, ))
        print("".join(traceback.format_exception(*sys.exc_info())[1:]))
        if msg:
            print(msg)
        pdb.post_mortem(sys.exc_info()[2])
        print("--------------------------------------------------")
