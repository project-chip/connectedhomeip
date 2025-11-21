"""Smart card reader monitoring classes.

ReaderObserver is a base class for objects that are to be notified
upon smartcard reader insertion/removal.

ReaderMonitor is a singleton object notifying registered ReaderObservers
upon reader insertion/removal.

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

import _thread
import traceback
from threading import Event, Thread
from time import sleep

import smartcard.System
from smartcard.Exceptions import SmartcardException
from smartcard.Observer import Observable, Observer
from smartcard.Synchronization import synchronize

# pylint: disable=too-few-public-methods


# ReaderObserver interface
class ReaderObserver(Observer):
    """
    ReaderObserver is a base abstract class for objects that are to be notified
    upon smartcard reader insertion/removal.
    """

    def __init__(self):
        pass

    def update(self, observable, handlers):
        """Called upon reader insertion/removal.

        @param observable:
        @param handlers:
          - addedreaders: list of added readers causing notification
          - removedreaders: list of removed readers causing notification
        """


class ReaderMonitor(Observable):
    """Class that monitors reader insertion/removal.
    and notify observers

    note: a reader monitoring thread will be running
    as long as the reader monitor has observers, or ReaderMonitor.stop()
    is called.

    It implements the shared state design pattern, where objects
    of the same type all share the same state, in our case essentially
    the ReaderMonitoring Thread. Thanks to Frank Aune for implementing
    the shared state pattern logics.
    """

    __shared_state = {}

    def __init__(
        self, startOnDemand=True, readerProc=smartcard.System.readers, period=1
    ):
        self.__dict__ = self.__shared_state
        Observable.__init__(self)
        self.startOnDemand = startOnDemand
        self.readerProc = readerProc
        self.period = period
        if self.startOnDemand:
            self.rmthread = None
        else:
            self.rmthread = ReaderMonitoringThread(self, self.readerProc, self.period)
            self.rmthread.start()

    def addObserver(self, observer):
        """Add an observer."""
        Observable.addObserver(self, observer)

        # If self.startOnDemand is True, the reader monitoring
        # thread only runs when there are observers.
        if self.startOnDemand:
            if 0 < self.countObservers():
                if not self.rmthread:
                    self.rmthread = ReaderMonitoringThread(
                        self, self.readerProc, self.period
                    )

                    # start reader monitoring thread in another thread to
                    # avoid a deadlock; addObserver and notifyObservers called
                    # in the ReaderMonitoringThread run() method are
                    # synchronized

                    _thread.start_new_thread(self.rmthread.start, ())
        else:
            observer.update(self, (self.rmthread.readers, []))

    def deleteObserver(self, observer):
        """Remove an observer."""
        Observable.deleteObserver(self, observer)
        # If self.startOnDemand is True, the reader monitoring
        # thread is stopped when there are no more observers.
        if self.startOnDemand:
            if 0 == self.countObservers():
                self.rmthread.stop()
                del self.rmthread
                self.rmthread = None

    def __str__(self):
        return self.__class__.__name__


synchronize(
    ReaderMonitor,
    "addObserver deleteObserver deleteObservers "
    + "setChanged clearChanged hasChanged "
    + "countObservers",
)


class ReaderMonitoringThread(Thread):
    """Reader insertion thread.
    This thread polls for pcsc reader insertion, since no
    reader insertion event is available in pcsc.
    """

    # pylint: disable=too-many-instance-attributes

    __shared_state = {}

    def __init__(self, observable, readerProc, period):
        self.__dict__ = self.__shared_state
        Thread.__init__(self)
        self.observable = observable
        self.stopEvent = Event()
        self.stopEvent.clear()
        self.readers = []
        self.daemon = True
        self.name = "smartcard.ReaderMonitoringThread"
        self.readerProc = readerProc
        self.period = period

    def run(self):
        """Runs until stopEvent is notified, and notify
        observers of all reader insertion/removal.
        """

        # pylint: disable=too-many-nested-blocks

        while not self.stopEvent.is_set():
            try:
                # no need to monitor if no observers
                if 0 < self.observable.countObservers():
                    currentReaders = self.readerProc()
                    addedReaders = []
                    removedReaders = []

                    if currentReaders != self.readers:
                        for reader in currentReaders:
                            if reader not in self.readers:
                                addedReaders.append(reader)
                        for reader in self.readers:
                            if reader not in currentReaders:
                                removedReaders.append(reader)

                        if addedReaders or removedReaders:
                            # Notify observers
                            self.readers = []
                            for r in currentReaders:
                                self.readers.append(r)
                            self.observable.setChanged()
                            self.observable.notifyObservers(
                                (addedReaders, removedReaders)
                            )

                # wait every second on stopEvent
                self.stopEvent.wait(self.period)

            except SmartcardException:
                traceback.print_exc()
                # Most likely raised during interpreter shutdown due
                # to unclean exit which failed to remove all observers.
                # To solve this, we set the stop event and pass the
                # exception to let the thread finish gracefully.
                self.stopEvent.set()

    def stop(self):
        """stop the thread by signaling stopEvent"""
        self.stopEvent.set()
        self.join()


if __name__ == "__main__":
    print("insert or remove readers in the next 20 seconds")

    class printobserver(ReaderObserver):
        """a simple reader observer that prints added/removed readers"""

        def __init__(self, obsindex):
            self.obsindex = obsindex

        def update(self, observable, handlers):
            addedreaders, removedreaders = handlers
            print(f"{self.obsindex} - added:   {addedreaders}")
            print(f"{self.obsindex} - removed: {removedreaders}")

    class testthread(Thread):
        """Test class"""

        # pylint: disable=duplicate-code

        def __init__(self, obsindex):
            Thread.__init__(self)
            self.readermonitor = ReaderMonitor()
            self.obsindex = obsindex
            self.observer = None

        def run(self):
            # create and register observer
            self.observer = printobserver(self.obsindex)
            self.readermonitor.addObserver(self.observer)
            sleep(20)
            self.readermonitor.deleteObserver(self.observer)

    t1 = testthread(1)
    t2 = testthread(2)
    t1.start()
    t2.start()
    t1.join()
    t2.join()
