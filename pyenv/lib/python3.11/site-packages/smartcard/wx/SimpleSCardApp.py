"""Simple wxPython wxApp for smartcard.

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

import wx

from smartcard.wx.SimpleSCardAppFrame import SimpleSCardAppFrame

TR_SMARTCARD = 0x001
TR_READER = 0x002
TB_SMARTCARD = 0x004
TB_READER = 0x008
PANEL_APDUTRACER = 0x010
TR_DEFAULT = TR_SMARTCARD


class SimpleSCardApp(wx.App):
    """The SimpleSCardApp class represents the smart card application.
    SimpleSCardApp is a subclass of wx.App.
    """

    def __init__(
        self,
        appname="",
        apppanel=None,
        appstyle=TR_DEFAULT,
        appicon=None,
        pos=(-1, -1),
        size=(-1, -1),
    ):
        r"""Constructor for simple smart card application.
        @param appname: the application name
        @param apppanel: the application panel to display in the application frame
        @param appicon: the application icon file; the default is no icon
        @param appstyle: a combination of the following styles (bitwise or |)
          - TR_SMARTCARD: display a smartcard tree panel
          - TR_READER: display a reader tree panel
          - TB_SMARTCARD: display a smartcard toolbar
          - TB_SMARTCARD: display a reader toolbar
          - PANEL_APDUTRACER: display an APDU tracer panel
          - default is TR_DEFAULT = TR_SMARTCARD
        @param pos: the application position as a (x,y) tuple; default is (-1,-1)
        @param size: the application window size as a (x,y) tuple; default is (-1,-1)

            Example:
            C{app = SimpleSCardApp(
            appname = 'A simple smartcard application',
            apppanel = testpanel.MyPanel,
            appstyle = TR_READER | TR_SMARTCARD,
            appicon = 'resources\mysmartcard.ico')}
        """
        self.appname = appname
        self.apppanel = apppanel
        self.appstyle = appstyle
        self.appicon = appicon
        self.pos = pos
        self.size = size
        wx.App.__init__(self, False)

    def OnInit(self):
        """Create and display application frame."""
        self.frame = SimpleSCardAppFrame(
            self.appname,
            self.apppanel,
            self.appstyle,
            self.appicon,
            self.pos,
            self.size,
        )
        self.frame.Show(True)
        self.SetTopWindow(self.frame)

        return True
