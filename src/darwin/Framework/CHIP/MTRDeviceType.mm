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

#import "MTRDefines_Internal.h"
#import "MTRDeviceTypeMetadata.h"
#import "MTRLogging_Internal.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

using namespace chip;

MTR_DIRECT_MEMBERS
@implementation MTRDeviceType

- (instancetype)initWithDeviceTypeID:(NSNumber *)id name:(NSString *)name isUtility:(BOOL)isUtility
{
    self = [super init];
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

    auto * name = [[NSString alloc] initWithBytesNoCopy:(void *) deviceTypeData->name
                                                 length:strlen(deviceTypeData->name)
                                               encoding:NSUTF8StringEncoding
                                           freeWhenDone:NO];

    return [[MTRDeviceType alloc] initWithDeviceTypeID:deviceTypeID
                                                  name:name
                                             isUtility:(deviceTypeData->deviceClass != MTRDeviceTypeClass::Simple)];
}

- (id)copyWithZone:(NSZone *)zone
{
    return self; // immutable
}

- (NSUInteger)hash
{
    return _id.hash;
}

- (BOOL)isEqual:(id)object
{
    VerifyOrReturnValue([object class] == [self class], NO);
    MTRDeviceType * other = object;
    return [_id isEqual:other->_id];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ 0x%" PRIx32 " (%@)>",
                     self.class, _id.unsignedIntValue, _name];
}

@end
