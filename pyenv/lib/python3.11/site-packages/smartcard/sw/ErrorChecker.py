"""Base class for status word error checkers.

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

# pylint: disable=too-few-public-methods


class ErrorChecker:
    """Base class for status word error checking strategies.

    Error checking strategies are chained into an L{ErrorCheckingChain} to
    implement a Chain of Responsibility. Each strategy in the chain is
    called until an error is  detected. The strategy raises a
    L{smartcard.sw.SWExceptions} exception when an error is detected.

    Implementation derived from Bruce Eckel, Thinking in Python. The
    L{ErrorCheckingChain} implements the Chain Of Responsibility design
    pattern.
    """

    def __call__(self, data, sw1, sw2):
        """Called to test data, sw1 and sw2 for error.

        Derived classes must raise a L{smartcard.sw.SWExceptions} upon error.

        @param data:       apdu response data
        @param sw1:        apdu data status word 1
        @param sw2:        apdu data status word 2
        """
