#!/usr/bin/env python3
"""Copyright (c) 2026 Project CHIP Authors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import logging
import shlex
import sys
from unittest.mock import patch

from helper.CHIPTestBase import CHIPVirtualHome
from helper.paths import (
    CHIP_ALL_CLUSTERS_APP_ESC,
    CHIP_REPO_STR,
    CONTROLLER_TEST_SCRIPTS_DIR_PATH,
    MATTER_CONTROLLER_INSTALL_WHEELS,
    MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC,
)

"""
Cirque integration test for connectedhomeip PR 74109:
Validate Quadruple-Guard CASE Server Preemption and Dynamic Busy Wait when
Sigma2 packets are dropped.

Steps for this test:
    1. Commission CHIPEndDevice (device1) onto Fabric 1 from MobileDevice.
    2. Install an ip6tables OUTPUT filter on CHIPEndDevice to drop outbound
       CASE_Sigma2 (UDP length 300:1500) while allowing Standalone MRP ACKs.
    3. Trigger CASE session establishment #1 from MobileDevice so CASEServer
       enters kSentSigma2 and handles incoming Sigma1 MRP retransmissions via
       Guard 1 (HandleMRPRetry) and Guard 3 (1.5s grace window).
    4. Wait >1.5s, remove the ip6tables Sigma2 drop rule, and trigger a
       superseding CASE Sigma1 from the same MobileDevice UDP PeerAddress with
       a fresh initiatorRandom and valid Fabric 1 IPK destinationId (Guard 4).
    5. Verify in CHIPEndDevice logs that CASEServer detected the Sigma1 MRP
       retry and subsequently passed Quadruple-Guard Preemption to reset the
       stale session and establish the new CASE session.
"""

logger = logging.getLogger("CaseServerPreemptionTest")
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter("%(asctime)s [%(name)s] %(levelname)s %(message)s")
)
logger.addHandler(sh)

CHIP_PORT = 5540
CIRQUE_URL = "http://localhost:5000"
TEST_EXTPANID = "fedcba9876543210"
TEST_DISCRIMINATOR = 3840
TEST_SCRIPT_ESC = shlex.quote(
    str(CONTROLLER_TEST_SCRIPTS_DIR_PATH / "case_server_preemption_test.py")
)

EXPECTED_SERVER_LOG_PATTERNS = [
    "Thread interface: wpan0",
    "MeshForwarder-",
    "CASE Server detected Sigma1 MRP retry. Resending Sigma2/ACK.",
    "CASE Server passed Quadruple Guard Preemption. Preempting stale session.",
    "Preempting stale CASE session for superseding retry",
]

DEVICE_CONFIG = {
    "device0": {
        "type": "MobileDevice",
        "base_image": "@default",
        "capability": ["Thread", "TrafficControl", "Mount"],
        "rcp_mode": True,
        "docker_network": "Ipv6",
        "traffic_control": {"latencyMs": 50},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    "device1": {
        "type": "CHIPEndDevice",
        "base_image": "@default",
        "capability": ["Thread", "TrafficControl", "Mount"],
        "rcp_mode": True,
        "docker_network": "Ipv6",
        "traffic_control": {"latencyMs": 50},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
}


class TestCaseServerPreemption(CHIPVirtualHome):

  def __init__(self, device_config):
    super().__init__(CIRQUE_URL, device_config)
    self.logger = logger

  def setup(self):
    self.initialize_home()
    self.connect_to_thread_network()

  def test_routine(self):
    self.run_case_server_preemption_test()

  def run_case_server_preemption_test(self):
    server_ids = [
        device["id"]
        for device in self.non_ap_devices
        if device["type"] == "CHIPEndDevice"
    ]
    req_ids = [
        device["id"]
        for device in self.non_ap_devices
        if device["type"] == "MobileDevice"
    ]

    server_device_id = server_ids[0]
    req_device_id = req_ids[0]

    # Start SSH daemon and launch chip-all-clusters-app --thread
    self.execute_device_cmd(server_device_id, "service ssh start")
    server_cmd = (
        "CHIPCirqueDaemon.py -- run gdb -batch -return-child-result -q "
        '-ex "set pagination off" -ex run -ex "thread apply all bt" '
        f"--args {CHIP_ALL_CLUSTERS_APP_ESC} --thread "
        f"--discriminator {TEST_DISCRIMINATOR}"
    )
    self.execute_device_cmd(server_device_id, server_cmd)

    self.assertTrue(
        self.wait_for_device_output(
            server_device_id, "Thread interface: wpan0", 15
        )
    )
    self.assertTrue(
        self.wait_for_device_output(
            server_device_id, "[SVR] Server Listening...", 15
        )
    )

    # Retrieve Thread OMR / Mesh-Local IPv6 address on wpan0
    thread_ip = self.get_device_thread_ip(server_device_id)
    self.assertTrue(
        thread_ip is not None,
        "Failed to resolve Thread IPv6 address on wpan0 for CHIPEndDevice",
    )
    self.logger.info(
        "Resolved CHIPEndDevice Thread wpan0 IPv6 address: %s", thread_ip
    )

    self.execute_device_cmd(
        req_device_id,
        MATTER_CONTROLLER_INSTALL_WHEELS.replace(" matter_repl", ""),
    )

    command = (
        "gdb -batch -return-child-result -q -ex run "
        f'-ex "thread apply all bt" --args python3 {TEST_SCRIPT_ESC} '
        f"-t 180 -a {thread_ip} "
        f"--paa-trust-store-path {MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC} "
        f"--discriminator {TEST_DISCRIMINATOR}"
    )
    ret = self.execute_device_cmd(req_device_id, command)

    self.assertEqual(
        ret["return_code"],
        "0",
        "CaseServerPreemption controller script failed: non-zero return code",
    )

    self.logger.info(
        "Checking CHIPEndDevice log for PR 74109 Preemption on %s",
        self.get_device_pretty_id(server_device_id),
    )
    device_log = self.get_device_log(server_device_id).decode(
        "utf-8", errors="replace"
    )
    self.assertTrue(
        self.sequenceMatch(device_log, EXPECTED_SERVER_LOG_PATTERNS),
        "CaseServerPreemption test failed: missing expected PR 74109 log "
        f"sequence from {server_device_id}",
    )


def run_self_test() -> int:
  """Deterministic unit verification of TestCaseServerPreemption harness."""
  test_instance = TestCaseServerPreemption(DEVICE_CONFIG)
  test_instance.home_id = "mock_home_74109"
  test_instance.non_ap_devices = [
      {
          "id": "mob_dev_001",
          "type": "MobileDevice",
          "description": {"ipv6_addr": "fd00::10"},
      },
      {
          "id": "end_dev_001",
          "type": "CHIPEndDevice",
          "description": {"ipv6_addr": "fd00::20"},
      },
  ]
  test_instance.device_config = {
      "mob_dev_001": test_instance.non_ap_devices[0],
      "end_dev_001": test_instance.non_ap_devices[1],
  }

  simulated_server_log = (
      "Thread interface: wpan0\n[SVR] Server Listening...\notbr-agent[30]:"
      " MeshForwarder-: Received IPv6 UDP msg, radio:15.4\n[SC] CASE Server"
      " detected Sigma1 MRP retry. Resending Sigma2/ACK.\n[SC] Preemption"
      " deferred: Sigma2 in transit (grace window 700 ms).\n[SC] CASE Server"
      " passed Quadruple Guard Preemption. Preempting stale session.\n[SC]"
      " Preempting stale CASE session for superseding retry\n[SC] CASE Session"
      " established to peer\n"
  ).encode("utf-8")

  with (
      patch.object(
          test_instance,
          "execute_device_cmd",
          return_value={
              "return_code": "0",
              "output": "fd01:2345:6789:abc:1122:3344:5566:7788\nDone",
          },
      ) as mock_exec,
      patch.object(
          test_instance, "get_device_log", return_value=simulated_server_log
      ),
      patch.object(test_instance, "wait_for_device_output", return_value=True),
  ):
    test_instance.run_case_server_preemption_test()
    assert mock_exec.call_count >= 5

  logger.info("CaseServerPreemptionTest.py self-test passed all assertions.")
  return 0


if __name__ == "__main__":
  if "--self-test" in sys.argv:
    sys.exit(run_self_test())
  sys.exit(TestCaseServerPreemption(DEVICE_CONFIG).run_test())
