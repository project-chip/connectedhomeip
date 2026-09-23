/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#import <Foundation/Foundation.h>

#ifndef MCDeviceInstanceInfo_h
#define MCDeviceInstanceInfo_h

/**
 * @brief Pull-based protocol for providing device instance info at runtime.
 *
 * Implement this protocol to supply device instance information fields
 * used during Matter commissioning. The platform layer queries these methods
 * when it needs each value, allowing the app to return dynamic/current data.
 *
 * All methods are optional. If a method is not implemented or returns nil,
 * the platform falls back to compile-time defaults.
 */
@protocol MCDeviceInstanceInfoProvider <NSObject>
@optional

- (NSString * _Nullable)vendorName;
- (NSNumber * _Nullable)vendorId;
- (NSString * _Nullable)productName;
- (NSNumber * _Nullable)productId;
- (NSString * _Nullable)serialNumber;
- (NSNumber * _Nullable)hardwareVersion;
- (NSString * _Nullable)hardwareVersionString;

/**
 * @brief The commissionable device name shown on the casting target's commissioning prompt.
 *
 * On Android, this corresponds to kConfigKey_DeviceName in PreferencesConfigurationManager.
 * The value is written into the platform ConfigurationManager at init time so it is used
 * by UDC and mDNS advertisements.
 */
- (NSString * _Nullable)deviceName;

@end

#endif /* MCDeviceInstanceInfo_h */
