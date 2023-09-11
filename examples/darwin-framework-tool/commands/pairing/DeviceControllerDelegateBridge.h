/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
#pragma once
#include "PairingCommandBridge.h"

#import <Matter/Matter.h>

@class MTRDeviceController;

@interface CHIPToolDeviceControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property PairingCommandBridge * commandBridge;
@property chip::NodeId deviceID;
@property MTRDeviceController * commissioner;
@property MTRCommissioningParameters * params;

- (void)controller:(MTRDeviceController *)controller statusUpdate:(MTRCommissioningStatus)status;
- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError *)error;
- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError *)error;

@end
