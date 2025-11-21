"""wxPython toolbar with reader icons implementing ReaderObserver.

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

from smartcard.ReaderMonitoring import ReaderMonitor, ReaderObserver
from smartcard.wx import ICO_READER, ICO_SMARTCARD


class ReaderComboBox(wx.ComboBox, ReaderObserver):

    def __init__(self, parent):
        """Constructor. Registers as ReaderObserver to get
        notifications of reader insertion/removal."""
        wx.ComboBox.__init__(
            self,
            parent,
            wx.NewId(),
            size=(170, -1),
            style=wx.CB_DROPDOWN | wx.CB_SORT,
            choices=[],
        )

        # register as a ReaderObserver; we will get
        #  notified of added/removed readers
        self.readermonitor = ReaderMonitor()
        self.readermonitor.addObserver(self)

    def update(self, observable, handlers):
        """Toolbar ReaderObserver callback that is notified when
        readers are added or removed."""
        addedreaders, removedreaders = handlers
        for reader in addedreaders:
            item = self.Append(str(reader))
            self.SetClientData(item, reader)
        for reader in removedreaders:
            item = self.FindString(str(reader))
            if wx.NOT_FOUND != item:
                self.Delete(item)
        selection = self.GetSelection()
        # if wx.NOT_FOUND == selection:
        #    self.SetSelection(0)


class ReaderToolbar(wx.ToolBar):
    """ReaderToolbar. Contains controls to select a reader from a listbox
    and connect to the cards."""

    def __init__(self, parent):
        """Constructor, creating the reader toolbar."""
        wx.ToolBar.__init__(
            self,
            parent,
            pos=wx.DefaultPosition,
            size=wx.DefaultSize,
            style=wx.SIMPLE_BORDER | wx.TB_HORIZONTAL | wx.TB_FLAT | wx.TB_TEXT,
            name="Reader Toolbar",
        )

        # create bitmaps for toolbar
        tsize = (16, 16)
        if None != ICO_READER:
            bmpReader = wx.Bitmap(ICO_READER, wx.BITMAP_TYPE_ICO)
        else:
            bmpReader = wx.ArtProvider_GetBitmap(wx.ART_HELP_BOOK, wx.ART_OTHER, tsize)
        if None != ICO_SMARTCARD:
            bmpCard = wx.Bitmap(ICO_SMARTCARD, wx.BITMAP_TYPE_ICO)
        else:
            bmpCard = wx.ArtProvider_GetBitmap(wx.ART_HELP_BOOK, wx.ART_OTHER, tsize)
        self.readercombobox = ReaderComboBox(self)

        # create and add controls
        self.AddSimpleTool(
            10, bmpReader, "Select smart card reader", "Select smart card reader"
        )
        self.AddControl(self.readercombobox)
        self.AddSeparator()
        self.AddSimpleTool(20, bmpCard, "Connect to smartcard", "Connect to smart card")
        self.AddSeparator()

        self.Realize()
