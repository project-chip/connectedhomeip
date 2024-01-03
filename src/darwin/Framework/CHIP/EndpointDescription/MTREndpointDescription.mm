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
#import <Matter/MTREndpointDescription.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>

using namespace chip;

@implementation MTREndpointDescription {
    NSArray<MTRDeviceType *> * _deviceTypes;
    NSArray<MTRAccessGrant *> * _accessGrants;
    NSArray<MTRServerClusterDescription *> * _serverClusters;
}

- (nullable instancetype)initWithEndpointID:(NSNumber *)endpointID deviceTypes:(NSArray<MTRDeviceType *> *)deviceTypes error:(NSError * __autoreleasing *)error
{
    auto endpointIDValue = endpointID.unsignedLongLongValue;
    if (!CanCastTo<EndpointId>(endpointIDValue)) {
        MTR_LOG_ERROR("MTREndpointDescription provided too-large endpoint ID: 0x%llx", endpointIDValue);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    auto id = static_cast<EndpointId>(endpointIDValue);
    if (!IsValidEndpointId(id)) {
        MTR_LOG_ERROR("MTREndpointDescription provided invalid endpoint ID: 0x%" PRIx16, id);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    if (id == kRootEndpointId) {
        // We don't allow this; we use that endpoint for our own purposes in
        // Matter.framework.
        MTR_LOG_ERROR("MTREndpointDescription provided invalid endpoint ID: 0x%" PRIx16, id);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    if (deviceTypes.count == 0) {
        MTR_LOG_ERROR("MTREndpointDescription needs a non-empty list of device types");
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    auto * deviceTypesCopy = [[NSArray alloc] initWithArray:deviceTypes copyItems:YES];
    return [self initWithEndpointID:[endpointID copy] deviceTypes:deviceTypesCopy autoEnable:YES];
}

- (NSArray<MTRDeviceType *> *)deviceTypes
{
    // Make a deep copy.
    return [[NSArray alloc] initWithArray:_deviceTypes copyItems:YES];
}

- (NSArray<MTRAccessGrant *> *)accessGrants
{
    // Make a deep copy.
    return [[NSArray alloc] initWithArray:_accessGrants copyItems:YES];
}

- (void)setAccessGrants:(NSArray *)accessGrants
{
    // Make a deep copy.
    _accessGrants = [[NSArray alloc] initWithArray:accessGrants copyItems:YES];
}

- (NSArray<MTRAccessGrant *> *)serverClusters
{
    // Make a deep copy.
    return [[NSArray alloc] initWithArray:_serverClusters copyItems:YES];
}

- (void)setServerClusters:(NSArray *)serverClusters
{
    // Make a deep copy.
    _serverClusters = [[NSArray alloc] initWithArray:serverClusters copyItems:YES];
}

// initWithEndpointID:deviceTypes:autoEnable: assumes that the endpoint ID and device types have already been
// validated and, if needed, copied from the input.
- (instancetype)initWithEndpointID:(NSNumber *)endpointID deviceTypes:(NSArray<MTRDeviceType *> *)deviceTypes autoEnable:(BOOL)autoEnable
{
    if (!(self = [super init])) {
        return nil;
    }

    _endpointID = endpointID;
    _deviceTypes = deviceTypes;
    _accessGrants = @[];
    _serverClusters = @[];
    _autoEnable = autoEnable;
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    // deviceTypes getter copies.
    auto other = [[MTREndpointDescription alloc] initWithEndpointID:[_endpointID copy] deviceTypes:self.deviceTypes autoEnable:_autoEnable];
    other.accessGrants = _accessGrants; // Setter copies
    other.serverClusters = _serverClusters; // Setter copies
    return other;
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    MTREndpointDescription * other = object;

    return [_endpointID isEqual:other.endpointID] && [_deviceTypes isEqual:other.deviceTypes] && [_accessGrants isEqual:other.accessGrants] && [_serverClusters isEqual:other.serverClusters] && _autoEnable == other.autoEnable;
}

- (NSUInteger)hash
{
    return _endpointID.unsignedShortValue ^ [_deviceTypes hash] ^ [_accessGrants hash] ^ [_serverClusters hash] ^ _autoEnable;
}

@end
