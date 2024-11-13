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
from default import FileArgument


class DacPKey(FileArgument):

    def __init__(self, arg):
        super().__init__(arg)
        self.private_key = None

    def key(self):
        return 1

    def length(self):
        assert (self.private_key is not None)
        return len(self.private_key)

    def encode(self):
        assert (self.private_key is not None)
        return self.private_key

    def generate_private_key(self, password, use_sss_blob=True):
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
        return 2


class PaiCert(FileArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 3


class CertDeclaration(FileArgument):

    def __init__(self, arg):
        super().__init__(arg)

    def key(self):
        return 4
