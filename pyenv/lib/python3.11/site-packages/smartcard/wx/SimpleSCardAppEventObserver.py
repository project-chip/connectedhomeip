"""Smartcard event observer.

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


class SimpleSCardAppEventObserver:
    """This interface defines the event handlers
    called by the SimpleSCardApp."""

    def __init__(self):
        self.selectedcard = None
        self.selectedreader = None

    # callbacks from SimpleCardAppFrame controls
    def OnActivateCard(self, card):
        """Called when a card is activated in the reader
        tree control or toolbar."""
        self.selectedcard = card

    def OnActivateReader(self, reader):
        """Called when a reader is activated in the reader
        tree control or toolbar."""
        self.selectedreader = reader

    def OnDeactivateCard(self, card):
        """Called when a card is deactivated in the reader
        tree control or toolbar."""
        pass

    def OnDeselectCard(self, card):
        """Called when a card is selected in the reader
        tree control or toolbar."""
        self.selectedcard = None

    def OnSelectCard(self, card):
        """Called when a card is selected in the reader
        tree control or toolbar."""
        self.selectedcard = card

    def OnSelectReader(self, reader):
        """Called when a reader is selected in the reader
        tree control or toolbar."""
        self.selectedreader = reader
