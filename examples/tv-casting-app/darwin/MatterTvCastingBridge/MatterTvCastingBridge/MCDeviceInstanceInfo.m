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

#import "MCDeviceInstanceInfo.h"
#import <Foundation/Foundation.h>

static const NSUInteger kMaxVendorNameLength = 32;
static const NSUInteger kMaxProductNameLength = 32;
static const NSUInteger kMaxSerialNumberLength = 32;
static const NSUInteger kMaxHardwareVersionStringLength = 64;

@implementation MCDeviceInstanceInfo

- (instancetype _Nullable)initWithVendorName:(NSString * _Nullable)vendorName
                                 productName:(NSString * _Nullable)productName
                                    vendorId:(NSNumber * _Nullable)vendorId
                                   productId:(NSNumber * _Nullable)productId
                                serialNumber:(NSString * _Nullable)serialNumber
                             hardwareVersion:(NSNumber * _Nullable)hardwareVersion
                       hardwareVersionString:(NSString * _Nullable)hardwareVersionString
{
    if (vendorName != nil && vendorName.length > kMaxVendorNameLength) {
        return nil;
    }
    if (productName != nil && productName.length > kMaxProductNameLength) {
        return nil;
    }
    if (serialNumber != nil && serialNumber.length > kMaxSerialNumberLength) {
        return nil;
    }
    if (hardwareVersionString != nil && hardwareVersionString.length > kMaxHardwareVersionStringLength) {
        return nil;
    }

    if (self = [super init]) {
        _vendorName = vendorName;
        _productName = productName;
        _vendorId = vendorId;
        _productId = productId;
        _serialNumber = serialNumber;
        _hardwareVersion = hardwareVersion;
        _hardwareVersionString = hardwareVersionString;
    }
    return self;
}

@end
