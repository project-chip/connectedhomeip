"""wxPython panel display cards/readers as a TreeCtrl.

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

# smartcard imports
from threading import RLock

# wxPython GUI modules (https://www.wxpython.org/)
import wx

import smartcard.wx.SimpleSCardApp
from smartcard.CardMonitoring import CardMonitor, CardObserver
from smartcard.Exceptions import CardConnectionException, NoCardException
from smartcard.ReaderMonitoring import ReaderMonitor, ReaderObserver
from smartcard.util import toHexString
from smartcard.wx import ICO_READER, ICO_SMARTCARD


class BaseCardTreeCtrl(wx.TreeCtrl):
    """Base class for the smart card and reader tree controls."""

    def __init__(
        self,
        parent,
        ID=wx.NewId(),
        pos=wx.DefaultPosition,
        size=wx.DefaultSize,
        style=0,
        clientpanel=None,
    ):
        """Constructor. Initializes a smartcard or reader tree control."""
        wx.TreeCtrl.__init__(
            self, parent, ID, pos, size, wx.TR_SINGLE | wx.TR_NO_BUTTONS
        )

        self.clientpanel = clientpanel
        self.parent = parent

        isz = (16, 16)
        il = wx.ImageList(isz[0], isz[1])
        self.capindex = il.Add(
            wx.ArtProvider.GetBitmap(wx.ART_HELP_BOOK, wx.ART_OTHER, isz)
        )
        self.fldrindex = il.Add(
            wx.ArtProvider.GetBitmap(wx.ART_FOLDER, wx.ART_OTHER, isz)
        )
        self.fldropenindex = il.Add(
            wx.ArtProvider.GetBitmap(wx.ART_FILE_OPEN, wx.ART_OTHER, isz)
        )
        if None != ICO_SMARTCARD:
            self.cardimageindex = il.Add(wx.Bitmap(ICO_SMARTCARD, wx.BITMAP_TYPE_ICO))
        if None != ICO_READER:
            self.readerimageindex = il.Add(wx.Bitmap(ICO_READER, wx.BITMAP_TYPE_ICO))
        self.il = il
        self.SetImageList(self.il)

    def Repaint(self):
        self.Refresh()


class CardTreeCtrl(BaseCardTreeCtrl):
    """The CardTreeCtrl monitors inserted cards and notifies the
    application client dialog of any card activation."""

    def __init__(
        self,
        parent,
        ID=wx.NewId(),
        pos=wx.DefaultPosition,
        size=wx.DefaultSize,
        style=0,
        clientpanel=None,
    ):
        """Constructor. Create a smartcard tree control."""
        BaseCardTreeCtrl.__init__(
            self, parent, ID, pos, size, wx.TR_SINGLE | wx.TR_NO_BUTTONS, clientpanel
        )

        self.root = self.AddRoot("Smartcards")
        self.SetItemData(self.root, None)
        self.SetItemImage(self.root, self.fldrindex, wx.TreeItemIcon_Normal)
        self.SetItemImage(self.root, self.fldropenindex, wx.TreeItemIcon_Expanded)
        self.Expand(self.root)

    def OnAddCards(self, addedcards):
        """Called when a card is inserted.
        Adds a smart card to the smartcards tree."""
        parentnode = self.root
        for cardtoadd in addedcards:
            childCard = self.AppendItem(parentnode, toHexString(cardtoadd.atr))
            self.SetItemText(childCard, toHexString(cardtoadd.atr))
            self.SetItemData(childCard, cardtoadd)
            self.SetItemImage(childCard, self.cardimageindex, wx.TreeItemIcon_Normal)
            self.SetItemImage(childCard, self.cardimageindex, wx.TreeItemIcon_Expanded)
            self.Expand(childCard)
        self.Expand(self.root)
        self.EnsureVisible(self.root)
        self.Repaint()

    def OnRemoveCards(self, removedcards):
        """Called when a card is removed.
        Removes a card from the tree."""
        parentnode = self.root
        for cardtoremove in removedcards:
            (childCard, cookie) = self.GetFirstChild(parentnode)
            while childCard.IsOk():
                if self.GetItemText(childCard) == toHexString(cardtoremove.atr):
                    self.Delete(childCard)
                (childCard, cookie) = self.GetNextChild(parentnode, cookie)
        self.Expand(self.root)
        self.EnsureVisible(self.root)
        self.Repaint()


class ReaderTreeCtrl(BaseCardTreeCtrl):
    """The ReaderTreeCtrl monitors inserted cards and readers and notifies the
    application client dialog of any card activation."""

    def __init__(
        self,
        parent,
        ID=wx.NewId(),
        pos=wx.DefaultPosition,
        size=wx.DefaultSize,
        style=0,
        clientpanel=None,
    ):
        """Constructor. Create a reader tree control."""

        BaseCardTreeCtrl.__init__(
            self, parent, ID, pos, size, wx.TR_SINGLE | wx.TR_NO_BUTTONS, clientpanel
        )

        self.mutex = RLock()

        self.root = self.AddRoot("Smartcard Readers")
        self.SetItemData(self.root, None)
        self.SetItemImage(self.root, self.fldrindex, wx.TreeItemIcon_Normal)
        self.SetItemImage(self.root, self.fldropenindex, wx.TreeItemIcon_Expanded)
        self.Expand(self.root)

    def AddATR(self, readernode, atr):
        """Add an ATR to a reader node."""
        capchild = self.AppendItem(readernode, atr)
        self.SetItemData(capchild, None)
        self.SetItemImage(capchild, self.cardimageindex, wx.TreeItemIcon_Normal)
        self.SetItemImage(capchild, self.cardimageindex, wx.TreeItemIcon_Expanded)
        self.Expand(capchild)
        return capchild

    def GetATR(self, reader):
        """Return the ATR of the card inserted into the reader."""
        atr = "no card inserted"
        try:
            if not type(reader) is str:
                connection = reader.createConnection()
                connection.connect()
                atr = toHexString(connection.getATR())
                connection.disconnect()
        except NoCardException:
            pass
        except CardConnectionException:
            pass
        return atr

    def OnAddCards(self, addedcards):
        """Called when a card is inserted.
        Adds the smart card child to the reader node."""
        self.mutex.acquire()
        try:
            parentnode = self.root
            for cardtoadd in addedcards:
                (childReader, cookie) = self.GetFirstChild(parentnode)
                found = False
                while childReader.IsOk() and not found:
                    if self.GetItemText(childReader) == str(cardtoadd.reader):
                        (childCard, cookie2) = self.GetFirstChild(childReader)
                        self.SetItemText(childCard, toHexString(cardtoadd.atr))
                        self.SetItemData(childCard, cardtoadd)
                        found = True
                    else:
                        (childReader, cookie) = self.GetNextChild(parentnode, cookie)

                # reader was not found, add reader node
                # this happens when card monitoring thread signals
                # added cards before reader monitoring thread signals
                # added readers
                if not found:
                    childReader = self.AppendItem(parentnode, str(cardtoadd.reader))
                    self.SetItemData(childReader, cardtoadd.reader)
                    self.SetItemImage(
                        childReader, self.readerimageindex, wx.TreeItemIcon_Normal
                    )
                    self.SetItemImage(
                        childReader, self.readerimageindex, wx.TreeItemIcon_Expanded
                    )
                    childCard = self.AddATR(childReader, toHexString(cardtoadd.atr))
                    self.SetItemData(childCard, cardtoadd)
                    self.Expand(childReader)

            self.Expand(self.root)
        finally:
            self.mutex.release()
        self.EnsureVisible(self.root)
        self.Repaint()

    def OnAddReaders(self, addedreaders):
        """Called when a reader is inserted.
        Adds the smart card reader to the smartcard readers tree."""
        self.mutex.acquire()

        try:
            parentnode = self.root
            for readertoadd in addedreaders:
                # is the reader already here?
                found = False
                (childReader, cookie) = self.GetFirstChild(parentnode)
                while childReader.IsOk() and not found:
                    if self.GetItemText(childReader) == str(readertoadd):
                        found = True
                    else:
                        (childReader, cookie) = self.GetNextChild(parentnode, cookie)
                if not found:
                    childReader = self.AppendItem(parentnode, str(readertoadd))
                    self.SetItemData(childReader, readertoadd)
                    self.SetItemImage(
                        childReader, self.readerimageindex, wx.TreeItemIcon_Normal
                    )
                    self.SetItemImage(
                        childReader, self.readerimageindex, wx.TreeItemIcon_Expanded
                    )
                    self.AddATR(childReader, self.GetATR(readertoadd))
                    self.Expand(childReader)
            self.Expand(self.root)
        finally:
            self.mutex.release()
        self.EnsureVisible(self.root)
        self.Repaint()

    def OnRemoveCards(self, removedcards):
        """Called when a card is removed.
        Removes the card from the tree."""
        self.mutex.acquire()
        try:
            parentnode = self.root
            for cardtoremove in removedcards:
                (childReader, cookie) = self.GetFirstChild(parentnode)
                found = False
                while childReader.IsOk() and not found:
                    if self.GetItemText(childReader) == str(cardtoremove.reader):
                        (childCard, cookie2) = self.GetFirstChild(childReader)
                        self.SetItemText(childCard, "no card inserted")
                        found = True
                    else:
                        (childReader, cookie) = self.GetNextChild(parentnode, cookie)
            self.Expand(self.root)
        finally:
            self.mutex.release()
        self.EnsureVisible(self.root)
        self.Repaint()

    def OnRemoveReaders(self, removedreaders):
        """Called when a reader is removed.
        Removes the reader from the smartcard readers tree."""
        self.mutex.acquire()
        try:
            parentnode = self.root
            for readertoremove in removedreaders:
                (childReader, cookie) = self.GetFirstChild(parentnode)
                while childReader.IsOk():
                    if self.GetItemText(childReader) == str(readertoremove):
                        self.Delete(childReader)
                    else:
                        (childReader, cookie) = self.GetNextChild(parentnode, cookie)
            self.Expand(self.root)
        finally:
            self.mutex.release()
        self.EnsureVisible(self.root)
        self.Repaint()


class CardAndReaderTreePanel(wx.Panel):
    """Panel containing the smart card and reader tree controls."""

    class _CardObserver(CardObserver):
        """Inner CardObserver. Gets notified of card insertion
        removal by the CardMonitor."""

        def __init__(self, cardtreectrl):
            self.cardtreectrl = cardtreectrl

        def update(self, observable, handlers):
            """CardObserver callback that is notified
            when cards are added or removed."""
            addedcards, removedcards = handlers
            self.cardtreectrl.OnRemoveCards(removedcards)
            self.cardtreectrl.OnAddCards(addedcards)

    class _ReaderObserver(ReaderObserver):
        """Inner ReaderObserver. Gets notified of reader insertion/removal
        by the ReaderMonitor."""

        def __init__(self, readertreectrl):
            self.readertreectrl = readertreectrl

        def update(self, observable, handlers):
            """ReaderObserver callback that is notified when
            readers are added or removed."""
            addedreaders, removedreaders = handlers
            self.readertreectrl.OnRemoveReaders(removedreaders)
            self.readertreectrl.OnAddReaders(addedreaders)

    def __init__(self, parent, appstyle, clientpanel):
        """Constructor. Create a smartcard and reader tree control on the
        left-hand side of the application main frame.
        @param parent: the tree panel parent
        @param appstyle: a combination of the following styles (bitwise or |)
          - TR_SMARTCARD: display a smartcard tree panel
          - TR_READER: display a reader tree panel
          - default is TR_DEFAULT = TR_SMARTCARD
        @param clientpanel: the client panel to notify of smartcard and reader events
        """
        wx.Panel.__init__(self, parent, -1, style=wx.WANTS_CHARS)

        sizer = wx.BoxSizer(wx.VERTICAL)

        # create the smartcard tree
        if appstyle & smartcard.wx.SimpleSCardApp.TR_SMARTCARD:
            self.cardtreectrl = CardTreeCtrl(self, clientpanel=clientpanel)

            # create the smartcard insertion observer
            self.cardtreecardobserver = self._CardObserver(self.cardtreectrl)

            # register as a CardObserver; we will ge
            # notified of added/removed cards
            self.cardmonitor = CardMonitor()
            self.cardmonitor.addObserver(self.cardtreecardobserver)

            sizer.Add(self.cardtreectrl, flag=wx.EXPAND | wx.ALL, proportion=1)

        # create the reader tree
        if appstyle & smartcard.wx.SimpleSCardApp.TR_READER:
            self.readertreectrl = ReaderTreeCtrl(self, clientpanel=clientpanel)

            # create the reader insertion observer
            self.readertreereaderobserver = self._ReaderObserver(self.readertreectrl)

            # register as a ReaderObserver; we will ge
            # notified of added/removed readers
            self.readermonitor = ReaderMonitor()
            self.readermonitor.addObserver(self.readertreereaderobserver)

            # create the smartcard insertion observer
            self.readertreecardobserver = self._CardObserver(self.readertreectrl)

            # register as a CardObserver; we will get
            # notified of added/removed cards
            self.cardmonitor = CardMonitor()
            self.cardmonitor.addObserver(self.readertreecardobserver)

            sizer.Add(self.readertreectrl, flag=wx.EXPAND | wx.ALL, proportion=1)

        self.SetSizer(sizer)
        self.SetAutoLayout(True)

    def OnDestroy(self, event):
        """Called on panel destruction."""
        # deregister observers
        if hasattr(self, "cardmonitor"):
            self.cardmonitor.deleteObserver(self.cardtreecardobserver)
        if hasattr(self, "readermonitor"):
            self.readermonitor.deleteObserver(self.readertreereaderobserver)
            self.cardmonitor.deleteObserver(self.readertreecardobserver)
        event.Skip()
