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
#import <Matter/MTRClusterConstants.h>
#import <Matter/MTRServerClusterDescription.h>

#include <app/clusters/descriptor/descriptor.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>

using namespace chip;

@implementation MTRServerClusterDescription {
    NSArray<MTRAccessGrant *> * _accessGrants;
    NSArray<MTRAttributeDescription *> * _attributes;
}

- (nullable instancetype)initWithClusterID:(NSNumber *)clusterID clusterRevision:(NSNumber *)clusterRevision error:(NSError * __autoreleasing *)error
{
    auto clusterIDValue = clusterID.unsignedLongLongValue;
    if (!CanCastTo<ClusterId>(clusterIDValue)) {
        MTR_LOG_ERROR("MTRServerClusterDescription provided too-large cluster ID: 0x%llx", clusterIDValue);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    auto id = static_cast<ClusterId>(clusterIDValue);
    if (!IsValidClusterId(id)) {
        MTR_LOG_ERROR("MTRServerClusterDescription provided invalid cluster ID: 0x%" PRIx32, id);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    if (id == MTRClusterIDTypeDescriptorID) {
        MTR_LOG_ERROR("Should be using initDescriptorCluster to initialize an MTRServerClusterDescription for Descriptor");
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    auto revisionValue = clusterRevision.unsignedLongLongValue;
    if (revisionValue < 1 || revisionValue > 0xFFFF) {
        MTR_LOG_ERROR("MTRServerClusterDescription provided invalid cluster revision: 0x%llx", revisionValue);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    return [self initWithClusterID:[clusterID copy] clusterRevision:[clusterRevision copy]];
}

- (instancetype)initDescriptorCluster
{
    return [self initWithClusterID:@(MTRClusterIDTypeDescriptorID) clusterRevision:@(app::Clusters::Descriptor::kClusterRevision)];
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

- (NSArray<MTRAttributeDescription *> *)attributes
{
    // Make a deep copy.
    return [[NSArray alloc] initWithArray:_attributes copyItems:YES];
}

- (void)setAttributes:(NSArray<MTRAttributeDescription *> *)attributes
{
    // Make a deep copy.
    _attributes = [[NSArray alloc] initWithArray:attributes copyItems:YES];
}

// initWithClusterID:clusterRevision: assumes that the cluster ID and revision have already been
// validated and, if needed, copied from the input.
- (instancetype)initWithClusterID:(NSNumber *)clusterID clusterRevision:(NSNumber *)clusterRevision
{
    if (!(self = [super init])) {
        return nil;
    }

    _clusterID = clusterID;
    _clusterRevision = clusterRevision;
    _accessGrants = @[];
    _attributes = @[];
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    auto other = [[MTRServerClusterDescription alloc] initWithClusterID:[_clusterID copy] clusterRevision:[_clusterRevision copy]];
    other.accessGrants = _accessGrants; // Setter copies
    other.attributes = _attributes; // Setter copies
    return other;
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    MTRServerClusterDescription * other = object;

    return [_clusterID isEqual:other.clusterID] && [_clusterRevision isEqual:other.clusterRevision] && [_accessGrants isEqual:other.accessGrants] && [_attributes isEqual:other.attributes];
}

- (NSUInteger)hash
{
    return _clusterID.unsignedLongValue ^ _clusterRevision.unsignedShortValue ^ [_accessGrants hash] ^ [_attributes hash];
}

@end
