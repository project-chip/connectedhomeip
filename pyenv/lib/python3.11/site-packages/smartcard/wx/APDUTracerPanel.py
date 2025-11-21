"""Graphical APDU Tracer.

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

# wxPython GUI modules (https://www.wxpython.org/)
import wx

from smartcard.CardConnectionObserver import CardConnectionObserver
from smartcard.util import toHexString

[
    wxID_APDUTEXTCTRL,
] = [wx.NewId() for x in range(1)]


class APDUTracerPanel(wx.Panel, CardConnectionObserver):

    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)

        boxsizer = wx.BoxSizer(wx.HORIZONTAL)
        self.apdutextctrl = wx.TextCtrl(
            self,
            wxID_APDUTEXTCTRL,
            "",
            pos=wx.DefaultPosition,
            style=wx.TE_MULTILINE | wx.TE_READONLY,
        )
        boxsizer.Add(self.apdutextctrl, 1, wx.EXPAND | wx.ALL, 5)
        self.SetSizer(boxsizer)
        self.SetAutoLayout(True)

        self.Bind(wx.EVT_TEXT_MAXLEN, self.OnMaxLength, self.apdutextctrl)

    def OnMaxLength(self, evt):
        """Reset text buffer when max length is reached."""
        self.apdutextctrl.SetValue("")
        evt.Skip()

    def update(self, cardconnection, ccevent):
        """CardConnectionObserver callback."""

        apduline = ""
        if "connect" == ccevent.type:
            apduline += "connecting to " + cardconnection.getReader()

        elif "disconnect" == ccevent.type:
            apduline += "disconnecting from " + cardconnection.getReader()

        elif "command" == ccevent.type:
            apduline += "> " + toHexString(ccevent.args[0])

        elif "response" == ccevent.type:
            if [] == ccevent.args[0]:
                apduline += "< %-2X %-2X" % tuple(ccevent.args[-2:])
            else:
                apduline += (
                    "< "
                    + toHexString(ccevent.args[0])
                    + "%-2X %-2X" % tuple(ccevent.args[-2:])
                )

        self.apdutextctrl.AppendText(apduline + "\n")
