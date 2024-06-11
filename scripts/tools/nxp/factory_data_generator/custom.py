#!/usr/bin/env python3
#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

'''This file should contain custom classes derived any class from default.py.

Each class implemented here should describe an input parameter and should
implement the InputArgument abstract interface, if its base class does not
already offer an implementation or if there is a need of a custom behavior.

Example of defining a new argument class:

    class FooArgument(IntArgument):
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

Then use this class in generate.py to create a FooArgument object from an
option:

    parser.add_argument("--foo", required=True, type=FooArgument,
                        help="[int | hex] Foo argument.")
'''

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.serialization import load_der_private_key
from default import Base64Argument, FileArgument, IntArgument, StrArgument


class Verifier(Base64Argument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 1


class Salt(Base64Argument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 2


class IterationCount(IntArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 3


class DacPKey(FileArgument):

    def __init__(self, arg):
        super().__init__(arg)
        self.private_key = None

    def key(self):
        return 4

    def length(self):
        assert (self.private_key is not None)
        return len(self.private_key)

    def encode(self):
        assert (self.private_key is not None)
        return self.private_key

    def generate_private_key(self, password, use_sss_blob=False):
        if use_sss_blob:
            self.private_key = self.val
        else:
            keys = load_der_private_key(self.val, password, backend=default_backend())
            self.private_key = keys.private_numbers().private_value.to_bytes(
                32, byteorder='big'
            )


class DacCert(FileArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 5


class PaiCert(FileArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 6


class Discriminator(IntArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 7


class SetupPasscode(IntArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 8


class VendorId(IntArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 9

    def length(self):
        return 2


class ProductId(IntArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 10

    def length(self):
        return 2


class CertDeclaration(FileArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 11


class VendorName(StrArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 12


class ProductName(StrArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 13


class SerialNum(StrArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 14


class ManufacturingDate(StrArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 15

    def max_length(self):
        return 16


class HardwareVersion(IntArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 16

    def length(self):
        return 2


class HardwareVersionStr(StrArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 17

    def max_length(self):
        return 64


class UniqueId(StrArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 18


class PartNumber(StrArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 19


class ProductURL(StrArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 20

    def max_length(self):
        return 256


class ProductLabel(StrArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 21

    def max_length(self):
        return 64


class ProductFinish(StrArgument):

    VALUES = ["Other", "Matte", "Satin", "Polished", "Rugged", "Fabric"]

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 22

    def length(self):
        return 1

    def encode(self):
        val = ""
        try:
            val = ProductFinish.VALUES.index(self.val)
        except Exception:
            print(f"Error: {self.val} not in {ProductFinish.VALUES}")
            exit()

        return val.to_bytes(self.length(), "little")

    def max_length(self):
        return 64


class ProductPrimaryColor(StrArgument):

    VALUES = [
        "Black", "Navy", "Green", "Teal", "Maroon",
        "Purple", "Olive", "Gray", "Blue", "Lime",
        "Aqua", "Red", "Fuchsia", "Yellow", "White",
        "Nickel", "Chrome", "Brass", "Copper", "Silver", "Gold"
    ]

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 23

    def length(self):
        return 1

    def encode(self):
        val = ""
        try:
            val = ProductPrimaryColor.VALUES.index(self.val)
        except Exception:
            print(f"Error: {self.val} not in {ProductPrimaryColor.VALUES}")
            exit()

        return val.to_bytes(self.length(), "little")

    def max_length(self):
        return 64
