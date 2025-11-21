"""
A wxValidator that matches APDU in hexadecimal such as::

    A4 A0 00 00 02
    A4A0000002

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

import re
import string

import wx

# a regexp to match ATRs and APDUs
hexbyte = "[0-9a-fA-F]{1,2}"
apduregexp = re.compile("((%s)[ ]*)*" % hexbyte)


class APDUHexValidator(wx.PyValidator):
    """A wxValidator that matches APDU in hexadecimal such as:
    A4 A0 00 00 02
    A4A0000002"""

    def __init__(self):
        wx.Validator.__init__(self)
        self.Bind(wx.EVT_CHAR, self.OnChar)

    def Clone(self):
        return APDUHexValidator()

    def Validate(self, win):
        tc = self.GetWindow()
        value = tc.GetValue()

        if not apduregexp.match(value):
            return False

        return True

    def OnChar(self, event):
        key = event.GetKeyCode()

        if wx.WXK_SPACE == key or chr(key) in string.hexdigits:
            value = event.GetEventObject().GetValue() + chr(key)
            if apduregexp.match(value):
                event.Skip()
            return

        if key < wx.WXK_SPACE or key == wx.WXK_DELETE or key > 255:
            event.Skip()
            return

        if not wx.Validator.IsSilent():
            wx.Bell()

        return
