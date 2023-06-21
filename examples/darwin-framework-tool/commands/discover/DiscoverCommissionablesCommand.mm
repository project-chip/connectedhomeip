/*
 *   Copyright (c) 2023 Project CHIP Authors
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
#include "DiscoverCommissionablesCommand.h"

NSMutableArray * gDiscoveredDevices = [[NSMutableArray alloc] init];
auto gDispatchQueue = dispatch_queue_create("com.chip.discover", DISPATCH_QUEUE_SERIAL);

@interface DeviceScannerDelegate : NSObject <MTRCommissionableBrowserDelegate>
- (void)didDiscoverCommissionable:(MTRCommissionableBrowserResult *)device;
- (void)commissionableUnavailable:(MTRCommissionableBrowserResult *)device;
@end

@implementation DeviceScannerDelegate
- (void)didDiscoverCommissionable:(MTRCommissionableBrowserResult *)device
{
    auto serviceName = device.serviceName;
    auto vendorId = device.vendorId;
    auto productId = device.productId;
    auto discriminator = device.discriminator;
    [gDiscoveredDevices addObject:device];

    NSLog(@"Found Device (%@) with discriminator: %@ (vendor: %@, product: %@)", serviceName, discriminator, vendorId, productId);
}

- (void)commissionableUnavailable:(MTRCommissionableBrowserResult *)device
{
    auto serviceName = device.serviceName;
    auto vendorId = device.vendorId;
    auto productId = device.productId;
    auto discriminator = device.discriminator;
    [gDiscoveredDevices removeObjectIdenticalTo:device];

    NSLog(@"Removed Device (%@) with discriminator: %@ (vendor: %@, product: %@)", serviceName, discriminator, vendorId, productId);
}
@end

CHIP_ERROR DiscoverCommissionablesStartCommand::RunCommand()
{
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);

    dispatch_sync(gDispatchQueue, ^{
        [gDiscoveredDevices removeAllObjects];
    });

    auto delegate = [[DeviceScannerDelegate alloc] init];
    auto success = [CurrentCommissioner() startScan:delegate queue:gDispatchQueue];
    VerifyOrReturnError(success, CHIP_ERROR_INTERNAL);

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoverCommissionablesStopCommand::RunCommand()
{
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);

    auto success = [CurrentCommissioner() stopScan];
    VerifyOrReturnError(success, CHIP_ERROR_INTERNAL);

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoverCommissionablesListCommand::RunCommand()
{
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);

    dispatch_sync(gDispatchQueue, ^{
        auto resultsCount = [gDiscoveredDevices count];
        VerifyOrReturn(resultsCount > 0, ChipLogProgress(chipTool, "No device discovered."));

        uint16_t index = 0;
        for (id device in gDiscoveredDevices) {
            auto serviceName = [device serviceName];
            auto vendorId = [device vendorId];
            auto productId = [device productId];
            auto discriminator = [device discriminator];

            NSLog(
                @"\t %u %@ - Discriminator: %@ - Vendor: %@ - Product: %@", index, serviceName, discriminator, vendorId, productId);

            index++;
        }
    });

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}
