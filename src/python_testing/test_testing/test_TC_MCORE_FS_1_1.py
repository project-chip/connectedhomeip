#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2024 Project CHIP Authors
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

import asyncio
import base64
import os
import sys
import typing
from pathlib import Path

import chip.clusters as Clusters
import click
from chip import ChipDeviceCtrl
from chip.clusters import Attribute
from chip.interaction_model import InteractionModelError, Status
from chip.testing.runner import AsyncMock, MockTestRunner

try:
    from chip.testing.matter_testing import MatterTestConfig, get_default_paa_trust_store, run_tests_no_exit
except ImportError:
    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from chip.testing.matter_testing import MatterTestConfig, get_default_paa_trust_store, run_tests_no_exit

invoke_call_count = 0
event_call_count = 0


def dynamic_invoke_return(*args, **argv):
    ''' Returns the response to a mocked SendCommand call.
    '''
    global invoke_call_count
    invoke_call_count += 1

    # passcode 20202024
    reverse_open = Clusters.CommissionerControl.Commands.ReverseOpenCommissioningWindow(commissioningTimeout=30,
                                                                                        PAKEPasscodeVerifier=b"+w1qZQR05Zn0bc2LDyNaDAhsrhDS5iRHPTN10+EmNx8E2OpIPC4SjWRDQVOgqcbnXdYMlpiZ168xLBqn1fx9659gGK/7f9Yc6GxpoJH8kwAUYAYyLGsYeEBt1kL6kpXjgA==",
                                                                                        discriminator=2222, iterations=10000, salt=base64.b64encode(bytes('SaltyMcSalterson', 'utf-8')))

    print(f'invoke call {invoke_call_count}')
    if invoke_call_count == 1:  # Commission node with no prior request, return failure - step 5
        return None
    elif invoke_call_count == 2:  # Commission node over pase - return unsupported access - step 7
        return reverse_open
    else:
        raise InteractionModelError(Status.Failure)


def dynamic_event_return(*args, **argv):
    ''' Returns the response to a mocked ReadEvent call.
    '''
    global event_call_count
    event_call_count += 1

    if event_call_count == 1:  # reading events, start empty - no events
        return []
    elif event_call_count == 2:  # read event with filter - expect empty
        header = Attribute.EventHeader(EndpointId=0, ClusterId=Clusters.CommissionerControl.id,
                                       EventId=Clusters.CommissionerControl.Events.CommissioningRequestResult.event_id, EventNumber=1)
        data = Clusters.CommissionerControl.Events.CommissioningRequestResult(
            requestID=0x1234567812345678, clientNodeID=112233, statusCode=0)
        result = Attribute.EventReadResult(Header=header, Status=Status.Success, Data=data)
        return [result]
    else:
        raise InteractionModelError(Status.Failure)


def wildcard() -> Attribute.AsyncReadTransaction.ReadResponse:
    ''' Returns the response to a wildcard read.
        For this test, we just need descriptors and a few attributes
        Tree
        EP1 (Aggregator): Descriptor
          - EP2 (Bridged Node): Descriptor, Bridged Device Basic Information, Ecosystem Information
    '''
    cc = Clusters.CommissionerControl
    ei = Clusters.EcosystemInformation
    desc = Clusters.Descriptor
    bdbi = Clusters.BridgedDeviceBasicInformation

    # EP1 is aggregator device type with a commissioner control cluster
    # children - EP2 type bridged node endpoint, ecosystem information, bridged device basic information. Should also have and admin commissioning, but I don't need it for this test.
    desc_ep1 = {desc.Attributes.PartsList: [2], desc.Attributes.ServerList: [
        cc.id], desc.Attributes.DeviceTypeList: [desc.Structs.DeviceTypeStruct(deviceType=0x000E, revision=2)]}
    desc_ep2 = {desc.Attributes.ServerList: [bdbi.id, ei.id], desc.Attributes.DeviceTypeList: [
        desc.Structs.DeviceTypeStruct(deviceType=0x0013, revision=3)]}

    # I'm not filling anything in here, because I don't care. I just care that the cluster exists.
    ei_attrs = {ei.Attributes.AttributeList: [ei.Attributes.DeviceDirectory.attribute_id,
                                              ei.Attributes.LocationDirectory.attribute_id], ei.Attributes.DeviceDirectory: [], ei.Attributes.LocationDirectory: []}

    # This cluster just needs to exist, so I'm just going to throw on the mandatory items for now.
    bdbi_attrs = {bdbi.Attributes.AttributeList: [bdbi.Attributes.Reachable.attribute_id,
                                                  bdbi.Attributes.UniqueID.attribute_id], bdbi.Attributes.Reachable: True, bdbi.Attributes.UniqueID: 'something'}

    cc_attrs = {cc.Attributes.AttributeList: [cc.Attributes.SupportedDeviceCategories], cc.Attributes.AcceptedCommandList: [cc.Commands.RequestCommissioningApproval, cc.Commands.CommissionNode],
                cc.Attributes.GeneratedCommandList: [cc.Commands.RequestCommissioningApproval], cc.Attributes.SupportedDeviceCategories: 1}

    resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
    resp.attributes = {1: {desc: desc_ep1, cc: cc_attrs}, 2: {desc: desc_ep2, ei: ei_attrs, bdbi: bdbi_attrs}}
    return resp


class MyMock(MockTestRunner):
    def run_test_with_mock(self, dynamic_invoke_return: typing.Callable, dynamic_event_return: typing.Callable, read_cache: Attribute.AsyncReadTransaction.ReadResponse, hooks=None):
        ''' Run the test using the Mocked versions of Read, SendCommand, OpenCommissioningWindow, FindOrEstablishPASESession and ReadEvent
            dynamic_invoke_return: Callable function that returns the result of a SendCommand call
                                   Function should return one of
                                   - command response for commands with responses
                                   - None for commands with success results
                                   - raise InteractionModelError for error results
            dynamic_event_return: Callable function that returns the result of a ReadEvent call
                                  Function should return one of
                                  - list of EventReadResult for successful reads
                                  - raise InteractionModelError for error results
            read_cache          : Response to a Read call. For this test, this will be the wildcard read of all teh attributes
            hooks               : Test harness hook object if desired.
        '''
        self.default_controller.Read = AsyncMock(return_value=read_cache)
        self.default_controller.SendCommand = AsyncMock(return_value=None, side_effect=dynamic_invoke_return)
        # It doesn't actually matter what we return here because I'm going to catch the next pase session connection anyway
        params = ChipDeviceCtrl.CommissioningParameters(setupPinCode=0, setupManualCode='', setupQRCode='')
        self.default_controller.OpenCommissioningWindow = AsyncMock(return_value=params)
        self.default_controller.FindOrEstablishPASESession = AsyncMock(return_value=None)
        self.default_controller.ReadEvent = AsyncMock(return_value=[], side_effect=dynamic_event_return)

        with asyncio.Runner() as runner:
            return run_tests_no_exit(self.test_class, self.config, runner.get_loop(),
                                     hooks, self.default_controller, self.stack)


@click.command()
@click.argument('th_server_app', type=click.Path(exists=True))
def main(th_server_app: str):
    root = os.path.abspath(os.path.join(Path(__file__).resolve().parent, '..', '..', '..'))
    print(f'root = {root}')
    paa_path = get_default_paa_trust_store(root)
    print(f'paa = {paa_path}')

    pics = {"PICS_SDK_CI_ONLY": True}
    test_runner = MyMock(Path(__file__).parent / '../TC_MCORE_FS_1_1.py',
                         'TC_MCORE_FS_1_1', 'test_TC_MCORE_FS_1_1', paa_trust_store_path=paa_path, pics=pics)
    config = MatterTestConfig()
    config.user_params = {'th_server_app_path': th_server_app}
    test_runner.set_test_config(config)

    test_runner.run_test_with_mock(dynamic_invoke_return, dynamic_event_return, wildcard())
    test_runner.Shutdown()


if __name__ == "__main__":
    sys.exit(main())
