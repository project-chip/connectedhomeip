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

#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import <Matter/MTRDeviceType.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>

using namespace chip;

@implementation MTRDeviceType

- (nullable instancetype)initWithDeviceTypeID:(NSNumber *)deviceTypeID revision:(NSNumber *)revision error:(NSError * __autoreleasing *)error
{
    auto deviceTypeIDValue = deviceTypeID.unsignedLongLongValue;
    if (!CanCastTo<DeviceTypeId>(deviceTypeIDValue)) {
        MTR_LOG_ERROR("MTRDeviceType provided too-large device type ID: 0x%llx", deviceTypeIDValue);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    auto id = static_cast<DeviceTypeId>(deviceTypeIDValue);
    if (!IsValidDeviceTypeId(id)) {
        MTR_LOG_ERROR("MTRDeviceType provided invalid device type ID: 0x%" PRIx32, id);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    auto revisionValue = revision.unsignedLongLongValue;
    if (!CanCastTo<uint16_t>(revisionValue) || revisionValue < 1) {
        MTR_LOG_ERROR("MTRDeviceType provided invalid device type revision: 0x%llx", revisionValue);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    return [self initWithDeviceTypeID:[deviceTypeID copy] revision:[revision copy]];
}

// initWithDeviceTypeID:revision assumes that the device type ID and device
// revision have already been validated and, if needed, copied from the input.
- (instancetype)initWithDeviceTypeID:(NSNumber *)deviceTypeID revision:(NSNumber *)revision
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
    return [[MTRDeviceType alloc] initWithDeviceTypeID:[_deviceTypeID copy] revision:[_deviceTypeRevision copy]];
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    MTRDeviceType * other = object;

    return [_deviceTypeID isEqual:other.deviceTypeID] && [_deviceTypeRevision isEqual:other.deviceTypeRevision];
}

- (NSUInteger)hash
{
    return _deviceTypeID.unsignedLongValue ^ _deviceTypeRevision.unsignedShortValue;
}

@end
