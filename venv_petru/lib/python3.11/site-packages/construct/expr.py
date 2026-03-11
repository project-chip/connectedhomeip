import operator
if not hasattr(operator, "div"):
    operator.div = operator.truediv


opnames = {
    operator.add : "+",
    operator.sub : "-",
    operator.mul : "*",
    operator.div : "/",
    operator.floordiv : "//",
    operator.mod : "%",
    operator.pow : "**",
    operator.xor : "^",
    operator.lshift : "<<",
    operator.rshift : ">>",
    operator.and_ : "&",
    operator.or_ : "|",
    operator.not_ : "not",
    operator.neg : "-",
    operator.pos : "+",
    operator.contains : "in",
    operator.gt : ">",
    operator.ge : ">=",
    operator.lt : "<",
    operator.le : "<=",
    operator.eq : "==",
    operator.ne : "!=",
}


class ExprMixin(object):

    def __add__(self, other):
        return BinExpr(operator.add, self, other)
    def __sub__(self, other):
        return BinExpr(operator.sub, self, other)
    def __mul__(self, other):
        return BinExpr(operator.mul, self, other)
    def __floordiv__(self, other):
        return BinExpr(operator.floordiv, self, other)
    def __truediv__(self, other):
        return BinExpr(operator.div, self, other)
    __div__ = __floordiv__
    def __mod__(self, other):
        return BinExpr(operator.mod, self, other)
    def __pow__(self, other):
        return BinExpr(operator.pow, self, other)
    def __xor__(self, other):
        return BinExpr(operator.xor, self, other)
    def __rshift__(self, other):
        return BinExpr(operator.rshift, self, other)
    def __lshift__(self, other):
        return BinExpr(operator.lshift, self, other)
    def __and__(self, other):
        return BinExpr(operator.and_, self, other)
    def __or__(self, other):
        return BinExpr(operator.or_, self, other)

    def __radd__(self, other):
        return BinExpr(operator.add, other, self)
    def __rsub__(self, other):
        return BinExpr(operator.sub, other, self)
    def __rmul__(self, other):
        return BinExpr(operator.mul, other, self)
    def __rfloordiv__(self, other):
        return BinExpr(operator.floordiv, other, self)
    def __rtruediv__(self, other):
        return BinExpr(operator.div, other, self)
    __rdiv__ = __rfloordiv__
    def __rmod__(self, other):
        return BinExpr(operator.mod, other, self)
    def __rpow__(self, other):
        return BinExpr(operator.pow, other, self)
    def __rxor__(self, other):
        return BinExpr(operator.xor, other, self)
    def __rrshift__(self, other):
        return BinExpr(operator.rshift, other, self)
    def __rlshift__(self, other):
        return BinExpr(operator.lshift, other, self)
    def __rand__(self, other):
        return BinExpr(operator.and_, other, self)
    def __ror__(self, other):
        return BinExpr(operator.or_, other, self)

    def __neg__(self):
        return UniExpr(operator.neg, self)
    def __pos__(self):
        return UniExpr(operator.pos, self)
    def __invert__(self):
        return UniExpr(operator.not_, self)
    __inv__ = __invert__

    def __contains__(self, other):
        return BinExpr(operator.contains, self, other)
    def __gt__(self, other):
        return BinExpr(operator.gt, self, other)
    def __ge__(self, other):
        return BinExpr(operator.ge, self, other)
    def __lt__(self, other):
        return BinExpr(operator.lt, self, other)
    def __le__(self, other):
        return BinExpr(operator.le, self, other)
    def __eq__(self, other):
        return BinExpr(operator.eq, self, other)
    def __ne__(self, other):
        return BinExpr(operator.ne, self, other)

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


class UniExpr(ExprMixin):

    def __init__(self, op, operand):
        self.op = op
        self.operand = operand

    def __repr__(self):
        return "%s %r" % (opnames[self.op], self.operand)

    def __str__(self):
        return "%s %s" % (opnames[self.op], self.operand)

    def __call__(self, obj, *args):
        operand = self.operand(obj) if callable(self.operand) else self.operand
        return self.op(operand)


class BinExpr(ExprMixin):

    def __init__(self, op, lhs, rhs):
        self.op = op
        self.lhs = lhs
        self.rhs = rhs

    def __repr__(self):
        return "(%r %s %r)" % (self.lhs, opnames[self.op], self.rhs)

    def __str__(self):
        return "(%s %s %s)" % (self.lhs, opnames[self.op], self.rhs)

    def __call__(self, obj, *args):
        lhs = self.lhs(obj) if callable(self.lhs) else self.lhs
        rhs = self.rhs(obj) if callable(self.rhs) else self.rhs
        return self.op(lhs, rhs)


class Path(ExprMixin):

    def __init__(self, name, field=None, parent=None):
        self.__name = name
        self.__field = field
        self.__parent = parent

    def __repr__(self):
        if self.__parent is None:
            return self.__name
        else:
            return "%s[%r]" % (self.__parent, self.__field)

    def __str__(self):
        if self.__parent is None:
            return self.__name
        else:
            return "%s[%r]" % (self.__parent, self.__field)

    def __call__(self, obj, *args):
        if self.__parent is None:
            return obj
        else:
            return self.__parent(obj)[self.__field]

    def __getfield__(self):
        return self.__field

    def __getattr__(self, name):
        return Path(self.__name, name, self)

    def __getitem__(self, name):
        return Path(self.__name, name, self)


class Path2(ExprMixin):

    def __init__(self, name, index=None, parent=None):
        self.__name = name
        self.__index = index
        self.__parent = parent

    def __repr__(self):
        if self.__parent is None:
            return self.__name
        else:
            return "%r[%r]" % (self.__parent, self.__index)

    def __call__(self, *args):
        if self.__parent is None:
            return args[1]
        else:
            return self.__parent(*args)[self.__index]

    def __getitem__(self, index):
        return Path2(self.__name, index, self)


class FuncPath(ExprMixin):

    def __init__(self, func, operand=None):
        self.__func = func
        self.__operand = operand

    def __repr__(self):
        if self.__operand is None:
            return "%s_" % (self.__func.__name__)
        else:
            return "%s_(%r)" % (self.__func.__name__, self.__operand)

    def __str__(self):
        if self.__operand is None:
            return "%s_" % (self.__func.__name__)
        else:
            return "%s_(%s)" % (self.__func.__name__, self.__operand)

    def __call__(self, operand, *args):
        if self.__operand is None:
            return FuncPath(self.__func, operand) if callable(operand) else operand
        else:
            return self.__func(self.__operand(operand) if callable(self.__operand) else self.__operand)


this = Path("this")
obj_ = Path("obj_")
list_ = Path2("list_")

len_ = FuncPath(len)
sum_ = FuncPath(sum)
min_ = FuncPath(min)
max_ = FuncPath(max)
abs_ = FuncPath(abs)
