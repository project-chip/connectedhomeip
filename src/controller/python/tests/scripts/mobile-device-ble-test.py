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

# BLE Commissioning and Thread / Wi-Fi Provisioning test between MobileDevice
# and CHIPEndDevice.

import asyncio
import logging
import os
import sys

import base
import click
import coloredlogs
from base import DEFAULT_REPL_STORAGE_PATH, BaseTestHelper, FailIfNot, SetTestSet, TestFail, TestTimeout, logger

import matter.CertificateAuthority
import matter.clusters as Clusters
import matter.FabricAdmin
import matter.native
from matter.ChipStack import ChipStack
from matter.clusters.Types import NullValue
from matter.storage import PersistentStorageJSON
from matter.tracing import TracingContext

TEST_THREAD_NETWORK_DATASET_TLV = (
    "0e080000000000010000"
    + "000300000c"
    + "35060004001fffe0"
    + "0208fedcba9876543210"
    + "0708fd00000000001234"
    + "0510ffeeddccbbaa99887766554433221100"
    + "030e54657374696e674e6574776f726b"
    + "0102d252"
    + "041081cb3b2efa781cc778397497ff520fa50c0302a0ff"
)
TEST_THREAD_NETWORK_ID = "fedcba9876543210"
DEFAULT_WIFI_SSID = "CHIP-VirtualWiFi-AP"
DEFAULT_WIFI_PSK = "ChipWiFiPassword123"
TEST_DISCRIMINATOR = 3840
TEST_SETUPPIN = 20202021

ENDPOINT_ID = 0
LIGHTING_ENDPOINT_ID = 1

TEST_CONTROLLER_NODE_ID = 112233
TEST_DEVICE_NODE_ID = 1


def _extract_thread_extpanid(dataset_bytes: bytes) -> bytes:
    idx = 0
    while idx + 2 <= len(dataset_bytes):
        tag = dataset_bytes[idx]
        length = dataset_bytes[idx + 1]
        if idx + 2 + length > len(dataset_bytes):
            break
        if tag == 0x02 and length == 8:
            return dataset_bytes[idx + 2:idx + 2 + length]
        idx += 2 + length
    return bytes.fromhex(TEST_THREAD_NETWORK_ID)


class BleTestHelper(BaseTestHelper):

    def __init__(
        self,
        nodeId: int,
        paaTrustStorePath: str,
        bleAdapter: int = 0,
        testCommissioner: bool = False,
    ):
        matter.native.Init(bluetoothAdapter=bleAdapter)

        self.chipStack = ChipStack(
            PersistentStorageJSON(DEFAULT_REPL_STORAGE_PATH),
            enableServerInteractions=True,
        )
        self.certificateAuthorityManager = (
            matter.CertificateAuthority.CertificateAuthorityManager(
                chipStack=self.chipStack
            )
        )
        self.certificateAuthority = (
            self.certificateAuthorityManager.NewCertificateAuthority()
        )
        self.fabricAdmin = self.certificateAuthority.NewFabricAdmin(
            vendorId=0xFFF1, fabricId=1
        )
        self.devCtrl = self.fabricAdmin.NewController(
            nodeId, paaTrustStorePath, testCommissioner
        )
        self.controllerNodeId = nodeId
        self.logger = logger
        self.paaTrustStorePath = paaTrustStorePath
        logging.getLogger().setLevel(logging.DEBUG)


async def _verify_network_commissioning_attributes(
    test: BleTestHelper,
    device_nodeid: int,
    expected_net_id: bytes,
    network_label: str,
):
    logger.info(
        "Verifying NetworkCommissioning cluster state provisioned over BLE (%s)",
        network_label,
    )
    res = await test.devCtrl.ReadAttribute(
        nodeId=device_nodeid,
        attributes=[
            (ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.Networks),
            (ENDPOINT_ID, Clusters.NetworkCommissioning.Attributes.LastNetworkID),
            (
                ENDPOINT_ID,
                Clusters.NetworkCommissioning.Attributes.LastNetworkingStatus,
            ),
            (
                ENDPOINT_ID,
                Clusters.NetworkCommissioning.Attributes.LastConnectErrorValue,
            ),
        ],
        returnClusterObject=True,
    )
    net_cluster = res[ENDPOINT_ID][Clusters.NetworkCommissioning]
    logger.info("Provisioned NetworkCommissioning attributes: %s", net_cluster)
    FailIfNot(
        len(net_cluster.networks) == 1,
        f"Expected 1 provisioned {network_label} network, got {len(net_cluster.networks)}",
    )
    FailIfNot(
        net_cluster.networks[0].networkID == expected_net_id,
        f"Expected networkID {expected_net_id!r}, got {net_cluster.networks[0].networkID!r}",
    )
    FailIfNot(
        net_cluster.networks[0].connected,
        f"Expected {network_label} network to be marked connected=True",
    )
    k_success = Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess
    FailIfNot(
        net_cluster.lastNetworkingStatus == k_success,
        f"Expected LastNetworkingStatus=kSuccess, got {net_cluster.lastNetworkingStatus}",
    )
    FailIfNot(
        net_cluster.lastNetworkID == expected_net_id,
        f"Expected LastNetworkID={expected_net_id!r}, got {net_cluster.lastNetworkID!r}",
    )
    FailIfNot(
        net_cluster.lastConnectErrorValue == NullValue,
        f"Expected LastConnectErrorValue=NullValue, got {net_cluster.lastConnectErrorValue}",
    )


async def _verify_thread_commissioning_cluster(
    test: BleTestHelper, device_nodeid: int, dataset_bytes: bytes
):
    expected_net_id = _extract_thread_extpanid(dataset_bytes)
    await _verify_network_commissioning_attributes(
        test, device_nodeid, expected_net_id, "Thread"
    )


async def _verify_wifi_commissioning_cluster(
    test: BleTestHelper, device_nodeid: int, ssid: str
):
    await _verify_network_commissioning_attributes(
        test, device_nodeid, ssid.encode("utf-8"), "Wi-Fi"
    )


async def _verify_wifi_diagnostics_cluster(
    test: BleTestHelper, device_nodeid: int
):
    logger.info(
        "Verifying WiFiNetworkDiagnostics cluster state over Wi-Fi CASE session"
    )
    diag_res = await test.devCtrl.ReadAttribute(
        nodeId=device_nodeid,
        attributes=[
            (ENDPOINT_ID, Clusters.WiFiNetworkDiagnostics.Attributes.Bssid),
            (
                ENDPOINT_ID,
                Clusters.WiFiNetworkDiagnostics.Attributes.SecurityType,
            ),
            (ENDPOINT_ID, Clusters.WiFiNetworkDiagnostics.Attributes.WiFiVersion),
        ],
        returnClusterObject=True,
    )
    wifi_diag = diag_res[ENDPOINT_ID][Clusters.WiFiNetworkDiagnostics]
    logger.info("WiFiNetworkDiagnostics attributes: %s", wifi_diag)
    FailIfNot(
        isinstance(wifi_diag.bssid, bytes) and len(wifi_diag.bssid) == 6,
        f"Expected 6-byte BSSID, got {wifi_diag.bssid!r}",
    )


async def ble_thread_commissioning(
    test: BleTestHelper,
    discriminator: int,
    setup_pin: int,
    device_nodeid: int,
    dataset_hex: str,
):
    logger.info(
        "Starting BLE Commissioning + Thread Provisioning (discriminator=%d,"
        " pin=%d, nodeId=%d)",
        discriminator,
        setup_pin,
        device_nodeid,
    )
    dataset_bytes = bytes.fromhex(dataset_hex)
    commissioned_node_id = await test.devCtrl.CommissionBleThread(
        discriminator,
        setup_pin,
        device_nodeid,
        dataset_bytes,
    )
    FailIfNot(
        commissioned_node_id == device_nodeid,
        "CommissionBleThread failed or returned unexpected nodeId:"
        f" {commissioned_node_id}",
    )
    logger.info(
        "BLE Commissioning and Thread Provisioning succeeded for nodeId=%d",
        commissioned_node_id,
    )
    await _verify_thread_commissioning_cluster(test, device_nodeid, dataset_bytes)


async def ble_wifi_commissioning(
    test: BleTestHelper,
    discriminator: int,
    setup_pin: int,
    device_nodeid: int,
    ssid: str,
    password: str,
):
    logger.info(
        "Starting BLE Commissioning + Wi-Fi Provisioning (discriminator=%d,"
        " pin=%d, nodeId=%d, ssid=%s)",
        discriminator,
        setup_pin,
        device_nodeid,
        ssid,
    )
    commissioned_node_id = await test.devCtrl.CommissionBleWiFi(
        discriminator,
        setup_pin,
        device_nodeid,
        ssid,
        password,
    )
    FailIfNot(
        commissioned_node_id == device_nodeid,
        "CommissionBleWiFi failed or returned unexpected nodeId:"
        f" {commissioned_node_id}",
    )
    logger.info(
        "BLE Commissioning and Wi-Fi Provisioning succeeded for nodeId=%d",
        commissioned_node_id,
    )
    await _verify_wifi_commissioning_cluster(test, device_nodeid, ssid)
    await _verify_wifi_diagnostics_cluster(test, device_nodeid)


@base.test_case
def TestDatamodel(test: BleTestHelper, device_nodeid: int):
    logger.info("Testing datamodel functions over operational CASE")

    logger.info("Testing on off cluster")
    FailIfNot(
        asyncio.run(
            test.TestOnOffCluster(
                nodeId=device_nodeid, endpoint=LIGHTING_ENDPOINT_ID
            )
        ),
        "Failed to test on off cluster",
    )

    logger.info("Testing level control cluster")
    FailIfNot(
        asyncio.run(
            test.TestLevelControlCluster(
                nodeId=device_nodeid, endpoint=LIGHTING_ENDPOINT_ID
            )
        ),
        "Failed to test level control cluster",
    )

    logger.info("Testing sending commands to non exist endpoint")
    FailIfNot(
        not asyncio.run(
            test.TestOnOffCluster(nodeId=device_nodeid, endpoint=233)
        ),
        "Failed to test on off cluster on non-exist endpoint",
    )

    logger.info("Testing attribute writing")
    FailIfNot(
        asyncio.run(
            test.TestWriteBasicAttributes(
                nodeId=device_nodeid, endpoint=ENDPOINT_ID
            )
        ),
        "Failed to test Write Basic Attributes",
    )

    logger.info("Testing attribute reading")
    FailIfNot(
        asyncio.run(
            test.TestReadBasicAttributes(
                nodeId=device_nodeid, endpoint=ENDPOINT_ID
            )
        ),
        "Failed to test Read Basic Attributes",
    )


def do_tests(config: dict):
    timeoutTicker = TestTimeout(config["timeout"])
    timeoutTicker.start()

    device_nodeid = config["device_nodeid"]
    network = config["network"].lower()
    test = BleTestHelper(
        nodeId=config["controller_nodeid"],
        paaTrustStorePath=config["paa_trust_store_path"],
        bleAdapter=config["ble_adapter"],
    )

    if network == "thread":
        asyncio.run(
            ble_thread_commissioning(
                test,
                config["discriminator"],
                config["setup_pin"],
                device_nodeid,
                config["dataset_hex"],
            )
        )
    elif network == "wifi":
        asyncio.run(
            ble_wifi_commissioning(
                test,
                config["discriminator"],
                config["setup_pin"],
                device_nodeid,
                config["ssid"],
                config["wifi_password"],
            )
        )
    else:
        TestFail(f"Unsupported network type: {network}")

    logger.info("Testing resolve")
    FailIfNot(test.TestResolve(nodeId=device_nodeid), "Failed to resolve nodeid")

    TestDatamodel(test, device_nodeid)

    logger.info("Testing closing sessions")
    FailIfNot(
        test.TestCloseSession(nodeId=device_nodeid), "Failed to close sessions"
    )

    timeoutTicker.stop()
    logger.info("BLE MobileDevice Test finished")
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(0)


@click.command()
@click.option(
    "--network",
    default="thread",
    type=click.Choice(["thread", "wifi"], case_sensitive=False),
    help="Operational network to provision over BLE.",
)
@click.option(
    "--dataset-hex",
    default=TEST_THREAD_NETWORK_DATASET_TLV,
    type=str,
    help="Thread active operational dataset TLV (hex) when --network=thread.",
)
@click.option(
    "--ssid",
    default=DEFAULT_WIFI_SSID,
    type=str,
    help="Wi-Fi SSID to provision over BLE when --network=wifi.",
)
@click.option(
    "--wifi-password",
    default=DEFAULT_WIFI_PSK,
    type=str,
    help="Wi-Fi PSK password to provision over BLE when --network=wifi.",
)
@click.option(
    "--controller-nodeid",
    default=TEST_CONTROLLER_NODE_ID,
    type=int,
    help="NodeId of the controller.",
)
@click.option(
    "--device-nodeid",
    default=TEST_DEVICE_NODE_ID,
    type=int,
    help="NodeId of the device.",
)
@click.option(
    "--timeout",
    "-t",
    default=240,
    type=int,
    help="The program will return with timeout after specified seconds.",
)
@click.option(
    "--discriminator",
    default=TEST_DISCRIMINATOR,
    type=int,
    help="Discriminator of the device.",
)
@click.option(
    "--setup-pin",
    default=TEST_SETUPPIN,
    type=int,
    help="Setup pincode of the device.",
)
@click.option(
    "--ble-adapter",
    default=0,
    type=int,
    help="Bluetooth HCI adapter index (e.g. 0 for hci0).",
)
@click.option(
    "--enable-test",
    default=["all"],
    type=str,
    multiple=True,
    help="The tests to be executed.",
)
@click.option(
    "--disable-test",
    default=[],
    type=str,
    multiple=True,
    help="The tests to be excluded.",
)
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(["ERROR", "WARN", "INFO", "DEBUG"]),
    help="The log level of the test.",
)
@click.option(
    "--log-format",
    default=None,
    type=str,
    help="Override logging format",
)
@click.option(
    "--paa-trust-store-path",
    default="",
    type=str,
    help="Path that contains valid and trusted PAA Root Certificates.",
)
@click.option(
    "--trace-to",
    multiple=True,
    default=[],
    help="Trace location",
)
def run(**kwargs):
    coloredlogs.install(
        level=kwargs["log_level"], fmt=kwargs["log_format"], logger=logger
    )
    SetTestSet(kwargs["enable_test"], kwargs["disable_test"])
    with TracingContext() as tracing_ctx:
        for destination in kwargs["trace_to"]:
            tracing_ctx.StartFromString(destination)

        do_tests(kwargs)


if __name__ == "__main__":
    try:
        run()
    except Exception as ex:
        logger.exception(ex)
        TestFail("Exception occurred when running BLE mobile device tests.")
