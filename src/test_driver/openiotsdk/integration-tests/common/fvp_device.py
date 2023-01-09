#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import logging
import os
import subprocess
import threading
from time import sleep

from .device import Device

log = logging.getLogger(__name__)


class FvpDevice(Device):

    def __init__(self, fvp, fvp_config, binary_file, connection_channel, network_interface, name=None):

        self.run = False
        self.connection_channel = connection_channel
        super(FvpDevice, self).__init__(name)

        input_thread_name = '<-- {}'.format(self.name)
        output_thread_name = '--> {}'.format(self.name)

        self.fvp_cmd = [
            fvp,
            '-f', f'{fvp_config}',
            '--application',  f'{binary_file}',
            '--quantum=25',
            '-C', 'mps3_board.telnetterminal0.start_port={}'.format(self.connection_channel.get_port())
        ]

        if network_interface != None:
            self.fvp_cmd.extend(['-C', 'mps3_board.hostbridge.interfaceName={}'.format(network_interface), ])
        else:
            self.fvp_cmd.extend(['-C', 'mps3_board.hostbridge.userNetworking=1'])

        self.it = threading.Thread(
            target=self._input_thread, name=input_thread_name)
        self.ot = threading.Thread(
            target=self._output_thread, name=output_thread_name)

    def start(self):
        """
        Start the FVP and connection channel with device
        """
        log.info('Starting "{}" runner...'.format(self.name))

        self.proc = subprocess.Popen(self.fvp_cmd)
        sleep(3)
        self.connection_channel.open()
        self.run = True
        self.it.start()
        self.ot.start()
        log.info('"{}" runner started'.format(self.name))

    def stop(self):
        """
        Stop the FVP and connection channel
        """
        log.info('Stopping "{}" runner...'.format(self.name))
        self.run = False
        self.connection_channel.close()
        self.oq.put(None)
        self.it.join()
        self.ot.join()
        self.proc.terminate()
        self.proc.wait()
        log.info('"{}" runner stoped'.format(self.name))

    def _input_thread(self):
        while self.run:
            line = self.connection_channel.readline()
            if line:
                log.info('<--|{}| {}'.format(self.name, line.strip()))
                self.iq.put(line)
            else:
                pass

    def _output_thread(self):
        while self.run:
            line = self.oq.get()
            if line:
                log.info('-->|{}| {}'.format(self.name, line.strip()))
                self.connection_channel.write(line)
            else:
                log.debug('Nothing sent')
