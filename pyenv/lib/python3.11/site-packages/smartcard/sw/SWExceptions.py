"""Status Word (SW) Exceptions

This module defines the exceptions raised by status word errors or warnings.

__author__ = "https://www.gemalto.com/"

Copyright 2001-2012 gemalto
Author: Jean-Daniel Aussel, mailto:jean-daniel.aussel@gemalto.com

This file is part of pyscard.

pyscard is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

pyscard is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with pyscard; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""


class SWException(Exception):
    """Base class for status word exceptions.

    Status word exceptions are generated when errors and warnings are detected
    in the sw1 and sw2 bytes of the response apdu.

    """

    def __init__(self, data, sw1, sw2, message=""):
        self.message = message
        """response apdu data"""
        self.data = data
        """response apdu sw1"""
        self.sw1 = sw1
        """response apdu sw2"""
        self.sw2 = sw2

    def __str__(self):
        return repr("Status word exception: " + self.message + "!")


class WarningProcessingException(SWException):
    """Raised when a warning processing is detected from sw1, sw2.
    Examples of warning processing exception: sw1=62 or sw=63 (ISO7816-4)."""

    def __init__(self, data, sw1, sw2, message=""):
        SWException.__init__(self, data, sw1, sw2, "warning processing - " + message)


class ExecutionErrorException(SWException):
    """Raised when an execution error is detected from sw1, sw2.
    Examples of execution error: sw1=64 or sw=65 (ISO7816-4)."""

    def __init__(self, data, sw1, sw2, message=""):
        SWException.__init__(self, data, sw1, sw2, "execution error - " + message)


class SecurityRelatedException(SWException):
    """Raised when a security issue is detected from sw1, sw2.
    Examples of security issue: sw1=66 (ISO7816-4)."""

    def __init__(self, data, sw1, sw2, message=""):
        SWException.__init__(self, data, sw1, sw2, "security issue - " + message)


class CheckingErrorException(SWException):
    """Raised when a checking error is detected from sw1, sw2.
    Examples of checking error: sw1=67 to 6F (ISO781604)."""

    def __init__(self, data, sw1, sw2, message=""):
        SWException.__init__(self, data, sw1, sw2, "checking error - " + message)
