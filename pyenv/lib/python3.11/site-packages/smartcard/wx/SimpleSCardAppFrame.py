"""Simple wxpython frame for smart card application.

__author__ = "gemalto https://www.gemalto.com/"
__date__ = "November 2006"
__version__ = "1.4.0"

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

import wx

import smartcard
import smartcard.wx
from smartcard.wx import APDUTracerPanel, CardAndReaderTreePanel, ReaderToolbar
from smartcard.wx.SimpleSCardAppEventObserver import SimpleSCardAppEventObserver

[
    wxID_SIMPLESCARDAPP_FRAME,
] = [wx.NewId() for x in range(1)]


class BlankPanel(wx.Panel, SimpleSCardAppEventObserver):
    """A blank panel in case no panel is provided to SimpleSCardApp."""

    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)
        sizer = wx.GridSizer(1, 1, 0)
        self.SetSizer(sizer)
        self.SetAutoLayout(True)


class TreeAndUserPanelPanel(wx.Panel):
    """The panel that contains the Card/Reader TreeCtrl
    and the user provided Panel."""

    def __init__(self, parent, apppanelclass, appstyle):
        """
        Constructor. Creates the panel with two panels:
          - the left-hand panel is holding the smartcard and/or reader tree
          - the right-hand panel is holding the application dialog

        @param apppanelclass: the class of the panel to instantiate in the
                    L{SimpleSCardAppFrame}
        @param appstyle: a combination of the following styles (bitwise or |)
           - TR_SMARTCARD: display a smartcard tree panel
           - TR_READER: display a reader tree panel
           - TB_SMARTCARD: display a smartcard toolbar
           - TB_SMARTCARD: display a reader toolbar
           - default is TR_DEFAULT = TR_SMARTCARD
        """
        wx.Panel.__init__(self, parent, -1)

        self.parent = parent
        self.selectedcard = None

        boxsizer = wx.BoxSizer(wx.HORIZONTAL)

        # create user dialog
        if None != apppanelclass:
            self.dialogpanel = apppanelclass(self)
        else:
            self.dialogpanel = BlankPanel(self)

        # create card/reader tree control
        if (
            appstyle & smartcard.wx.SimpleSCardApp.TR_SMARTCARD
            or appstyle & smartcard.wx.SimpleSCardApp.TR_READER
        ):
            self.readertreepanel = CardAndReaderTreePanel.CardAndReaderTreePanel(
                self, appstyle, self.dialogpanel
            )
            boxsizer.Add(self.readertreepanel, 1, wx.EXPAND | wx.ALL, 5)

        boxsizer.Add(self.dialogpanel, 2, wx.EXPAND | wx.ALL)

        if appstyle & smartcard.wx.SimpleSCardApp.TR_READER:
            self.Bind(
                wx.EVT_TREE_ITEM_ACTIVATED,
                self.OnActivateReader,
                self.readertreepanel.readertreectrl,
            )
            self.Bind(
                wx.EVT_TREE_SEL_CHANGED,
                self.OnSelectReader,
                self.readertreepanel.readertreectrl,
            )
            self.Bind(
                wx.EVT_TREE_ITEM_RIGHT_CLICK,
                self.OnReaderRightClick,
                self.readertreepanel.readertreectrl,
            )
            self.Bind(
                wx.EVT_TREE_ITEM_COLLAPSED,
                self.OnItemCollapsed,
                self.readertreepanel.readertreectrl,
            )

        if appstyle & smartcard.wx.SimpleSCardApp.TR_SMARTCARD:
            self.Bind(
                wx.EVT_TREE_ITEM_ACTIVATED,
                self.OnActivateCard,
                self.readertreepanel.cardtreectrl,
            )
            self.Bind(
                wx.EVT_TREE_SEL_CHANGED,
                self.OnSelectCard,
                self.readertreepanel.cardtreectrl,
            )
            self.Bind(
                wx.EVT_TREE_ITEM_RIGHT_CLICK,
                self.OnCardRightClick,
                self.readertreepanel.cardtreectrl,
            )

        self.SetSizer(boxsizer)
        self.SetAutoLayout(True)

    def ActivateCard(self, card):
        """Activate a card."""
        if not hasattr(card, "connection"):
            card.connection = card.createConnection()
            if None != self.parent.apdutracerpanel:
                card.connection.addObserver(self.parent.apdutracerpanel)
            card.connection.connect()
        self.dialogpanel.OnActivateCard(card)

    def DeactivateCard(self, card):
        """Deactivate a card."""
        if hasattr(card, "connection"):
            card.connection.disconnect()
            if None != self.parent.apdutracerpanel:
                card.connection.deleteObserver(self.parent.apdutracerpanel)
            delattr(card, "connection")
        self.dialogpanel.OnDeactivateCard(card)

    def OnActivateCard(self, event):
        """Called when the user activates a card in the tree."""
        item = event.GetItem()
        if item:
            itemdata = self.readertreepanel.cardtreectrl.GetItemData(item)
            if isinstance(itemdata, smartcard.Card.Card):
                self.ActivateCard(itemdata)
            else:
                self.dialogpanel.OnDeselectCard(itemdata)

    def OnActivateReader(self, event):
        """Called when the user activates a reader in the tree."""
        item = event.GetItem()
        if item:
            itemdata = self.readertreepanel.readertreectrl.GetItemData(item)
            if isinstance(itemdata, smartcard.Card.Card):
                self.ActivateCard(itemdata)
            elif isinstance(itemdata, smartcard.reader.Reader.Reader):
                self.dialogpanel.OnActivateReader(itemdata)
        event.Skip()

    def OnItemCollapsed(self, event):
        item = event.GetItem()
        self.readertreepanel.readertreectrl.Expand(item)

    def OnCardRightClick(self, event):
        """Called when user right-clicks a node in the card tree control."""
        item = event.GetItem()
        if item:
            itemdata = self.readertreepanel.cardtreectrl.GetItemData(item)
            if isinstance(itemdata, smartcard.Card.Card):
                self.selectedcard = itemdata
                if not hasattr(self, "connectID"):
                    self.connectID = wx.NewId()
                    self.disconnectID = wx.NewId()

                    self.Bind(wx.EVT_MENU, self.OnConnect, id=self.connectID)
                    self.Bind(wx.EVT_MENU, self.OnDisconnect, id=self.disconnectID)

                menu = wx.Menu()
                if not hasattr(self.selectedcard, "connection"):
                    menu.Append(self.connectID, "Connect")
                else:
                    menu.Append(self.disconnectID, "Disconnect")
                self.PopupMenu(menu)
                menu.Destroy()

    def OnReaderRightClick(self, event):
        """Called when user right-clicks a node in the reader tree control."""
        item = event.GetItem()
        if item:
            itemdata = self.readertreepanel.readertreectrl.GetItemData(item)
            if isinstance(itemdata, smartcard.Card.Card):
                self.selectedcard = itemdata
                if not hasattr(self, "connectID"):
                    self.connectID = wx.NewId()
                    self.disconnectID = wx.NewId()

                    self.Bind(wx.EVT_MENU, self.OnConnect, id=self.connectID)
                    self.Bind(wx.EVT_MENU, self.OnDisconnect, id=self.disconnectID)

                menu = wx.Menu()
                if not hasattr(self.selectedcard, "connection"):
                    menu.Append(self.connectID, "Connect")
                else:
                    menu.Append(self.disconnectID, "Disconnect")
                self.PopupMenu(menu)
                menu.Destroy()

    def OnConnect(self, event):
        if isinstance(self.selectedcard, smartcard.Card.Card):
            self.ActivateCard(self.selectedcard)

    def OnDisconnect(self, event):
        if isinstance(self.selectedcard, smartcard.Card.Card):
            self.DeactivateCard(self.selectedcard)

    def OnSelectCard(self, event):
        """Called when the user selects a card in the tree."""
        item = event.GetItem()
        if item:
            itemdata = self.readertreepanel.cardtreectrl.GetItemData(item)
            if isinstance(itemdata, smartcard.Card.Card):
                self.dialogpanel.OnSelectCard(itemdata)
            else:
                self.dialogpanel.OnDeselectCard(itemdata)

    def OnSelectReader(self, event):
        """Called when the user selects a reader in the tree."""
        item = event.GetItem()
        if item:
            itemdata = self.readertreepanel.readertreectrl.GetItemData(item)
            if isinstance(itemdata, smartcard.Card.Card):
                self.dialogpanel.OnSelectCard(itemdata)
            elif isinstance(itemdata, smartcard.reader.Reader.Reader):
                self.dialogpanel.OnSelectReader(itemdata)
            else:
                self.dialogpanel.OnDeselectCard(itemdata)


class SimpleSCardAppFrame(wx.Frame):
    """The main frame of the simple smartcard application."""

    def __init__(
        self,
        appname,
        apppanelclass,
        appstyle,
        appicon,
        pos=(-1, -1),
        size=(-1, -1),
    ):
        """
        Constructor. Creates the frame with two panels:
          - the left-hand panel is holding the smartcard and/or reader tree
          - the right-hand panel is holding the application dialog

        @param appname: name of the application
        @param apppanelclass: the class of the panel to instantiate in the
                       L{SimpleSCardAppFrame}
        @param appstyle: a combination of the following styles (bitwise or |)
          - TR_SMARTCARD: display a smartcard tree panel
          - TR_READER: display a reader tree panel
          - TB_SMARTCARD: display a smartcard toolbar
          - TB_SMARTCARD: display a reader toolbar
          - PANEL_APDUTRACER: display an APDU tracer panel
          - default is TR_DEFAULT = TR_SMARTCARD
        @param pos: the application position as a (x,y) tuple; default is (-1,-1)
        @param size: the application window size as a (x,y) tuple; default is (-1,-1)
        """
        wx.Frame.__init__(
            self,
            None,
            wxID_SIMPLESCARDAPP_FRAME,
            appname,
            pos=pos,
            size=size,
            style=wx.DEFAULT_FRAME_STYLE,
        )

        if appicon:
            _icon = wx.Icon(appicon, wx.BITMAP_TYPE_ICO)
            self.SetIcon(_icon)
        elif os.path.exists(smartcard.wx.ICO_SMARTCARD):
            _icon = wx.Icon(smartcard.wx.ICO_SMARTCARD, wx.BITMAP_TYPE_ICO)
            self.SetIcon(_icon)

        boxsizer = wx.BoxSizer(wx.VERTICAL)
        self.treeuserpanel = TreeAndUserPanelPanel(self, apppanelclass, appstyle)
        boxsizer.Add(self.treeuserpanel, 3, wx.EXPAND | wx.ALL)

        # create a toolbar if required
        if appstyle & smartcard.wx.SimpleSCardApp.TB_SMARTCARD:
            self.toolbar = ReaderToolbar.ReaderToolbar(self)
            self.SetToolBar(self.toolbar)
        else:
            self.toolbar = None

        # create an apdu tracer console if required
        if appstyle & smartcard.wx.SimpleSCardApp.PANEL_APDUTRACER:
            self.apdutracerpanel = APDUTracerPanel.APDUTracerPanel(self)
            boxsizer.Add(self.apdutracerpanel, 1, wx.EXPAND | wx.ALL)
        else:
            self.apdutracerpanel = None

        self.SetSizer(boxsizer)
        self.SetAutoLayout(True)

        self.Bind(wx.EVT_CLOSE, self.OnCloseFrame)
        if appstyle & smartcard.wx.SimpleSCardApp.TB_SMARTCARD:
            self.Bind(
                wx.EVT_COMBOBOX, self.OnReaderComboBox, self.toolbar.readercombobox
            )

    def OnCloseFrame(self, evt):
        """Called when frame is closed, i.e. on wx.EVT_CLOSE"""
        evt.Skip()

    def OnExit(self, evt):
        """Called when frame application exits."""
        self.Close(True)
        evt.Skip()

    def OnReaderComboBox(self, event):
        """Called when the user activates a reader in the toolbar combo box."""
        cb = event.GetEventObject()
        reader = cb.GetClientData(cb.GetSelection())
        if isinstance(reader, smartcard.reader.Reader.Reader):
            self.treeuserpanel.dialogpanel.OnActivateReader(reader)
