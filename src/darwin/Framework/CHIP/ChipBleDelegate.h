/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file defines ChipBleDelegate interface
 *
 */

#import "CHIPError.h"
#import <CoreBluetooth/CoreBluetooth.h>

@class CHIPDeviceController;

typedef void (^PreparationCompleteHandler)(CHIPDeviceController * dm, CHIP_ERROR err);

@interface ChipBleDelegate : NSObject <CBPeripheralDelegate>

/*
 * \defgroup BlePlatformDelegate
 * @{
 */

/*
 *  @brief Command from BleLayer to subscribe to a characteristic
 */
- (bool)SubscribeCharacteristic:(id)connObj service:(const CBUUID *)svcId characteristic:(const CBUUID *)charId;

/*
 *  @brief Command from BleLayer to unsubscribe a characteristic
 */
- (bool)UnsubscribeCharacteristic:(id)connObj service:(const CBUUID *)svcId characteristic:(const CBUUID *)charId;

/*
 *  @brief Command from BleLayer to close the underlying BLE connection
 */
- (bool)CloseConnection:(id)connObj;

/*
 *  @brief Command from BleLayer to retrieve the current MTU of the BLE connection
 */
- (uint16_t)GetMTU:(id)connObj;

/*
 *  @brief Command from BleLayer to send indication
 */
- (bool)SendIndication:(id)connObj service:(const CBUUID *)svcId characteristic:(const CBUUID *)charId data:(const NSData *)buf;

/*
 *  @brief Command from BleLayer to send write request
 */
- (bool)SendWriteRequest:(id)connObj service:(const CBUUID *)svcId characteristic:(const CBUUID *)charId data:(const NSData *)buf;

/*
 *  @brief Command from BleLayer to send read request
 */
- (bool)SendReadRequest:(id)connObj service:(const CBUUID *)svcId characteristic:(const CBUUID *)charId data:(const NSData *)buf;

/*
 *  @brief Command from BleLayer to send read response
 */
- (bool)SendReadResponse:(id)connObj
          requestContext:(id)readContext
                 service:(const CBUUID *)svcId
          characteristic:(const CBUUID *)charId;

/*
 * @}
 */

/*
 * \defgroup BleApplicationDelegate
 * @{
 */

/*
 *  @brief Notification from BleLayer when the BLE connection is no longer needed by CHIP
 */
- (void)NotifyChipConnectionClosed:(id)connObj;
/*
 * @}
 */

/*
 *  @brief Create an instance which fails all BLE activity
 */
- (instancetype)initDummyDelegate;

/*
 *  @brief Create an instance which drives all BLE activity in given CoreBluetooth work queue
 */
- (instancetype)init:(dispatch_queue_t)cbWorkQueue;

/*
 *  @brief Returns true if the CBPeripheral passed in is under management
 */
- (bool)isPeripheralValid:(CBPeripheral *)peripheral;

/*
 *  @brief Command from ChipDeviceController to prepare the CBPeripheral contained in the ChipDeviceController for WoBLE
 */

- (void)prepareNewBleConnection:(CHIPDeviceController *)dm;

/*
 *  @brief Send async notification to BleLayer when the underlying BLE connection is broken
 */
- (void)notifyBleDisconnected:(CBPeripheral *)peripheral;

/*
 *  @brief Command from ChipDeviceController to BleLayer so this connection is no longer managed
 */
- (void)forceBleDisconnect_Sync:(CBPeripheral *)peripheral;

@end
