"""The error checking chain is a list of status word
(sw1, sw2) error check strategies.

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

from sys import exc_info


class ErrorCheckingChain:
    """The error checking chain is a list of response apdu status word
    (sw1, sw2) error check strategies. Each strategy in the chain is
    called until an error is detected. A L{smartcard.sw.SWExceptions}
    exception is raised when an error is detected. No exception is
    raised if no error is detected.

    Implementation derived from Bruce Eckel, Thinking in Python. The
    L{ErrorCheckingChain} implements the Chain Of Responsibility design
    pattern.
    """

    def __init__(self, chain, strategy):
        """constructor. Appends a strategy to the L{ErrorCheckingChain}
        chain."""
        self.strategy = strategy
        self.chain = chain
        self.chain.append(self)
        self.excludes = []

    def next(self):
        """Returns next error checking strategy."""
        # Where this link is in the chain:
        location = self.chain.index(self)
        if not self.end():
            return self.chain[location + 1]
        return None

    def addFilterException(self, exClass):
        """Add an exception filter to the error checking chain.

        @param exClass:    the exception to exclude, e.g.
        L{smartcard.sw.SWExceptions.WarningProcessingException} A filtered
        exception will not be raised when the sw1,sw2 conditions that
        would raise the exception are met.
        """

        self.excludes.append(exClass)
        if self.end():
            return
        self.next().addFilterException(exClass)

    def end(self):
        """Returns True if this is the end of the error checking
        strategy chain."""
        return self.chain.index(self) + 1 >= len(self.chain)

    def __call__(self, data, sw1, sw2):
        """Called to test data, sw1 and sw2 for error on the chain."""
        try:
            self.strategy(data, sw1, sw2)
        except tuple(self.excludes):
            # The following additional filter may look redundant, it isn't.
            # It checks that type(exc) is *equal* to any of self.excludes,
            # rather than equal-or-subclass to any of self.excludes.
            # This maintains backward compatibility with the behaviour of
            # pyscard <= 1.6.16.
            # if exception is filtered, return
            for exception in self.excludes:
                if exception == exc_info()[0]:
                    return None
            # otherwise reraise exception
            raise

        # if not done, call next strategy
        if self.end():
            return None
        return self.next()(data, sw1, sw2)
