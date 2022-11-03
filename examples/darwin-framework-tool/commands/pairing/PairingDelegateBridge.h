/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include "PairingCommandBridge.h"

#import <Matter/Matter.h>

@interface CHIPToolPairingDelegate : NSObject <MTRDevicePairingDelegate>
@property PairingCommandBridge * commandBridge;
@property chip::NodeId deviceID;
@property MTRDeviceController * commissioner;
@property MTRCommissioningParameters * params;

- (void)onPairingComplete:(NSError *)error;
- (void)onPairingDeleted:(NSError *)error;
- (void)onCommissioningComplete:(NSError *)error;

@end
