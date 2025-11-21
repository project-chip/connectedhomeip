"""Smart card Reader abstract class.

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


class Reader:
    """Reader abstract class.

    The reader class is responsible for creating connections
    with a card.
    """

    def __init__(self, readername):
        """Constructs a new reader and store readername."""
        self.name = readername

    def addtoreadergroup(self, groupname):
        """Add reader to a reader group."""

    def removefromreadergroup(self, groupname):
        """Remove reader from a reader group."""

    def createConnection(self):
        """Returns a card connection thru reader."""

    def __eq__(self, other):
        """Returns True if self==other (same name)."""
        if isinstance(other, type(self)):
            return self.name == other.name

        return False

    def __hash__(self):
        """Returns a hash value for this object (self.name is unique)."""
        return hash(self.name)

    def __repr__(self):
        """Returns card reader name string for `object` calls."""
        return f"'{self.name}'"

    def __str__(self):
        """Returns card reader name string for str(object) calls."""
        return self.name
