"""The CardConnectionDecorator is a Decorator around the CardConnection
abstract class, and allows dynamic addition of features to the
CardConnection, e.g. implementing a secure channel..

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


class CardConnectionDecorator:
    """Card connection decorator class."""

    def __init__(self, cardConnectionComponent):
        """Construct a new card connection decorator.

        CardConnectionComponent: CardConnection component to decorate
        """
        self.component = cardConnectionComponent

    def addSWExceptionToFilter(self, exClass):
        """call inner component addSWExceptionToFilter"""
        self.component.addSWExceptionToFilter(exClass)

    def addObserver(self, observer):
        """call inner component addObserver"""
        self.component.addObserver(observer)

    def deleteObserver(self, observer):
        """call inner component deleteObserver"""
        self.component.deleteObserver(observer)

    def connect(self, protocol=None, mode=None, disposition=None):
        """call inner component connect"""
        self.component.connect(protocol, mode, disposition)

    def reconnect(self, protocol=None, mode=None, disposition=None):
        """call inner component reconnect"""
        self.component.reconnect(protocol, mode, disposition)

    def disconnect(self):
        """call inner component disconnect"""
        self.component.disconnect()

    def release(self):
        """call inner component release"""
        self.component.release()

    def getATR(self):
        """call inner component getATR"""
        return self.component.getATR()

    def getProtocol(self):
        """call inner component getProtocol"""
        return self.component.getProtocol()

    def getReader(self):
        """call inner component getReader"""
        return self.component.getReader()

    def setErrorCheckingChain(self, errorcheckingchain):
        """call inner component setErrorCheckingChain"""
        self.component.setErrorCheckingChain(errorcheckingchain)

    def setProtocol(self, protocol):
        """call inner component setProtocol"""
        return self.component.setProtocol(protocol)

    def transmit(self, command, protocol=None):
        """call inner component transmit"""
        return self.component.transmit(command, protocol)

    def control(self, controlCode, command=None):
        """call inner component control"""
        if command is None:
            command = []
        return self.component.control(controlCode, command)

    def getAttrib(self, attribId):
        """call inner component getAttrib"""
        return self.component.getAttrib(attribId)

    def __enter__(self):
        return self

    def __exit__(self, e_type, value, traceback):
        self.disconnect()
        self.release()
