# Silabs Factory Data Generator

## Tool implementation

The tool comprises of two files: `default.py`, `custom.py`

### `default.py`

Defines the base `InputArgument` class and its derived classes that will be
referenced as **default classes**.

`InputArgument` offers an abstract interface in the form of three methods:
`key()`, `length()`, `encode()`, that will be used to generate the `(K, L, V)`
tuple through the public `output()` method. Each custom class should implement
the abstract interface, if its direct parent does not offer a relevant
implementation.

### `custom.py`

Defines classes for each argument that should generate data in the output binary
(will be referenced as **custom classes**). Please note that each new class
should derive from a default class, not from `InputArgument` directly.

### How to add a new argument

Example of defining a new argument class in `custom.py`:

```
class FooArgument(BarArgument):
    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return <unique key identifier>

    def length(self):
        return <actual length of data>

    def encode(self):
        return <data as encoded bytes>

    def custom_function(self):
        pass
```

where `BarArgument` is one of the **default classes**. Please note that a user
can define additional methods if needed (e.g. `custom_function`; also see
`generate_private_key` from `DacPKey` class).

Then use this class in `generate.py` to create a `FooArgument` object from an
option:

```
parser.add_argument("--foo", required=True, type=FooArgument,
                    help="[int | hex] Foo argument.")
```
