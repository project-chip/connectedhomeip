"""wxpython smartcard utility module.

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

import os.path
import sys


def main_is_frozen():
    return hasattr(sys, "frozen") or hasattr(sys, "importers")


ICO_SMARTCARD = None
ICO_READER = None

# running from a script, i.e. not running from standalone exe built with py2exe
if not main_is_frozen():
    ICO_SMARTCARD = os.path.join(
        os.path.dirname(__file__), "resources", "smartcard.ico"
    )
    ICO_READER = os.path.join(os.path.dirname(__file__), "resources", "reader.ico")

# running from a standalone exe built with py2exe
# resources expected images directory
else:
    if os.path.exists(os.path.join("images", "smartcard.ico")):
        ICO_SMARTCARD = os.path.join("images", "smartcard.ico")
    if os.path.exists(os.path.join("images", "reader.ico")):
        ICO_READER = os.path.join("images", "reader.ico")
