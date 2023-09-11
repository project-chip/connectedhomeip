/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <ble/BleUUID.h>

#import <CoreBluetooth/CoreBluetooth.h>

@interface UUIDHelper : NSObject
+ (CBUUID *)GetShortestServiceUUID:(const chip::Ble::ChipBleUUID *)svcId;
@end
