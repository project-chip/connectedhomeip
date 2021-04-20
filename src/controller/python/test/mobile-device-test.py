#!/usr/bin/env python3

from chip import ChipDeviceCtrl
from chip import exceptions
from optparse import OptionParser, OptionValueError
import threading
import os
import sys
import logging
import time

logger = logging.getLogger('CHIPMobileDevice')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
sh.setStream(sys.stdout)
logger.addHandler(sh)

# The thread network dataset tlv for testing, splited into T-L-V.
TEST_THREAD_NETWORK_DATASET_TLV = "0e080000000000010000" + \
                "000300000c" + \
                "35060004001fffe0" + \
                "0208fedcba9876543210" + \
                "0708fd00000000001234" + \
                "0510ffeeddccbbaa99887766554433221100" + \
                "030e54657374696e674e6574776f726b" + \
                "0102d252" + \
                "041081cb3b2efa781cc778397497ff520fa50c0302a0ff"
# Network id, for the thread network, current a const value, will be changed to XPANID of the thread network.
TEST_THREAD_NETWORK_ID = "fedcba9876543210"


def TestFail(message):
    logger.fatal("Testfail: {}".format(message))
    os._exit(1)


def FailIfNot(cond, message):
    if not cond:
        TestFail(message)


class TestTimeout(threading.Thread):
    def __init__(self, timeout: int):
        threading.Thread.__init__(self)
        self._timeout = timeout
        self._should_stop = False
        self._cv = threading.Condition()

    def stop(self):
        with self._cv:
            self._should_stop = True
            self._cv.notify_all()
        self.join()

    def run(self):
        stop_time = time.time() + self._timeout
        logger.info("Test timeout set to {} seconds".format(self._timeout))
        with self._cv:
            wait_time = stop_time - time.time()
            while wait_time > 0 and not self._should_stop:
                self._cv.wait(wait_time)
                wait_time = stop_time - time.time()
        if time.time() > stop_time:
            TestFail("Timeout")


class MobileDeviceTests():
    def __init__(self, nodeid: int):
        self.devCtrl = ChipDeviceCtrl.ChipDeviceController(
            controllerNodeId=nodeid)
        self.logger = logger

    def TestKeyExchange(self, ip: str, setuppin: int, nodeid: int):
        self.logger.info("Conducting key exchange with device {}".format(ip))
        if not self.devCtrl.ConnectIP(ip.encode("utf-8"), setuppin, nodeid):
            self.logger.info(
                "Failed to finish key exchange with device {}".format(ip))
            return False
        self.logger.info("Device finished key exchange.")
        return True

    def TestNetworkCommissioning(self, nodeid: int):
        self.logger.info("Commissioning network to device {}".format(nodeid))
        try:
            self.devCtrl.ZCLSend("NetworkCommissioning", "AddThreadNetwork", nodeid, 1, 0, {
                "operationalDataset": bytes.fromhex(TEST_THREAD_NETWORK_DATASET_TLV),
                "breadcrumb": 0,
                "timeoutMs": 1000})
        except Exception as ex:
            self.logger.exception("Failed to send AddThreadNetwork command")
            return False
        self.logger.info("Send EnableNetwork command to device {}".format(nodeid))
        try:
            self.devCtrl.ZCLSend("NetworkCommissioning", "EnableNetwork", nodeid, 1, 0, {
                "networkID": bytes.fromhex(TEST_THREAD_NETWORK_ID),
                "breadcrumb": 0,
                "timeoutMs": 1000})
        except Exception as ex:
            self.logger.exception("Failed to send EnableNetwork command")
            return False
        return True

    def TestOnOffCluster(self, nodeid: int):
        self.logger.info("Sending On/Off commands to device {}".format(nodeid))
        try:
            self.devCtrl.ZCLSend("OnOff", "On", nodeid, 1, 0, {})
        except Exception as ex:
            self.logger.exception("Failed to send On command")
            return False
        try:
            self.devCtrl.ZCLSend("OnOff", "Off", nodeid, 1, 0, {})
        except Exception as ex:
            self.logger.exception("Failed to send Off command")
            return False
        return True


def main():
    optParser = OptionParser()
    optParser.add_option(
        "-t",
        "--timeout",
        action="store",
        dest="testTimeout",
        default=75,
        type='int',
        help="The program will return with timeout after specified seconds.",
        metavar="<timeout-second>",
    )
    optParser.add_option(
        "-a",
        "--address",
        action="store",
        dest="deviceAddress",
        default='',
        type='str',
        help="Address of the device",
        metavar="<device-addr>",
    )

    (options, remainingArgs) = optParser.parse_args(sys.argv[1:])

    timeoutTicker = TestTimeout(options.testTimeout)
    timeoutTicker.start()

    test = MobileDeviceTests(112233)

    FailIfNot(test.TestKeyExchange(options.deviceAddress,
                                   20202021, 1), "Failed to finish key exchange")
    FailIfNot(test.TestNetworkCommissioning(1), "Failed to finish network commissioning")
    FailIfNot(test.TestOnOffCluster(1), "Failed to test on off cluster")

    timeoutTicker.stop()

    logger.info("Test finished")

    # TODO: Python device controller cannot be shutdown clean sometimes and will block on AsyncDNSResolverSockets shutdown.
    # Call os._exit(0) to force close it.
    os._exit(0)


if __name__ == "__main__":
    main()
