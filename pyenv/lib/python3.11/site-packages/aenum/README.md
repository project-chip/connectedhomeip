aenum --- support for advanced enumerations, namedtuples, and constants
=======================================================================

Advanced Enumerations (compatible with Python's stdlib Enum), NamedTuples,
and NamedConstants

aenum includes a Python stdlib Enum-compatible data type, as well as a
metaclass-based NamedTuple implementation and a NamedConstant class.

An Enum is a set of symbolic names (members) bound to unique, constant
values. Within an enumeration, the members can be compared by identity, and
the enumeration itself can be iterated over.  If using Python 3 there is
built-in support for unique values, multiple values, auto-numbering, and
suspension of aliasing (members with the same value are not identical), plus
the ability to have values automatically bound to attributes.

A NamedTuple is a class-based, fixed-length tuple with a name for each
possible position accessible using attribute-access notation as well as the
standard index notation.

A NamedConstant is a class whose members cannot be rebound; it lacks all other
Enum capabilities, however; consequently, it can have duplicate values.


Module Contents
===============

NamedTuple
------------

Base class for `creating NamedTuples`, either by subclassing or via it's
functional API.

Constant
--------

Constant class for creating groups of constants.  These names cannot be rebound
to other values.

Enum
----

Base class for creating enumerated constants.  See section `Enum Functional API`
for an alternate construction syntax.

IntEnum
-------

Base class for creating enumerated constants that are also subclasses of `int`.

StrEnum
-------

Base class for creating enumerated constants that are also subclasses of `str`.

AutoNumberEnum
--------------

Derived class that automatically assigns an `int` value to each member.

OrderedEnum
-----------

Derived class that adds `<`, `<=`, `>=`, and `>` methods to an `Enum`.

UniqueEnum
----------

Derived class that ensures only one name is bound to any one value.

Flag
----

Base class for creating enumerated constants that can be combined using
the bitwise operations without losing their `Flag` membership.

IntFlag
-------

Base class for creating enumerated constants that can be combined using
the bitwise operators without losing their `IntFlag` membership.
`IntFlag` members are also subclasses of `int`.
   
unique
------

Enum class decorator that ensures only one name is bound to any one value.

constant
--------

Descriptor to add constant values to an `Enum`

convert
-------

Helper to transform target global variables into an `Enum`.

enum
----

Helper for specifying keyword arguments when creating `Enum` members.

export
------

Helper for inserting `Enum` members into a namespace (usually `globals()`).

extend_enum
-----------

Helper for adding new `Enum` members after creation.

module
------

Function to take a `Constant` or `Enum` class and insert it into
`sys.modules` with the effect of a module whose top-level constant and
member names cannot be rebound.

member
------

Decorator to force a member in an `Enum` or `Constant`.

nonmember
---------

Decorator to force a normal (non-`Enum` member) attribute in an `Enum`
or `Constant`.


Creating an Enum
================

Enumerations can be created using the `class` syntax, which makes them
easy to read and write.  To define an enumeration, subclass `Enum` as
follows:

    >>> from aenum import Enum
    >>> class Color(Enum):
    ...     RED = 1
    ...     GREEN = 2
    ...     BLUE = 3

The `Enum` class is also callable, providing the following functional API:

    >>> Animal = Enum('Animal', 'ANT BEE CAT DOG')
    >>> Animal
    <enum 'Animal'>
    >>> Animal.ANT
    <Animal.ANT: 1>
    >>> Animal.ANT.value
    1
    >>> list(Animal)
    [<Animal.ANT: 1>, <Animal.BEE: 2>, <Animal.CAT: 3>, <Animal.DOG: 4>]

Note that `Enum` members are boolean `True` unless the `__nonzero__`
(Python 2) or `__bool__` (Python 3) method is overridden to provide
different semantics.


Creating a Flag
===============

`Flag` (and `IntFlag`) has members that can be combined with each other
using the bitwise operators (&, \|, ^, ~).  `IntFlag` members can be combined
with `int` and other `IntFlag` members.  While it is possible to specify
the values directly it is recommended to use `auto` as the value and let
`(Int)Flag` select an appropriate value:

    >>> from enum import Flag
    >>> class Color(Flag):
    ...     RED = auto()
    ...     BLUE = auto()
    ...     GREEN = auto()
    ...
    >>> Color.RED & Color.GREEN
    <Color.0: 0>
    >>> bool(Color.RED & Color.GREEN)
    False
    >>> Color.RED | Color.BLUE
    <Color.RED|BLUE: 3>

If you want to name the empty flag, or various combinations of flags, you may:

    >>> class Color(Flag):
    ...     BLACK = 0
    ...     RED = auto()
    ...     BLUE = auto()
    ...     GREEN = auto()
    ...     WHITE = RED | BLUE | GREEN
    ...
    >>> Color.BLACK
    <Color.BLACK: 0>
    >>> Color.WHITE
    <Color.WHITE: 7>

Note that `(Int)Flag` zero-value members have the usual boolean value of
`False`.


Creating NamedTuples
====================

Simple
------

The most common way to create a new NamedTuple will be via the functional API:

    >>> from aenum import NamedTuple
    >>> Book = NamedTuple('Book', 'title author genre', module=__name__)

Advanced
--------

The simple method of creating `NamedTuples` requires always specifying all
possible arguments when creating instances; failure to do so will raise
exceptions.

However, it is possible to specify both docstrings and default values when
creating a `NamedTuple` using the class method:

    >>> class Point(NamedTuple):
    ...     x = 0, 'horizontal coordinate', 0
    ...     y = 1, 'vertical coordinate', 0
    ...
    >>> Point()
    Point(x=0, y=0)


Creating Constants
==================

`Constant` is similar to `Enum`, but does not support the `Enum`
protocols, and have no restrictions on duplications:

    >>> class K(Constant):
    ...     PI = 3.141596
    ...     TAU = 2 * PI
    ...
    >>> K.TAU
    6.283192

More Information
================

Detailed documentation can be found at `<aenum/doc/aenum.rst>`_
