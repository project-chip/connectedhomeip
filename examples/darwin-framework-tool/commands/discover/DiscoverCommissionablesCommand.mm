/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
#include "DiscoverCommissionablesCommand.h"

NSMutableArray * gDiscoveredDevices = [[NSMutableArray alloc] init];
auto gDispatchQueue = dispatch_queue_create("com.chip.discover", DISPATCH_QUEUE_SERIAL);

@interface DeviceScannerDelegate : NSObject <MTRCommissionableBrowserDelegate>
- (void)controller:(MTRDeviceController *)controller didFindCommissionableDevice:(MTRCommissionableBrowserResult *)device;
- (void)controller:(MTRDeviceController *)controller didRemoveCommissionableDevice:(MTRCommissionableBrowserResult *)device;
@end

@implementation DeviceScannerDelegate
- (void)controller:(MTRDeviceController *)controller didFindCommissionableDevice:(MTRCommissionableBrowserResult *)device
{
    auto instanceName = device.instanceName;
    auto vendorId = device.vendorID;
    auto productId = device.productID;
    auto discriminator = device.discriminator;
    [gDiscoveredDevices addObject:device];

    NSLog(@"Found Device (%@) with discriminator: %@ (vendor: %@, product: %@)", instanceName, discriminator, vendorId, productId);
}

- (void)controller:(MTRDeviceController *)controller didRemoveCommissionableDevice:(MTRCommissionableBrowserResult *)device
{
    auto instanceName = device.instanceName;
    auto vendorId = device.vendorID;
    auto productId = device.productID;
    auto discriminator = device.discriminator;
    [gDiscoveredDevices removeObjectIdenticalTo:device];

    NSLog(
        @"Removed Device (%@) with discriminator: %@ (vendor: %@, product: %@)", instanceName, discriminator, vendorId, productId);
}
@end

CHIP_ERROR DiscoverCommissionablesStartCommand::RunCommand()
{
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);

    dispatch_sync(gDispatchQueue, ^{
        [gDiscoveredDevices removeAllObjects];
    });

    auto delegate = [[DeviceScannerDelegate alloc] init];
    auto success = [CurrentCommissioner() startBrowseForCommissionables:delegate queue:gDispatchQueue];
    VerifyOrReturnError(success, CHIP_ERROR_INTERNAL);

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoverCommissionablesStopCommand::RunCommand()
{
    VerifyOrReturnError(IsInteractive(), CHIP_ERROR_INCORRECT_STATE);

    auto success = [CurrentCommissioner() stopBrowseForCommissionables];
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
            auto instanceName = [device instanceName];
            auto vendorId = [device vendorID];
            auto productId = [device productID];
            auto discriminator = [device discriminator];

            NSLog(@"\t %u %@ - Discriminator: %@ - Vendor: %@ - Product: %@", index, instanceName, discriminator, vendorId,
                productId);

            index++;
        }
    });

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}
