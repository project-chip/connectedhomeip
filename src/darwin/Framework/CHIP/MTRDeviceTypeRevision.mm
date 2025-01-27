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

#import "MTRDeviceTypeRevision.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceType.h"
#import "MTRLogging_Internal.h"
#import "MTRStructsObjc.h"

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

NS_ASSUME_NONNULL_BEGIN

using namespace chip;

MTR_DIRECT_MEMBERS
@implementation MTRDeviceTypeRevision

- (nullable instancetype)initWithDeviceTypeID:(NSNumber *)deviceTypeID revision:(NSNumber *)revision
{
    VerifyOrReturnValue(deviceTypeID != nil, nil);
    VerifyOrReturnValue(revision != nil, nil);

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

- (nullable instancetype)initWithDeviceTypeStruct:(MTRDescriptorClusterDeviceTypeStruct *)deviceTypeStruct
{
    return [self initWithDeviceTypeID:deviceTypeStruct.deviceType revision:deviceTypeStruct.revision];
}

// initInternalWithDeviceTypeID:revision assumes that the device type ID and device
// revision have already been validated and, if needed, copied from the input.
- (instancetype)initInternalWithDeviceTypeID:(NSNumber *)deviceTypeID revision:(NSNumber *)revision
{
    self = [super init];
    _deviceTypeID = deviceTypeID;
    _deviceTypeRevision = revision;
    return self;
}

static NSString * const sTypeIdCodingKey = @"ty";
static NSString * const sRevisionCodingKey = @"re";

- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    _deviceTypeID = @(static_cast<DeviceTypeId>([coder decodeInt64ForKey:sTypeIdCodingKey])); // int64_t encompasses uint32_t
    _deviceTypeRevision = @(static_cast<uint16_t>([coder decodeIntegerForKey:sRevisionCodingKey]));
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeInt64:static_cast<DeviceTypeId>(_deviceTypeID.unsignedLongLongValue) forKey:sTypeIdCodingKey];
    [coder encodeInteger:static_cast<uint16_t>(_deviceTypeRevision.unsignedIntegerValue) forKey:sRevisionCodingKey];
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (nullable MTRDeviceType *)typeInformation
{
    return [MTRDeviceType deviceTypeForID:_deviceTypeID];
}

- (id)copyWithZone:(nullable NSZone *)zone
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

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ 0x%" PRIx32 " (%@) rev %d>",
                     self.class, _deviceTypeID.unsignedIntValue,
                     self.typeInformation.name ?: @"???", _deviceTypeRevision.unsignedIntValue];
}

@end

NS_ASSUME_NONNULL_END
