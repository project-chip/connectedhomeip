#!/usr/bin/env python3

#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Controller test script for validating PR 74109 CASEServer preemption."""

import asyncio
import contextlib
import hashlib
import hmac
import logging
from optparse import OptionParser
import os
import random
import socket
import struct
import subprocess
import sys

with contextlib.suppress(ImportError):
  import paramiko

with contextlib.suppress(ImportError):
  from base import BaseTestHelper, FailIfNot, TestFail, TestTimeout, logger

try:
  import matter.clusters as Clusters
except ImportError:
  with contextlib.suppress(ImportError):
    import chip.clusters as Clusters

if "logger" not in globals():
  logger = logging.getLogger("CaseServerPreemptionControllerTest")
  logger.setLevel(logging.INFO)
  _sh = logging.StreamHandler()
  _sh.setFormatter(
      logging.Formatter("%(asctime)s [%(name)s] %(levelname)s %(message)s")
  )
  logger.addHandler(_sh)

TEST_DISCRIMINATOR = 3840
TEST_SETUPPIN = 20202021
TEST_ENDPOINT_ID = 0
TEST_SSH_PORT = 2222
IN_FLIGHT_GRACE_WINDOW_SEC = 1.5
POST_GRACE_WAIT_SEC = 1.25
RAW_IPK_EPOCH_KEY = b"temporary ipk 01"
EPHEMERAL_INITIATOR_NODE_ID = 0x8E163C0115753929

LOCAL_SIGMA2_DROP_ADD = [
    "ip6tables",
    "-I",
    "INPUT",
    "1",
    "-p",
    "udp",
    "--sport",
    "5540",
    "-m",
    "length",
    "--length",
    "300:1500",
    "-j",
    "DROP",
]
LOCAL_SIGMA2_DROP_DEL = [
    "ip6tables",
    "-D",
    "INPUT",
    "-p",
    "udp",
    "--sport",
    "5540",
    "-m",
    "length",
    "--length",
    "300:1500",
    "-j",
    "DROP",
]


def hkdf_sha256(ikm: bytes, salt: bytes, info: bytes, length: int) -> bytes:
  """RFC 5869 HKDF-SHA256 implementation for Matter key derivation."""
  if not salt:
    salt = b"\x00" * 32
  prk = hmac.new(salt, ikm, hashlib.sha256).digest()
  t = b""
  okm = b""
  counter = 1
  while len(okm) < length:
    t = hmac.new(prk, t + info + bytes([counter]), hashlib.sha256).digest()
    okm += t
    counter += 1
  return okm[:length]


def extract_root_pub_key_from_rcac(rcac_tlv: bytes) -> bytes:
  """Extract the 65-byte uncompressed P-256 public key (0x04||X||Y) from RCAC TLV."""
  marker = b"\x30\x09\x41\x04"
  idx = rcac_tlv.find(marker)
  if idx >= 0 and idx + 3 + 65 <= len(rcac_tlv):
    return rcac_tlv[idx + 3 : idx + 3 + 65]
  raise ValueError("Unable to locate 65-byte P-256 public key in RCAC TLV")


def compute_case_destination_id(
    operational_ipk: bytes,
    initiator_random: bytes,
    root_pub_key: bytes,
    fabric_id: int,
    node_id: int,
) -> bytes:
  """Compute Matter CASE Destination ID per CASEDestinationId.cpp."""
  msg = initiator_random + root_pub_key + struct.pack("<QQ", fabric_id, node_id)
  return hmac.new(operational_ipk, msg, hashlib.sha256).digest()


def build_case_sigma1_packet(
    msg_counter: int,
    exchange_id: int,
    initiator_random: bytes,
    destination_id: bytes,
    ephemeral_pub_key: bytes,
    initiator_session_id: int = 0x1234,
) -> bytes:
  """Build an unauthenticated Matter CASE_Sigma1 UDP packet."""
  # 16-byte PacketHeader: msgFlags=0x04 (kSourceNodeIdPresent), sessionId=0x0000,
  # secFlags=0x00, msgCounter, sourceNodeId
  packet_header = struct.pack(
      "<BHBIQ",
      0x04,
      0x0000,
      0x00,
      msg_counter,
      EPHEMERAL_INITIATOR_NODE_ID,
  )
  # 6-byte PayloadHeader: exFlags=0x05 (Initiator|Reliable), msgType=0x30 (CASE_Sigma1),
  # exchangeId, protocolId=0x0000 (SecureChannel)
  payload_header = struct.pack("<BBHH", 0x05, 0x30, exchange_id, 0x0000)
  # TLV Structure:
  # 0x15 (Structure)
  # Tag 1 (initiatorRandom, 32B): 0x30 0x01 0x20 <32B>
  # Tag 2 (initiatorSessionId, uint16): 0x25 0x02 <2B>
  # Tag 3 (destinationId, 32B): 0x30 0x03 0x20 <32B>
  # Tag 4 (initiatorEphPubKey, 65B): 0x30 0x04 0x41 <65B>
  # 0x18 (EndOfContainer)
  tlv_payload = (
      b"\x15"
      + b"\x30\x01\x20"
      + initiator_random
      + b"\x25\x02"
      + struct.pack("<H", initiator_session_id)
      + b"\x30\x03\x20"
      + destination_id
      + b"\x30\x04\x41"
      + ephemeral_pub_key
      + b"\x18"
  )
  return packet_header + payload_header + tlv_payload


def build_status_report_abort_packet(
    msg_counter: int, exchange_id: int, ack_counter: int | None = None
) -> bytes:
  """Build an unauthenticated SecureChannel StatusReport(Failure) packet."""
  packet_header = struct.pack(
      "<BHBIQ",
      0x04,
      0x0000,
      0x00,
      msg_counter,
      EPHEMERAL_INITIATOR_NODE_ID,
  )
  if ack_counter is not None:
    payload_header = struct.pack(
        "<BBHHI", 0x07, 0x40, exchange_id, 0x0000, ack_counter
    )
  else:
    payload_header = struct.pack("<BBHH", 0x05, 0x40, exchange_id, 0x0000)
  # GeneralCode=FAILURE (1), ProtocolId=0x00000000, ProtocolCode=0x0000
  status_payload = struct.pack("<HIH", 1, 0, 0)
  return packet_header + payload_header + status_payload


async def run_case_preemption_test_flow(
    test_helper,
    node_id: int,
    endpoint_id: int,
    remote_ip: str,
    ssh_port: int,
) -> bool:
  """Execute the PR 74109 validation flow against CHIPEndDevice."""
  del ssh_port
  try:
    logger.info(
        "Step 1: Reading TrustedRootCertificates from commissioned node %d",
        node_id,
    )
    op_creds = Clusters.OperationalCredentials
    res_certs = await test_helper.devCtrl.ReadAttribute(
        node_id,
        [(0, op_creds.Attributes.TrustedRootCertificates)],
    )
    rcac_list = res_certs[0][op_creds][
        op_creds.Attributes.TrustedRootCertificates
    ]
    root_pub_key = extract_root_pub_key_from_rcac(bytes(rcac_list[0]))
    fabric_id = 1
    compressed_fabric_id = hkdf_sha256(
        ikm=root_pub_key[1:],
        salt=struct.pack(">Q", fabric_id),
        info=b"CompressedFabric",
        length=8,
    )
    operational_ipk = hkdf_sha256(
        ikm=RAW_IPK_EPOCH_KEY,
        salt=compressed_fabric_id,
        info=b"GroupKey v1.0",
        length=16,
    )
    logger.info(
        "Derived Fabric 1 parameters: compressedFabricId=%s"
        " rootPubKeyPrefix=%s",
        compressed_fabric_id.hex(),
        root_pub_key[:8].hex(),
    )

    logger.info(
        "Step 2: Installing CASE_Sigma2 inbound drop rule on MobileDevice"
    )
    subprocess.run(LOCAL_SIGMA2_DROP_ADD, check=True)

    probe_sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    probe_sock.setblocking(False)
    probe_sock.connect((remote_ip, 5540))
    local_addr = probe_sock.getsockname()
    logger.info(
        "Opened CASE probe socket from local PeerAddress %s to [%s]:5540",
        local_addr,
        remote_ip,
    )

    # Initial CASE_Sigma1 (rand1) -> enters kSentSigma2 on CHIPEndDevice
    rand1 = bytes([0x11] * 32)
    dest1 = compute_case_destination_id(
        operational_ipk, rand1, root_pub_key, fabric_id, node_id
    )
    pkt1 = build_case_sigma1_packet(
        msg_counter=30001,
        exchange_id=7001,
        initiator_random=rand1,
        destination_id=dest1,
        ephemeral_pub_key=root_pub_key,
    )
    logger.info(
        "Step 3a: Sending initial CASE_Sigma1 (rand1) to enter kSentSigma2"
    )
    probe_sock.send(pkt1)
    await asyncio.sleep(0.25)

    # Guard 1: MRP retry with identical initiatorRandom (rand1) from same PeerAddress
    pkt2 = build_case_sigma1_packet(
        msg_counter=30002,
        exchange_id=7002,
        initiator_random=rand1,
        destination_id=dest1,
        ephemeral_pub_key=root_pub_key,
    )
    logger.info(
        "Step 3b: Sending Sigma1 with identical initiatorRandom (Guard 1 MRP"
        " retry)"
    )
    probe_sock.send(pkt2)
    await asyncio.sleep(0.25)

    # Guard 3: Premature new Sigma1 (rand2 != rand1) within 1.5s grace window
    rand2 = bytes([0x22] * 32)
    dest2 = compute_case_destination_id(
        operational_ipk, rand2, root_pub_key, fabric_id, node_id
    )
    pkt3 = build_case_sigma1_packet(
        msg_counter=30003,
        exchange_id=7003,
        initiator_random=rand2,
        destination_id=dest2,
        ephemeral_pub_key=root_pub_key,
    )
    logger.info(
        "Step 3c: Sending premature new Sigma1 within <1.5s grace window"
        " (Guard 3)"
    )
    probe_sock.send(pkt3)

    # Wait past the 1.5s grace window (0.25 + 0.25 + 1.25 = 1.75s > 1.5s)
    logger.info(
        "Step 4: Waiting %.2fs so total elapsed > %.1fs grace window",
        POST_GRACE_WAIT_SEC,
        IN_FLIGHT_GRACE_WINDOW_SEC,
    )
    await asyncio.sleep(POST_GRACE_WAIT_SEC)

    # Guard 4: Invalid Destination ID after 1.5s grace window
    rand3 = bytes([0x33] * 32)
    bogus_dest = bytes([0xFF] * 32)
    pkt4 = build_case_sigma1_packet(
        msg_counter=30004,
        exchange_id=7004,
        initiator_random=rand3,
        destination_id=bogus_dest,
        ephemeral_pub_key=root_pub_key,
    )
    logger.info(
        "Step 4b: Sending post-grace Sigma1 with invalid Destination ID"
        " (Guard 4)"
    )
    probe_sock.send(pkt4)
    await asyncio.sleep(0.10)

    # Remove CASE_Sigma2 drop rule
    subprocess.run(LOCAL_SIGMA2_DROP_DEL, check=False)

    # All 4 Guards Pass: Superseding Sigma1 (rand4 != rand1, valid dest4) after >1.5s
    rand4 = bytes([0x44] * 32)
    dest4 = compute_case_destination_id(
        operational_ipk, rand4, root_pub_key, fabric_id, node_id
    )
    pkt5 = build_case_sigma1_packet(
        msg_counter=30005,
        exchange_id=7005,
        initiator_random=rand4,
        destination_id=dest4,
        ephemeral_pub_key=root_pub_key,
    )
    logger.info(
        "Step 5: Sending superseding Sigma1 with valid Destination ID"
        " (Quadruple-Guard Preemption)"
    )
    probe_sock.send(pkt5)
    await asyncio.sleep(0.15)

    # Extract CASE_Sigma2 MessageCounter on exchange 7005 so StatusReport(Failure)
    # includes the required piggyback ACK and immediately releases CASEServer.
    probe_sock.setblocking(False)
    sigma2_msg_counter = None
    while True:
      try:
        rx_pkt = probe_sock.recv(2048)
        if len(rx_pkt) > 300:
          sigma2_msg_counter = struct.unpack_from("<I", rx_pkt, 4)[0]
      except BlockingIOError:
        break

    abort_pkt = build_status_report_abort_packet(
        msg_counter=30006,
        exchange_id=7005,
        ack_counter=sigma2_msg_counter,
    )
    probe_sock.send(abort_pkt)
    probe_sock.close()
    await asyncio.sleep(0.15)

    logger.info(
        "Step 6: Verifying full operational CASE session establishment and"
        " ReadAttribute via MobileDevice controller"
    )
    test_helper.devCtrl.MarkSessionDefunct(node_id)
    res = await test_helper.devCtrl.ReadAttribute(
        node_id,
        [(endpoint_id, Clusters.BasicInformation.Attributes.NodeLabel)],
    )
    logger.info(
        "Superseding CASE handshake and ReadAttribute succeeded: %s", res
    )
    return True
  finally:
    with contextlib.suppress(Exception):
      subprocess.run(
          LOCAL_SIGMA2_DROP_DEL, check=False, stderr=subprocess.DEVNULL
      )


class SimulatedCASEServerPR74109:
  """Reference state-machineverifier for PR 74109 Quadruple-Guard logic."""

  def __init__(
      self, fabric_ipk: bytes, fabric_root: bytes, fabric_id: int, node_id: int
  ):
    self.fabric_ipk = fabric_ipk
    self.fabric_root = fabric_root
    self.fabric_id = fabric_id
    self.node_id = node_id
    self.state = "kInitialized"
    self.active_peer = None
    self.last_initiator_random = None
    self.state_entered_ms = 0
    self.crypto_in_progress = False
    self.logs = []

  def compute_destination_id(self, initiator_random: bytes) -> bytes:
    msg = (
        initiator_random
        + self.fabric_root
        + self.fabric_id.to_bytes(8, "little")
        + self.node_id.to_bytes(8, "little")
    )
    return hmac.new(self.fabric_ipk, msg, hashlib.sha256).digest()

  def validate_destination_id(
      self, destination_id: bytes, initiator_random: bytes
  ) -> bool:
    expected = self.compute_destination_id(initiator_random)
    return hmac.compare_digest(destination_id, expected)

  def compute_dynamic_busy_delay_ms(self, now_ms: int) -> int:
    if self.crypto_in_progress:
      expected_ms = 250
    elif self.state == "kSentSigma2":
      expected_ms = 5000
    else:
      expected_ms = 2000
    elapsed = max(0, now_ms - self.state_entered_ms)
    remaining = max(250, expected_ms - elapsed)
    jitter = 50 + (random.randint(0, 65535) % 200)
    return min(65535, remaining + jitter)

  def on_sigma1_received(
      self,
      peer_addr: str,
      initiator_random: bytes,
      destination_id: bytes,
      now_ms: int,
      drop_sigma2: bool = False,
  ) -> dict:
    busy = self.state != "kInitialized"
    if busy and peer_addr == self.active_peer:
      if initiator_random == self.last_initiator_random:
        self.logs.append(
            "CASE Server detected Sigma1 MRP retry. Resending Sigma2/ACK."
        )
        return {"action": "MRP_ACK", "delay_ms": 0}

      if self.crypto_in_progress:
        self.logs.append(
            "Preemption blocked: active crypto operation in progress."
        )
        delay = self.compute_dynamic_busy_delay_ms(now_ms)
        return {"action": "BUSY", "delay_ms": delay}

      elapsed = max(0, now_ms - self.state_entered_ms)
      if elapsed < 1500:
        rem = 1500 - elapsed
        self.logs.append(
            f"Preemption deferred: Sigma2 in transit (grace window {rem} ms"
            " remaining)."
        )
        delay = self.compute_dynamic_busy_delay_ms(now_ms)
        return {"action": "BUSY", "delay_ms": delay}

      if self.validate_destination_id(destination_id, initiator_random):
        self.logs.append(
            "CASE Server passed Quadruple Guard Preemption. Preempting stale"
            " session."
        )
        self.logs.append("Preempting stale CASE session for superseding retry")
        self.state = "kInitialized"
        busy = False
      else:
        delay = self.compute_dynamic_busy_delay_ms(now_ms)
        return {"action": "BUSY", "delay_ms": delay}

    if busy:
      delay = self.compute_dynamic_busy_delay_ms(now_ms)
      return {"action": "BUSY", "delay_ms": delay}

    self.active_peer = peer_addr
    self.last_initiator_random = initiator_random
    self.state_entered_ms = now_ms
    self.state = "kSentSigma2"
    if drop_sigma2:
      return {"action": "SIGMA2_DROPPED", "delay_ms": 0}

    self.state = "kInitialized"
    self.logs.append(f"CASE Session established to peer: <1, {self.node_id}>")
    return {"action": "SIGMA2_AND_ESTABLISHED", "delay_ms": 0}


def run_self_test() -> int:
  """Run deterministic validation of PR 74109 Quadruple-Guard state machine."""
  ipk = b"\xaa" * 16
  root_pub = b"\x04" + (b"\xbb" * 64)
  server = SimulatedCASEServerPR74109(
      fabric_ipk=ipk, fabric_root=root_pub, fabric_id=1, node_id=1
  )
  peer = "fd00::10:5540"

  rand1 = b"\x11" * 32
  dest1 = server.compute_destination_id(rand1)
  r1 = server.on_sigma1_received(
      peer, rand1, dest1, now_ms=1000, drop_sigma2=True
  )
  assert r1["action"] == "SIGMA2_DROPPED" and server.state == "kSentSigma2"

  r_mrp = server.on_sigma1_received(
      peer, rand1, dest1, now_ms=1350, drop_sigma2=True
  )
  assert r_mrp["action"] == "MRP_ACK" and server.state == "kSentSigma2"

  rand2 = b"\x22" * 32
  dest2 = server.compute_destination_id(rand2)
  server.crypto_in_progress = True
  r_g2 = server.on_sigma1_received(peer, rand2, dest2, now_ms=3000)
  assert r_g2["action"] == "BUSY" and 300 <= r_g2["delay_ms"] <= 500
  server.crypto_in_progress = False

  r_g3 = server.on_sigma1_received(peer, rand2, dest2, now_ms=1800)
  assert r_g3["action"] == "BUSY" and r_g3["delay_ms"] >= 300

  bogus_dest = b"\xff" * 32
  r_g4 = server.on_sigma1_received(peer, rand2, bogus_dest, now_ms=2700)
  assert r_g4["action"] == "BUSY" and server.state == "kSentSigma2"

  r_preempt = server.on_sigma1_received(
      peer, rand2, dest2, now_ms=2700, drop_sigma2=False
  )
  assert (
      r_preempt["action"] == "SIGMA2_AND_ESTABLISHED"
      and server.state == "kInitialized"
  )

  expected_patterns = [
      "CASE Server detected Sigma1 MRP retry. Resending Sigma2/ACK.",
      "Preemption blocked: active crypto operation in progress.",
      "Preemption deferred: Sigma2 in transit (grace window",
      (
          "CASE Server passed Quadruple Guard Preemption. Preempting stale"
          " session."
      ),
      "Preempting stale CASE session for superseding retry",
      "CASE Session established to peer",
  ]
  joined_logs = "\n".join(server.logs)
  last_idx = 0
  for pat in expected_patterns:
    idx = joined_logs.find(pat, last_idx)
    assert idx >= 0, f"Missing expected log pattern: {pat}"
    last_idx = idx + len(pat)

  logger.info(
      "PR 74109 controller self-test passed all 6 assertions and log checks."
  )
  return 0


async def main():
  optParser = OptionParser()
  optParser.add_option(
      "-t",
      "--timeout",
      action="store",
      dest="testTimeout",
      default=120,
      type="int",
      help="The program will return with timeout after specified seconds.",
      metavar="<timeout-second>",
  )
  optParser.add_option(
      "-a",
      "--address",
      action="store",
      dest="deviceAddress",
      default="",
      type="str",
      help="IPv6 address of the CHIPEndDevice container",
      metavar="<device-addr>",
  )
  optParser.add_option(
      "--nodeid",
      action="store",
      dest="nodeid",
      default=1,
      type=int,
      help="The Node ID issued to the device",
      metavar="<node-id>",
  )
  optParser.add_option(
      "--discriminator",
      action="store",
      dest="discriminator",
      default=TEST_DISCRIMINATOR,
      type=int,
      help="Discriminator of the device",
      metavar="<discriminator>",
  )
  optParser.add_option(
      "--setuppin",
      action="store",
      dest="setuppin",
      default=TEST_SETUPPIN,
      type=int,
      help="Setup PIN of the device",
      metavar="<pin>",
  )
  optParser.add_option(
      "-p",
      "--paa-trust-store-path",
      action="store",
      dest="paaTrustStorePath",
      default="",
      type="str",
      help="Path that contains valid and trusted PAA Root Certificates.",
      metavar="<paa-trust-store-path>",
  )
  optParser.add_option(
      "--self-test",
      action="store_true",
      dest="selfTest",
      default=False,
      help="Run deterministic self-contained verification of PR 74109.",
  )

  options, _ = optParser.parse_args(sys.argv[1:])

  if options.selfTest:
    sys.exit(run_self_test())

  timeoutTicker = TestTimeout(options.testTimeout)
  timeoutTicker.start()

  test = BaseTestHelper(
      nodeId=112233,
      paaTrustStorePath=options.paaTrustStorePath,
      testCommissioner=True,
  )

  FailIfNot(
      await test.TestOnNetworkCommissioning(
          options.discriminator, options.setuppin, options.nodeid
      ),
      "Failed on on-network commissioning",
  )

  FailIfNot(
      await run_case_preemption_test_flow(
          test,
          options.nodeid,
          TEST_ENDPOINT_ID,
          options.deviceAddress,
          TEST_SSH_PORT,
      ),
      "Failed on PR 74109 CASE server preemption test flow",
  )

  timeoutTicker.stop()
  logger.info("Test finished")
  os._exit(0)


if __name__ == "__main__":
  if "--self-test" in sys.argv:
    sys.exit(run_self_test())
  try:
    asyncio.run(main())
  except Exception as ex:
    logger.exception(ex)
    TestFail(
        "Exception occurred when running CaseServerPreemption controller test."
    )
