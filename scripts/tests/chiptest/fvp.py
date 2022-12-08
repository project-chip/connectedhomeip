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
Handles FVP-specific functionality for running test cases
"""

import atexit
import logging
import os
import subprocess
import sys
import time
from typing import Optional

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


def _runCommands(*commands, stopOnFail=True):
    """Run a series of commands with os.system"""
    for command in commands:
        logging.debug("Executing '%s'" % command)
        if os.system(command) != 0:
            logging.error("Failed to execute '%s'" % command)
            logging.error("Are you using --privileged if running in docker?")
            if stopOnFail:
                sys.exit(1)


def _destroyNamespaceForAppTest():
    """Revert the changes made by CreateNamespacesForAppTest"""
    atexit.unregister(_destroyNamespaceForAppTest)
    _runCommands(
        'ip link delete chiphveth',
        'ip netns delete chipns',
        stopOnFail=False
    )


def CreateNamespacesForAppTest():
    """
    Creates appropriate namespaces for a tool and app binaries in a simulated
    isolated network.
    """
    _runCommands(
        'sysctl net.ipv6.conf.all.disable_ipv6=0 net.ipv4.conf.all.forwarding=1 net.ipv6.conf.all.forwarding=1',
        # Create chipns network namespace
        'ip netns add chipns',
        'ip netns exec chipns ip link set dev lo up',
        # Adding chiphveth veth with peer chipnveth
        'ip link add chiphveth type veth peer name chipnveth',
        # Set IP addresses 10.200.1.1/24 fe00::1/64 to chiphveth interface
        'ip addr add 10.200.1.1/24 dev chiphveth',
        'ip -6 addr add fe00::1/64 dev chiphveth',
        'ip link set chiphveth up',
        # Adding chipnveth veth to namespace chipns
        'ip link set chipnveth netns chipns',
        'ip netns exec chipns ip link set dev chipnveth up',
        # Create chiptap TAP device
        'ip netns exec chipns ip tuntap add dev chiptap mode tap user root',
        'ip netns exec chipns ifconfig chiptap 0.0.0.0 promisc',
        # Create chipbr bridge interface between chipnveth and chiptap
        'ip netns exec chipns ip link add chipbr type bridge',
        # Set IP addresses 10.200.1.2/24 fe00::2/64 to chipbr bridge interface
        'ip netns exec chipns ip -6 addr add fe00::2/64 dev chipbr',
        'ip netns exec chipns ip addr add 10.200.1.2/24 dev chipbr',
        'ip netns exec chipns ip addr flush dev chipnveth',
        'ip netns exec chipns ip link set chiptap master chipbr',
        'ip netns exec chipns ip link set chipnveth master chipbr',
        'ip netns exec chipns ip link set dev chipbr up',
        'ip netns exec chipns ip route add default via 10.200.1.1'
    )

    atexit.register(_destroyNamespaceForAppTest)

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


def PrepareNamespacesForTestExecution(in_unshare: bool):
    if not in_unshare:
        EnsureNetworkNamespaceAvailability()
    elif in_unshare:
        EnsurePrivateState()

    CreateNamespacesForAppTest()


def _Prefixify(prefix:str, path:Optional[str]) -> str:
    '''Return path with prefix if path is not None, else return None.'''
    return prefix + path if path else None

def PathsWithNetworkNamespaces(paths: ApplicationPaths) -> ApplicationPaths:
    """
    Returns a copy of paths with updated command arrays to invoke the
    commands in an appropriate network namespace.
    """
    prefix = 'ip netns exec chipns'.split()
    return ApplicationPaths(
        chip_tool=_Prefixify(prefix, paths.chip_tool),
        all_clusters_app=_Prefixify(prefix, paths.all_clusters_app),
        lock_app=_Prefixify(prefix, paths.lock_app),
        ota_provider_app=_Prefixify(prefix, paths.ota_provider_app),
        ota_requestor_app=_Prefixify(prefix, paths.ota_requestor_app),
        tv_app=_Prefixify(prefix, paths.tv_app),
        bridge_app=_Prefixify(prefix, paths.bridge_app)
    )
