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

import os
import sys
import pathlib
import typing

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.clusters import Attribute
from MockTestRunner import MockTestRunner, AsyncMock
from chip.interaction_model import InteractionModelError, Status


try:
    from matter_testing_support import get_default_paa_trust_store, run_tests_no_exit
except ImportError:
    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_testing_support import get_default_paa_trust_store, run_tests_no_exit

call_count = 0

def dynamic_return(*args, **argv):
    print("using Mock invoke")
    global call_count
    call_count += 1

    if call_count == 1: # Commission node with no prior request, return failure
        raise InteractionModelError(status=Status.Failure)
    elif call_count == 2: # Commission node over pase - return unsupported access
        raise InteractionModelError(status=Status.UnsupportedAccess)
    elif call_count == 3: # request commissioning approval over pase - return unsupported access
        raise InteractionModelError(status=Status.UnsupportedAccess)
    elif call_count == 4: # good RequestCommissioningApproval over CASE
        return None
    else:
        raise InteractionModelError(Status.Failure)

def wildcard() -> Attribute.AsyncReadTransaction.ReadResponse:
    cc = Clusters.CommissionerControl
    ei = Clusters.EcosystemInformation
    desc = Clusters.Descriptor
    bdbi = Clusters.BridgedDeviceBasicInformation

    # EP1 is aggregator device type with a commissioner control cluster
    # children - EP2 type bridged node endpoint, ecosystem information, bridged device basic information. Should also have and admin commissioning, but I don't need it for this test.
    desc_ep1 = {desc.Attributes.PartsList: [2], desc.Attributes.ServerList: [cc.id], desc.Attributes.DeviceTypeList: [desc.Structs.DeviceTypeStruct(deviceType=0x000E, revision=2)]}
    desc_ep2 = {desc.Attributes.ServerList: [bdbi.id, ei.id], desc.Attributes.DeviceTypeList: [desc.Structs.DeviceTypeStruct(deviceType=0x0013, revision=3)]}

    # I'm not filling anything in here, because I don't care. I just care that the cluster exists.
    ei_attrs = {ei.Attributes.AttributeList:[ei.Attributes.DeviceDirectory.attribute_id, ei.Attributes.LocationDirectory.attribute_id], ei.Attributes.DeviceDirectory:[], ei.Attributes.LocationDirectory:[]}

    # This cluster just needs to exist, so I'm just going to throw on the mandatory items for now.
    bdbi_attrs = {bdbi.Attributes.AttributeList:[bdbi.Attributes.Reachable.attribute_id, bdbi.Attributes.UniqueID.attribute_id], bdbi.Attributes.Reachable:True, bdbi.Attributes.UniqueID:'something'}

    cc_attrs = {cc.Attributes.AttributeList:[cc.Attributes.SupportedDeviceCategories], cc.Attributes.AcceptedCommandList:[cc.Commands.RequestCommissioningApproval, cc.Commands.CommissionNode],
                cc.Attributes.GeneratedCommandList:[cc.Commands.RequestCommissioningApproval], cc.Attributes.SupportedDeviceCategories:1}

    resp = Attribute.AsyncReadTransaction.ReadResponse({}, [], {})
    resp.attributes = {1: {desc: desc_ep1, cc:cc_attrs}, 2:{desc:desc_ep2, ei:ei_attrs, bdbi:bdbi_attrs}}
    return resp

class MyMock(MockTestRunner):
    # TODO consolidate with above
    def run_test_with_mock(self, dynamic_invoke_return: typing.Callable, read_cache: Attribute.AsyncReadTransaction.ReadResponse, hooks=None):
        ''' Effects is a list of callable functions with *args, **kwargs parameters. It can either throw an InteractionModelException or return the command response.'''
        self.default_controller.Read = AsyncMock(return_value=read_cache)
        self.default_controller.SendCommand = AsyncMock(return_value=None, side_effect=dynamic_invoke_return)
        # It doesn't actually matter what we return here because I'm going to catch the next pase session connection anyway
        params = ChipDeviceCtrl.CommissioningParameters(setupPinCode=0, setupManualCode='', setupQRCode='')
        self.default_controller.OpenCommissioningWindow = AsyncMock(return_value=params)
        self.default_controller.FindOrEstablishPASESession = AsyncMock(return_value=None)

        return run_tests_no_exit(self.test_class, self.config, hooks, self.default_controller, self.stack)

def main():
    root = os.path.abspath(os.path.join(pathlib.Path(__file__).resolve().parent, '..','..','..'))
    print(f'root = {root}')
    paa_path = get_default_paa_trust_store(root)
    print(f'paa = {paa_path}')

    test_runner = MyMock('TC_CCTRL', 'TC_CCTRL', 'test_TC_CCTRL_3_1', 1, paa_trust_store_path=paa_path)

    test_runner.run_test_with_mock(dynamic_return, wildcard())
    test_runner.Shutdown()

if __name__ == "__main__":
    sys.exit(main())
