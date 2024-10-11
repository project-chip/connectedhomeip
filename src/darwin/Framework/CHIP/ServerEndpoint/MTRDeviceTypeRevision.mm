/**
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

#import "MTRDefines_Internal.h"
#import "MTRLogging_Internal.h"
#import <Matter/MTRDeviceTypeRevision.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>

using namespace chip;

MTR_DIRECT_MEMBERS
@implementation MTRDeviceTypeRevision

- (nullable instancetype)initWithDeviceTypeID:(NSNumber *)deviceTypeID revision:(NSNumber *)revision
{
    auto deviceTypeIDValue = deviceTypeID.unsignedLongLongValue;
    if (!CanCastTo<DeviceTypeId>(deviceTypeIDValue)) {
        MTR_LOG_ERROR("MTRDeviceTypeRevision provided too-large device type ID: 0x%llx", deviceTypeIDValue);
        return nil;
    }

    auto id = static_cast<DeviceTypeId>(deviceTypeIDValue);
    if (!IsValidDeviceTypeId(id)) {
        MTR_LOG_ERROR("MTRDeviceTypeRevision provided invalid device type ID: 0x%" PRIx32, id);
        return nil;
    }

    auto revisionValue = revision.unsignedLongLongValue;
    if (!CanCastTo<uint16_t>(revisionValue) || revisionValue < 1) {
        MTR_LOG_ERROR("MTRDeviceTypeRevision provided invalid device type revision: 0x%llx", revisionValue);
        return nil;
    }

    return [self initInternalWithDeviceTypeID:[deviceTypeID copy] revision:[revision copy]];
}

// initInternalWithDeviceTypeID:revision assumes that the device type ID and device
// revision have already been validated and, if needed, copied from the input.
- (instancetype)initInternalWithDeviceTypeID:(NSNumber *)deviceTypeID revision:(NSNumber *)revision
{
    if (!(self = [super init])) {
        return nil;
    }

    _deviceTypeID = deviceTypeID;
    _deviceTypeRevision = revision;
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    // We have no mutable state.
    return self;
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    MTRDeviceTypeRevision * other = object;

    return [_deviceTypeID isEqual:other.deviceTypeID] && [_deviceTypeRevision isEqual:other.deviceTypeRevision];
}

- (NSUInteger)hash
{
    return _deviceTypeID.unsignedLongValue ^ _deviceTypeRevision.unsignedShortValue;
}

@end
