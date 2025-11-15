"""ulist is a subclass of list where items cannot appear twice in the list.

[1,2,2,3,3,4] is a valid list, whereas in ulist we can only have [1,2,3,4].

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


class ulist(list):
    """ulist ensures that all items are unique and provides an __onadditem__
    hook to perform custom action in subclasses."""

    #
    # override list methods
    #

    def __init__(self, initlist=None):
        if initlist is not None and initlist != []:
            list.__init__(self, [initlist[0]])
            for item in initlist[1:]:
                if not list.__contains__(self, item):
                    list.append(self, item)
        else:
            list.__init__(self, initlist)

    def __add__(self, other):
        newother = self.__remove_duplicates(other)
        self.__appendother__(newother)
        return self.__class__(list(self) + list(newother))

    def __iadd__(self, other):
        newother = self.__remove_duplicates(other)
        self.__appendother__(newother)
        list.__iadd__(self, newother)
        return self

    def __radd__(self, other):
        newother = self.__remove_duplicates(other)
        return list.__add__(self, newother)

    def append(self, item):
        if not list.__contains__(self, item):
            list.append(self, item)
            self.__onadditem__(item)

    def insert(self, i, item):
        if not list.__contains__(self, item):
            list.insert(self, i, item)
            self.__onadditem__(item)

    def pop(self, i=-1):
        item = list.pop(self, i)
        self.__onremoveitem__(item)
        return item

    def remove(self, item):
        list.remove(self, item)
        self.__onremoveitem__(item)

    #
    # non list methods
    #

    def __remove_duplicates(self, _other):
        """Remove from other items already in list."""
        if (
            not isinstance(_other, type(self))
            and not isinstance(_other, type(list))
            and not isinstance(_other, list)
        ):
            other = [_other]
        else:
            other = list(_other)

        # remove items already in self
        newother = []
        for _ in range(0, len(other)):
            item = other.pop(0)
            if not list.__contains__(self, item):
                newother.append(item)

        # remove duplicate items in other
        other = []
        if newother:
            other.append(newother[0])
            for _ in range(1, len(newother)):
                item = newother.pop()
                if item not in other:
                    other.append(item)
        return other

    def __appendother__(self, other):
        """Append other to object."""
        for item in other:
            self.__onadditem__(item)

    def __onadditem__(self, item):
        """Called for each item added. Override in subclasses for adding
        custom action."""

    def __onremoveitem__(self, item):
        """Called for each item removed. Override in subclasses for
        adding custom action."""
