#
#    Copyright (c) 2021 Project CHIP Authors
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

"""
Handles linux-specific functionality for running test cases
"""

import logging
import os
import subprocess
import sys
import time

from .test_definition import ApplicationPaths

test_environ = os.environ.copy()


def EnsureNetworkNamespaceAvailability():
    if os.getuid() == 0:
        logging.debug("Current user is root")
        logging.warn("Running as root and this will change global namespaces.")
        return

    os.execvpe(
        "unshare", ["unshare", "--map-root-user", "-n", "-m", "python3",
                    sys.argv[0], '--internal-inside-unshare'] + sys.argv[1:],
        test_environ)


def EnsurePrivateState():
    logging.info("Ensuring /run is privately accessible")

    logging.debug("Making / private")
    if os.system("mount --make-private /") != 0:
        logging.error("Failed to make / private")
        logging.error("Are you using --privileged if running in docker?")
        sys.exit(1)

    logging.debug("Remounting /run")
    if os.system("mount -t tmpfs tmpfs /run") != 0:
        logging.error("Failed to mount /run as a temporary filesystem")
        logging.error("Are you using --privileged if running in docker?")
        sys.exit(1)


def CreateNamespacesForAppTest():
    """
    Creates appropriate namespaces for a tool and app binaries in a simulated
    isolated network.
    """
    COMMANDS = [
        # 2 virtual hosts: for app and for the tool
        "ip netns add app",
        "ip netns add tool",

        # create links for switch to net connections
        "ip link add eth-app type veth peer name eth-app-switch",
        "ip link add eth-tool type veth peer name eth-tool-switch",
        "ip link add eth-ci type veth peer name eth-ci-switch",

        # link the connections together
        "ip link set eth-app netns app",
        "ip link set eth-tool netns tool",

        "ip link add name br1 type bridge",
        "ip link set br1 up",
        "ip link set eth-app-switch master br1",
        "ip link set eth-tool-switch master br1",
        "ip link set eth-ci-switch master br1",

        # mark connections up
        "ip netns exec app ip addr add 10.10.10.1/24 dev eth-app",
        "ip netns exec app ip link set dev eth-app up",
        "ip netns exec app ip link set dev lo up",
        "ip link set dev eth-app-switch up",

        "ip netns exec tool ip addr add 10.10.10.2/24 dev eth-tool",
        "ip netns exec tool ip link set dev eth-tool up",
        "ip netns exec tool ip link set dev lo up",
        "ip link set dev eth-tool-switch up",

        # Force IPv6 to use ULAs that we control
        "ip netns exec tool ip -6 addr flush eth-tool",
        "ip netns exec app ip -6 addr flush eth-app",
        "ip netns exec tool ip -6 a add fd00:0:1:1::2/64 dev eth-tool",
        "ip netns exec app ip -6 a add fd00:0:1:1::3/64 dev eth-app",

        # create link between virtual host 'tool' and the test runner
        "ip addr add 10.10.10.5/24 dev eth-ci",
        "ip link set dev eth-ci up",
        "ip link set dev eth-ci-switch up",
    ]

    for command in COMMANDS:
        logging.debug("Executing '%s'" % command)
        if os.system(command) != 0:
            logging.error("Failed to execute '%s'" % command)
            logging.error("Are you using --privileged if running in docker?")
            sys.exit(1)

    # IPv6 does Duplicate Address Detection even though
    # we know ULAs provided are isolated. Wait for 'tenative'
    # address to be gone.

    logging.info('Waiting for IPv6 DaD to complete (no tentative addresses)')
    for i in range(100):  # wait at most 10 seconds
        output = subprocess.check_output(['ip', 'addr'])
        if b'tentative' not in output:
            logging.info('No more tentative addresses')
            break
        time.sleep(0.1)
    else:
        logging.warn("Some addresses look to still be tentative")


def RemoveNamespaceForAppTest():
    """
    Removes namespaces for a tool and app binaries previously created to simulate an
    isolated network. This tears down what was created in CreateNamespacesForAppTest.
    """
    COMMANDS = [
        "ip link set dev eth-ci down",
        "ip link set dev eth-ci-switch down",
        "ip addr del 10.10.10.5/24 dev eth-ci",

        "ip link set br1 down",
        "ip link delete br1",

        "ip link delete eth-ci-switch",
        "ip link delete eth-tool-switch",
        "ip link delete eth-app-switch",

        "ip netns del tool",
        "ip netns del app",
    ]

    for command in COMMANDS:
        logging.debug("Executing '%s'" % command)
        if os.system(command) != 0:
            breakpoint()
            logging.error("Failed to execute '%s'" % command)
            sys.exit(1)


def PrepareNamespacesForTestExecution(in_unshare: bool):
    if not in_unshare:
        EnsureNetworkNamespaceAvailability()
    elif in_unshare:
        EnsurePrivateState()

    CreateNamespacesForAppTest()


def ShutdownNamespaceForTestExecution():
    RemoveNamespaceForAppTest()


def PathsWithNetworkNamespaces(paths: ApplicationPaths) -> ApplicationPaths:
    """
    Returns a copy of paths with updated command arrays to invoke the
    commands in an appropriate network namespace.
    """
    return ApplicationPaths(
        chip_tool='ip netns exec tool'.split() + paths.chip_tool,
        all_clusters_app='ip netns exec app'.split() + paths.all_clusters_app,
        lock_app='ip netns exec app'.split() + paths.lock_app,
        fabric_bridge_app='ip netns exec app'.split() + paths.fabric_bridge_app,
        ota_provider_app='ip netns exec app'.split() + paths.ota_provider_app,
        ota_requestor_app='ip netns exec app'.split() + paths.ota_requestor_app,
        tv_app='ip netns exec app'.split() + paths.tv_app,
        lit_icd_app='ip netns exec app'.split() + paths.lit_icd_app,
        microwave_oven_app='ip netns exec app'.split() + paths.microwave_oven_app,
        rvc_app='ip netns exec app'.split() + paths.rvc_app,
        network_manager_app='ip netns exec app'.split() + paths.network_manager_app,
        bridge_app='ip netns exec app'.split() + paths.bridge_app,
        chip_repl_yaml_tester_cmd='ip netns exec tool'.split() + paths.chip_repl_yaml_tester_cmd,
        chip_tool_with_python_cmd='ip netns exec tool'.split() + paths.chip_tool_with_python_cmd,
    )
