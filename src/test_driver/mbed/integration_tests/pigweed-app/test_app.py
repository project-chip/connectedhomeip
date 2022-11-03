# SPDX-FileCopyrightText: (c) 2009-2021 Arm Limited
# SPDX-License-Identifier: Apache-2.0

import pytest
from echo_service import echo_pb2

from common.pigweed_client import PigweedClient

RPC_PROTOS = [echo_pb2]
PW_ECHO_TEST_MESSAGE = "Test_message"


@pytest.mark.smoketest
def test_smoke_test(device):
    device.reset(duration=1)
    ret = device.wait_for_output("Mbed pigweed-app example application start")
    assert ret != None and len(ret) > 0
    ret = device.wait_for_output("Mbed pigweed-app example application run")
    assert ret != None and len(ret) > 0


def test_echo(device):
    pw_client = PigweedClient(device, RPC_PROTOS)
    status, payload = pw_client.rpcs.pw.rpc.EchoService.Echo(
        msg=PW_ECHO_TEST_MESSAGE)
    assert status.ok() == True
    assert payload.msg == PW_ECHO_TEST_MESSAGE
