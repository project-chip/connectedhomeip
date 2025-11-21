"""ReaderFactory: creates smartcard readers.

__author__ = "gemalto https://www.gemalto.com/"

Factory pattern implementation borrowed from
Thinking in Python, Bruce Eckel,
http://mindview.net/Books/TIPython

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

import importlib

from smartcard.pcsc.PCSCReader import PCSCReader


class ReaderFactory:
    """Class to create readers from reader type id."""

    factories = {}
    factorymethods = [PCSCReader.readers]

    # A Template Method:
    @staticmethod
    def createReader(clazz: str, readername: str):
        """Static method to create a reader from a reader clazz.

        @param clazz:      the reader class name
        @param readername: the reader name
        """

        if clazz not in ReaderFactory.factories:
            module_name, _, class_name = clazz.rpartition(".")
            imported_module = importlib.import_module(module_name)
            imported_class = getattr(imported_module, class_name)
            ReaderFactory.factories[clazz] = imported_class.Factory()

        return ReaderFactory.factories[clazz].create(readername)

    @staticmethod
    def readers(groups=None):
        """Return the list of readers"""
        if groups is None:
            groups = []
        zreaders = []
        for fm in ReaderFactory.factorymethods:
            zreaders += fm(groups)
        return zreaders
