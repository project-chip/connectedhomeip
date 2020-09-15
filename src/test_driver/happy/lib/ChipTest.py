#!/usr/bin/env python3

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2015-2017 Nest Labs, Inc.
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
#       Implements ChipTest class that provides generic CHIP testing features.
#

import os
import random
import sys
import time
import datetime

from happy.Utils import *
from Chip import Chip

import happy.HappyProcessOutput
import happy.HappyProcessStart
import happy.HappyProcessStop
import happy.HappyProcessStrace
import happy.HappyProcessWait
import happy.State

gready_to_service_events_str = "CHIP node ready to service event"


class ChipTest(Chip):
    def __init__(self):
        self.ready_to_service_events_str = gready_to_service_events_str
        Chip.__init__(self)
        if "JITTER_DISTRIBUTION_CURVE" in os.environ.keys():
            self.jitter_distribution_curve = int(
                os.environ["JITTER_DISTRIBUTION_CURVE"])
        else:
            self.jitter_distribution_curve = None

    def start_chip_process(self, node_id, cmd, tag, quiet=None, strace=True, env={}, sync_on_output=None, rootMode=False):
        emsg = "start_chip_process %s at %s node." % (tag, node_id)
        self.logger.debug("[%s] ChipTest: %s" % (node_id, emsg))

        options = happy.HappyProcessStart.option()
        options["node_id"] = node_id
        options["tag"] = tag
        options["command"] = cmd
        options["strace"] = strace
        options["env"] = env
        options["sync_on_output"] = sync_on_output
        options["rootMode"] = rootMode

        if env:
            options["with_stderr"] = True

        proc = happy.HappyProcessStart.HappyProcessStart(options)
        proc.run()

        time.sleep(0.1)

    def stop_chip_process(self, node_id, tag, quiet=None):
        emsg = "stop_chip_process %s at %s node." % (tag, node_id)
        self.logger.debug("[%s] ChipTest: %s" % (node_id, emsg))

        options = happy.HappyProcessStop.option()
        options["node_id"] = node_id
        options["tag"] = tag

        stop_server = happy.HappyProcessStop.HappyProcessStop(options)
        stop_server.run()

    def start_simple_chip_server(self, cmd_path, server_addr, node_id, tag, quiet=None, listen=True, strace=True, env={},
                                 sync_on_output=gready_to_service_events_str, use_persistent_storage=True, reset_persistent_storage=True):
        cmd = cmd_path

        if server_addr != None:
            cmd += " --node-addr " + str(server_addr)

        if listen:
            cmd += " --listen"

        persistent_storage_file = self.getHappyLogDir() + "/" + self.getStateId() + \
            "_persistent_state." + tag

        if use_persistent_storage:
            cmd += " --persistent-cntr-file " + persistent_storage_file

        if reset_persistent_storage:
            if os.path.isfile(persistent_storage_file):
                self.logger.debug("[%s] ChipTest: deleting existing persistent storage file: %s" % (
                    node_id, persistent_storage_file))
                os.remove(persistent_storage_file)

        self.logger.debug(
            "[%s] CHIP start server on address %s." % (node_id, server_addr))
        emsg = "CHIP start server with cmd: %s." % cmd
        self.logger.debug("[%s] CHIP: %s" % (node_id, emsg))

        self.start_chip_process(
            node_id, cmd, tag, quiet, strace, env=env, sync_on_output=sync_on_output)

    def jitter(self, jitter_distribution_curve):
        if isinstance(jitter_distribution_curve, int) and jitter_distribution_curve >= 0:
            fibonacci_numbers = [0, 10]
            if jitter_distribution_curve == 0:
                delayTimeSec = fibonacci_numbers[0]
            elif jitter_distribution_curve == 1:
                delayTimeSec = fibonacci_numbers[1]
            else:
                jitter_distribution_curve -= 2
                for i in xrange(jitter_distribution_curve):
                    fibonacci_numbers.append(
                        fibonacci_numbers[i+1]+fibonacci_numbers[i])
                delayTimeSec = random.choice(fibonacci_numbers)
            self.logger.debug("Test Delay %d seconds" % delayTimeSec)
            time.sleep(delayTimeSec)
        else:
            self.logger.debug(
                "Expect that jitter_distribution_curve is integer and not less than 0 %d" % jitter_distribution_curve)

    def start_simple_chip_client(self, cmd_path, client_addr, server_addr, server_id, node_id, tag,
                                 quiet=None, strace=True, env={}, use_persistent_storage=True, reset_persistent_storage=True):
        cmd = cmd_path

        if client_addr != None:
            cmd += " --node-addr " + str(client_addr)

        if server_addr != None:
            cmd += " --dest-addr " + str(server_addr)

        persistent_storage_file = self.getHappyLogDir() + "/" + self.getStateId() + \
            "_persistent_state." + tag

        if use_persistent_storage:
            cmd += " --persistent-cntr-file " + persistent_storage_file

        if reset_persistent_storage:
            if os.path.isfile(persistent_storage_file):
                self.logger.debug("[%s] ChipTest: deleting existing persistent storage file: %s" % (
                    node_id, persistent_storage_file))
                os.remove(persistent_storage_file)

        if server_id != None:
            cmd += " " + str(server_id)

        if self.jitter_distribution_curve is not None:
            self.jitter(self.jitter_distribution_curve)

        self.logger.debug("[%s] CHIP start client on address %s" %
                          (node_id, client_addr))
        emsg = "CHIP start client with cmd: %s" % cmd
        self.logger.debug("[%s] CHIP: %s" % (node_id, emsg))

        self.start_chip_process(node_id, cmd, tag, quiet, strace, env=env)

    def wait_for_test_to_end(self, node_id, tag, quiet=None, timeout=None):
        emsg = "wait_for_test_to_end for %s at %s node." % (tag, node_id)
        self.logger.debug("[%s] ChipTest: %s" % (node_id, emsg))

        if timeout == None:
            timeout = self.max_running_time

        options = happy.HappyProcessWait.option()
        options["node_id"] = node_id
        options["tag"] = tag
        options["timeout"] = timeout

        wait_for_client = happy.HappyProcessWait.HappyProcessWait(options)
        wait_for_client.run()

    def wait_for_test_time_secs(self, node_id, tag, secs=None):
        emsg = "wait_for_test_time_secs for %d secs, tag %s " % (secs, tag)
        self.logger.debug("[%s] ChipTest: %s" % (node_id, emsg))

        poll_interval_sec = 0.1
        max_poll_time_sec = secs + 60
        time_slept = 0

        state = happy.State.State()
        state.readState()
        process = state.getNodeProcess(tag, node_id)
        tail = open(process["out"], "r")
        self.logger.debug(
            "[%s] wait_for_test_time_secs: polling for output" % (self.node_id))

        initial_timestamp = None

        while (True):
            line = tail.readline()
            if not line:
                time.sleep(poll_interval_sec)
                time_slept += poll_interval_sec
                if poll_interval_sec < 20:
                    poll_interval_sec *= 2
                if (time_slept > max_poll_time_sec):
                    self.logger.debug("[%s] wait_for_test_time_secs: can't find a timestamp %d secs after %s" %
                                      (self.node_id, secs, str(initial_timestamp)))
                    self.exit()
            else:
                try:
                    # Not every line begins with a timestamp; if they do, it look like this:
                    # 2017-09-27 20:42:48-0700.402
                    # Note that for the purpose of time deltas we don't care about the timezone,
                    # so the first 19 chars are enough
                    cur_timestamp = datetime.datetime.strptime(
                        line[0:19], "%Y-%m-%d %H:%M:%S")
                    if initial_timestamp is None:
                        initial_timestamp = cur_timestamp
                except Exception:
                    # if this line does not start with a timestamp, continue
                    continue
                else:
                    # read the timestamp and check
                    delta = cur_timestamp - initial_timestamp
                    # print "delta = " + str(delta.seconds)
                    if delta.seconds >= secs:
                        break

        tail.close()
        return

    def get_test_output(self, node_id, tag, quiet=False):
        emsg = "get_test_output for %s at %s node." % (tag, node_id)
        self.logger.debug("[%s] ChipTest: %s" % (node_id, emsg))

        options = happy.HappyProcessOutput.option()
        options["node_id"] = node_id
        options["tag"] = tag
        options["quiet"] = quiet

        client_output = happy.HappyProcessOutput.HappyProcessOutput(options)
        ret = client_output.run()

        value = ret.Value()
        data = ret.Data()

        return (value, data)

    def get_test_strace(self, node_id, tag, quiet=None):
        emsg = "get_test_strace for %s at %s node." % (tag, node_id)
        self.logger.debug("[%s] ChipTest: %s" % (node_id, emsg))

        options = happy.HappyProcessStrace.option()
        options["node_id"] = node_id
        options["tag"] = tag

        client_strace = happy.HappyProcessStrace.HappyProcessStrace(options)
        ret = client_strace.run()

        value = ret.Value()
        data = ret.Data()

        return (value, data)
