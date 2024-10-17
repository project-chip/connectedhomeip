/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#import <Matter/MTRDeviceType.h>

#import "MTRDeviceTypeMetadata.h"
#import "MTRLogging_Internal.h"

#include <lib/support/SafeInt.h>

using namespace chip;

@implementation MTRDeviceType

- (nullable instancetype)initWithDeviceTypeID:(NSNumber *)id name:(NSString *)name isUtility:(BOOL)isUtility
{
    if (!(self = [super init])) {
        return nil;
    }

    _id = id;
    _name = name;
    _isUtility = isUtility;
    return self;
}

+ (nullable MTRDeviceType *)deviceTypeForID:(NSNumber *)deviceTypeID
{
    if (!CanCastTo<DeviceTypeId>(deviceTypeID.unsignedLongLongValue)) {
        MTR_LOG_ERROR("Invalid device type ID: 0x%llx", deviceTypeID.unsignedLongLongValue);
        return nil;
    }

    auto * deviceTypeData = MTRDeviceTypeDataForID(static_cast<DeviceTypeId>(deviceTypeID.unsignedLongLongValue));
    if (!deviceTypeData) {
        return nil;
    }

    return [[MTRDeviceType alloc]
        initWithDeviceTypeID:deviceTypeID
                        name:[NSString stringWithUTF8String:deviceTypeData->name]
                   isUtility:(deviceTypeData->deviceClass != MTRDeviceTypeClass::Simple)];
}

@end
