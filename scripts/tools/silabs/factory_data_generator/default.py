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
'''This file should contain default argument classes.

Base class is InputArgument. It defines the abstract interface to be
implemented and offers a way to compute a KLV value through output().
Other classes that derive InputArgument directly will be referenced
as default classes throughout the docstrings.

The default classes should not be used to instantiate arguments.
If one wants to add another argument, a custom class should be derived
from one of the default classes.
'''

import base64
import logging


class InputArgument:
    '''Base class for any input argument that will be added to KLV.

    The user will define its arguments as instances of InputArgument
    by setting the "type" attribute of ArgumentParser add_argument to
    an instance of a derived class. This means that all derived classes
    must accept an additional "arg" parameter in the constructor. In the
    end, the list of arguments will be parsed into objects derived from
    InputArgument (or default derived classes), which decouples the object
    creation from its processing.

    Abstract methods:
        key: Should be overwritten by final classes to return a "magic number".
        length: Can be overwritten by default classes to specify a default value
                (e.g. int arguments with a default length value of 4); can also
                be overwritten by final classes to specify a custom value for a
                certain argument.
        encode: Should be overwritten to generate the correct bytes array from
                its internal value.

    Main usage is to iterate over an iterable entity of InputArguments and call
    the output() method to generate the (K, L, V) tuple. Note that the output()
    method should not be implemented, since its a common functionality across
    all InputArgument classes.
    '''

    def __init__(self):
        self.val = None

    def key(self):
        logging.error("key() should be implemented in derived classes.")

    def length(self):
        logging.error("length() should be implemented in derived classes.")

    def encode(self):
        logging.error("encode() should be implemented in derived classes.")

    def output(self):
        out = (self.key(), self.length(), self.encode())
        logging.info("'{}' length: {}".format(type(self).__name__, self.length()))
        return out


class IntArgument(InputArgument):

    def __init__(self, arg):
        super().__init__()
        self.val = int(arg, 0)

    def length(self):
        return 4

    def encode(self):
        return self.val.to_bytes(self.length(), "little")


class Base64Argument(InputArgument):

    def __init__(self, arg):
        super().__init__()
        self.val = base64.b64decode(arg)

    def length(self):
        return len(self.encode())

    def encode(self):
        return base64.b64encode(self.val)


class StrArgument(InputArgument):

    def __init__(self, arg):
        super().__init__()
        self.val = str(arg)

    def length(self):
        return len(self.encode())

    def encode(self):
        return str.encode(self.val)

    def max_length(self):
        return 32


class FileArgument(InputArgument):

    def __init__(self, arg):
        super().__init__()
        with open(arg, "rb") as _file:
            self.val = _file.read()

    def length(self):
        return len(self.val)

    def encode(self):
        return self.val
