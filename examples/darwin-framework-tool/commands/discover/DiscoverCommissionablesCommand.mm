/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#import <Matter/Matter.h>
#include "DiscoverCommissionablesCommand.h"
#include "DeviceDiscoveryDelegateBridge.h"
#include <lib/support/BytesToHex.h>

using namespace ::chip;

CHIP_ERROR DiscoverCommissionablesCommand::RunCommand()
{
    SetUpDeviceControllerDelegate();
    auto * controller = CurrentCommissioner();
    [controller discoverCommissionableNodes];
    return CHIP_NO_ERROR;
}

void DiscoverCommissionablesCommand::SetUpDeviceControllerDelegate()
{
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.discovery", DISPATCH_QUEUE_SERIAL);
    CHIPToolDeviceDiscoveryDelegate * deviceDiscoveryDelegate = [[CHIPToolDeviceDiscoveryDelegate alloc] init];
    MTRCommissioningParameters * params = [[MTRCommissioningParameters alloc] init];
    MTRDeviceController * commissioner = CurrentCommissioner();

    [deviceDiscoveryDelegate setParams:params];
    [deviceDiscoveryDelegate setCommissioner:commissioner];

    [commissioner setDeviceDiscoveryDelegate:deviceDiscoveryDelegate queue:callbackQueue];
}
