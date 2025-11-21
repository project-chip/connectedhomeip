"""ReaderGroups manages smart card reader in groups.

__author__ = "gemalto https://www.gemalto.com/"

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

from smartcard.Exceptions import SmartcardException
from smartcard.ulist import ulist

# pylint: disable=too-few-public-methods


class BadReaderGroupException(SmartcardException):
    """Raised when trying to add an invalid reader group."""

    def __init__(self):
        SmartcardException.__init__(self, "Invalid reader group")


class innerreadergroups(ulist):
    """Smartcard readers groups private class.

    The readergroups singleton manages the creation of the unique
    instance of this class.
    """

    def __init__(self, initlist=None):
        """Retrieve and store list of reader groups"""
        if initlist is None:
            initlist = self.getreadergroups() or []
        ulist.__init__(self, initlist)
        self.unremovablegroups = []

    def __onadditem__(self, item):
        """Called when a reader group is added."""
        self.addreadergroup(item)

    def __onremoveitem__(self, item):
        """Called when a reader group is added."""
        self.removereadergroup(item)

    def __iter__(self):
        return ulist.__iter__(self)

    #
    # abstract methods implemented in subclasses
    #

    def getreadergroups(self):
        """Returns the list of smartcard reader groups."""
        return []

    def addreadergroup(self, newgroup):
        """Add a reader group"""
        if not isinstance(newgroup, str):
            raise BadReaderGroupException
        self += newgroup

    def removereadergroup(self, group):
        """Remove a reader group"""
        if not isinstance(group, str):
            raise BadReaderGroupException
        self.remove(group)

    def addreadertogroup(self, readername, groupname):
        """Add a reader to a reader group"""

    def removereaderfromgroup(self, readername, groupname):
        """Remove a reader from a reader group"""


class readergroups:
    """ReadersGroups organizes smart card reader as groups."""

    # The single instance of __readergroups
    instance = None
    innerclazz = innerreadergroups

    def __init__(self, initlist=None):
        """Create a single instance of innerreadergroups on first call"""
        if readergroups.instance is None:
            readergroups.instance = self.innerclazz(initlist)

    # All operators redirected to inner class.

    def __getattr__(self, name):
        return getattr(self.instance, name)
