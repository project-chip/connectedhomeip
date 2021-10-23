#!/usr/bin/env python3

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2018-2019 Google LLC.
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

#
#    @file
#       Implements ChipInet class that tests Inet Layer multicast
#       among multiple nodes.
#

import os
import sys
import time

from happy.ReturnMsg import ReturnMsg
from happy.Utils import *
from happy.utils.IP import IP
from happy.HappyNode import HappyNode
from happy.HappyNetwork import HappyNetwork

from ChipTest import ChipTest


options = {}
options["configuration"] = None
options["interface"] = None
options["ipversion"] = None
options["transport"] = None
options["interval"] = None
options["quiet"] = False
options["tap"] = None


def option():
    return options.copy()


class ChipInetMulticast(HappyNode, HappyNetwork, ChipTest):
    """
    Note:
        Supports two networks: IPv4 and IPv6
        Supports one transport: UDP
    return:
        0   success
        1   failure
    """

    def __init__(self, opts=options):
        HappyNode.__init__(self)
        HappyNetwork.__init__(self)
        ChipTest.__init__(self)

        # Dictionary that will eventually contain 'node' and 'tag' for
        # the sender node process.

        self.sender = {}

        # Array that will eventually contain 'node' and 'tag'
        # dictionaries for the receiver node processes.

        self.receivers = []

        self.configuration = opts["configuration"]
        self.interface = opts["interface"]
        self.ipversion = opts["ipversion"]
        self.transport = opts["transport"]
        self.interval = opts["interval"]
        self.quiet = opts["quiet"]
        self.tap = opts["tap"]

    def __log_error_and_exit(self, error):
        self.logger.error("[localhost] ChipInetMulticast: %s" % (error))
        sys.exit(1)

    def __checkNodeExists(self, node, description):
        if not self._nodeExists(node):
            emsg = "The %s '%s' does not exist in the test topology." % (
                description, node)
            self.__log_error_and_exit(emsg)

    def __pre_check(self):
        """Sanity check the instantiated options and configuration"""
        # Sanity check the transport
        if self.transport != "udp":
            emsg = "Transport type must be 'udp'."
            self.__log_error_and_exit(emsg)

        # Sanity check the IP version
        if self.ipversion != "4" and self.ipversion != "6":
            emsg = "The IP version must be one of '4' or '6'."
            self.__log_error_and_exit(emsg)

        # Sanity check the configuration
        if self.configuration == None:
            emsg = "The test configuration is missing."
            self.__log_error_and_exit(emsg)

        # There must be exactly one sender
        if self.configuration['sender'] == None or len(self.configuration['sender']) != 1:
            emsg = "The test configuration must have exactly one sender, 'sender'."
            self.__log_error_and_exit(emsg)

        # There must be at least one receiver
        if self.configuration["receivers"] == None or len(self.configuration["receivers"]) < 1:
            emsg = "The test configuration must at least one receiver in 'receivers'."
            self.__log_error_and_exit(emsg)

        # Each specified sender and receiver node must exist in the
        # loaded Happy configuration.

        for node in self.configuration['sender'].keys():
            self.__checkNodeExists(node, "sender")

        for node in self.configuration['receivers'].keys():
            self.__checkNodeExists(node, "receiver")

        if not self.interval == None and not self.interval.isdigit():
            emsg = "Please specify a valid send interval in milliseconds."
            self.__log_error_and_exit(emsg)

    def __gather_process_results(self, process, quiet):
        """Gather and return the exit status and output as a tuple for the
        specified process node and tag.
        """
        node = process['node']
        tag = process['tag']

        status, output = self.get_test_output(node, tag, quiet)

        return (status, output)

    def __gather_sender_results(self, quiet):
        """Gather and return the exit status and output as a tuple for the
        sender process.
        """
        status, output = self.__gather_process_results(self.sender, quiet)

        return (status, output)

    def __gather_receiver_results(self, receiver, quiet):
        """Gather and return the exit status and output as a tuple for the
        specified receiver process.
        """
        status, output = self.__gather_process_results(receiver, quiet)

        return (status, output)

    def __gather_receivers_results(self, quiet):
        """Gather and return the exit status and output as a tuple for all
        receiver processes.
        """
        receiver_results = {}
        receivers_results = []

        for receiver in self.receivers:
            receiver_results['status'], receiver_results['output'] = \
                self.__gather_receiver_results(receiver, quiet)
            receivers_results.append(receiver_results)

        return (receivers_results)

    def __gather_results(self):
        """Gather and return the exit status and output as a dictionary for all
        sender and receiver processes.
        """
        quiet = True
        results = {}
        sender_results = {}
        receivers_results = []

        sender_results['status'], sender_results['output'] = \
            self.__gather_sender_results(quiet)

        receivers_results = self.__gather_receivers_results(quiet)

        results['sender'] = sender_results
        results['receivers'] = receivers_results

        return (results)

    def __process_results(self, results):
        status = True
        output = {}
        output['sender'] = ""
        output['receivers'] = []

        # Iterate through the sender and receivers return status. If
        # all had successful (0) status, then the cumulative return
        # status is successful (True). If any had unsuccessful status,
        # then the cumulative return status is unsuccessful (False).
        #
        # For the output, simply key it by sender and receivers.

        # Sender

        status = (results['sender']['status'] == 0)

        output['sender'] = results['sender']['output']

        # Receivers

        for receiver_results in results['receivers']:
            status = (status and (receiver_results['status'] == 0))
            output['receivers'].append(receiver_results['output'])

        return (status, output)

    def __start_node(self, node, attributes, extra, tag):
        """Start the test process on the specified node with the provided
        tag using the provided attributes and extra command line
        options.
        """
        cmd = self.getChipInetLayerMulticastPath()

        # Generate and accumulate the multicast group-related command
        # line options.

        for group in attributes['groups']:
            cmd += " --group %u" % (group['id'])
            cmd += " --group-expected-tx-packets %u" % (group['send'])
            cmd += " --group-expected-rx-packets %u" % (group['receive'])

        # Generate and accumulate the transport and IP version command
        # line arguments.

        cmd += " --ipv" + self.ipversion

        cmd += " --" + self.transport

        # If present, generate and accumulate the LwIP hosted OS
        # network tap device interface.

        if self.tap:
            cmd += " --tap-device " + self.tap

            # If present, generate and accumulate the LwIP hosted OS
            # network local IPv4 address.

            if self.ipversion == "4" and attributes.has_key('tap-ipv4-local-addr'):
                cmd += " --local-addr " + attributes['tap-ipv4-local-addr']

        # Generate and accumulate, if present, the bound network
        # interface command line option.

        if not self.interface == None:
            cmd += " --interface " + self.interface

        # Accumulate any extra command line options specified.

        cmd += extra

        self.logger.debug(
            "[localhost] ChipInetMulticast: Will start process on node '%s' with tag '%s' and command '%s'" % (node, tag, cmd))

        self.start_chip_process(
            node, cmd, tag, sync_on_output=self.ready_to_service_events_str)

    def __start_receiver(self, node, attributes, identifier):
        """Start a receiver test process on the specified node with the
        provided attributes and tag identifier.
        """
        receiver = {}
        tag = "INET-MCAST-RX-%u" % (identifier)

        extra_cmd = " --listen"

        self.__start_node(node, attributes, extra_cmd, tag)

        receiver['node'] = node
        receiver['tag'] = tag

        self.receivers.append(receiver)

    def __start_receivers(self):
        """Start all receiver test processes."""
        receiver = {}
        identifier = 0
        for receiver in self.configuration['receivers']:
            self.__start_receiver(
                receiver, self.configuration['receivers'][receiver], identifier)
            identifier += 1

    def __start_sender(self):
        """Start the sender test process."""
        node = list(self.configuration['sender'])[0]
        attributes = self.configuration['sender'][node]
        tag = "INET-MCAST-TX-0"

        extra_cmd = ""

        if not self.interval == None:
            extra_cmd += " --interval " + str(self.interval)

        extra_cmd += " --no-loopback"

        self.__start_node(node, attributes, extra_cmd, tag)

        self.sender['node'] = node
        self.sender['tag'] = tag

    def __wait_for_sender(self):
        """Block and wait for the sender test process."""
        node = self.sender['node']
        tag = self.sender['tag']

        self.logger.debug(
            "[localhost] ChipInetMulticast: Will wait for sender on node %s with tag %s..." % (node, tag))

        self.wait_for_test_to_end(node, tag)

    def __stop_receiver(self, receiver):
        node = receiver['node']
        tag = receiver['tag']

        self.logger.debug(
            "[localhost] ChipInetMulticast: Will stop receiver on node %s with tag %s..." % (node, tag))

        self.stop_chip_process(node, tag)

    def __stop_receivers(self):
        """Stop all receiver test processes."""
        for receiver in self.receivers:
            self.__stop_receiver(receiver)

    def run(self):
        results = {}

        self.logger.debug("[localhost] ChipInetMulticast: Run.")

        self.__pre_check()

        self.__start_receivers()

        self.logger.debug("[%s] ChipInetMulticast: %u receivers should be running." % (
            "localhost", len(self.receivers)))

        for receiver in self.receivers:
            emsg = "receiver %s should be running." % (receiver['tag'])
            self.logger.debug("[%s] ChipInetMulticast: %s" %
                              (receiver['node'], emsg))

        self.__start_sender()

        self.__wait_for_sender()

        self.__stop_receivers()

        # Gather results from the sender and receivers

        results = self.__gather_results()

        # Process the results from the sender and receivers into a
        # singular status value and a results dictionary containing
        # process output.

        status, results = self.__process_results(results)

        self.logger.debug("[localhost] ChipInetMulticast: Done.")

        return ReturnMsg(status, results)
