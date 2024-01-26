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
#import "MTRDeviceController_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRServerAttribute_Internal.h"
#import "MTRServerCluster_Internal.h"
#import "MTRServerEndpoint_Internal.h"
#import <Matter/MTRClusterConstants.h>
#import <Matter/MTRServerCluster.h>

#include <app/clusters/descriptor/descriptor.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>

using namespace chip;

MTR_DIRECT_MEMBERS
@implementation MTRServerCluster {
    MTRServerEndpointState _state;
    /**
     * The access grants our API consumer can manipulate directly.  Never
     * touched on the Matter queue.
     */
    NSMutableSet<MTRAccessGrant *> * _accessGrants;
    NSMutableArray<MTRServerAttribute *> * _attributes;
    MTRDeviceController * __weak _deviceController;
}

- (nullable instancetype)initWithClusterID:(NSNumber *)clusterID clusterRevision:(NSNumber *)clusterRevision
{
    auto clusterIDValue = clusterID.unsignedLongLongValue;
    if (!CanCastTo<ClusterId>(clusterIDValue)) {
        MTR_LOG_ERROR("MTRServerCluster provided too-large cluster ID: 0x%llx", clusterIDValue);
        return nil;
    }

    auto id = static_cast<ClusterId>(clusterIDValue);
    if (!IsValidClusterId(id)) {
        MTR_LOG_ERROR("MTRServerCluster provided invalid cluster ID: 0x%" PRIx32, id);
        return nil;
    }

    if (id == MTRClusterIDTypeDescriptorID) {
        MTR_LOG_ERROR("Should be using initDescriptorCluster to initialize an MTRServerCluster for Descriptor");
        return nil;
    }

    auto revisionValue = clusterRevision.unsignedLongLongValue;
    if (revisionValue < 1 || revisionValue > 0xFFFF) {
        MTR_LOG_ERROR("MTRServerCluster provided invalid cluster revision: 0x%llx", revisionValue);
        return nil;
    }

    return [self initInternalWithClusterID:clusterID clusterRevision:clusterRevision accessGrants:[NSSet set] attributes:@[]];
}

- (instancetype)initDescriptorCluster
{
    return [self initInternalWithClusterID:@(MTRClusterIDTypeDescriptorID) clusterRevision:@(app::Clusters::Descriptor::kClusterRevision) accessGrants:[NSSet set] attributes:@[]];
}

- (instancetype)initInternalWithClusterID:(NSNumber *)clusterID clusterRevision:(NSNumber *)clusterRevision accessGrants:(NSSet *)accessGrants attributes:(NSArray *)attributes
{
    if (!(self = [super init])) {
        return nil;
    }

    _state = MTRServerEndpointStateInitializing;
    _clusterID = [clusterID copy];
    _clusterRevision = [clusterRevision copy];
    _accessGrants = [[NSMutableSet alloc] init];
    _attributes = [[NSMutableArray alloc] init];
    _matterAccessGrants = [NSSet set];
    _parentEndpoint = kInvalidEndpointId;

    for (MTRAccessGrant * grant in accessGrants) {
        // Since our state is MTRServerEndpointStateInitializing, we know this
        // will succeed.
        [self addAccessGrant:[grant copy]];
    }

    for (MTRServerAttribute * attr in attributes) {
        // Since our state is MTRServerEndpointStateInitializing and the initial
        // attribute array was valid, we know this will succeed.
        [self addAttribute:[attr copy]];
    }

    return self;
}

/**
 * Ensures we are in a valid state to manipulate our Matter-side state.
 */
- (BOOL)ensureState:(NSString *)action
{
    if (_state == MTRServerEndpointStateInitializing) {
        return YES;
    }

    MTRDeviceController * deviceController = _deviceController;
    if (deviceController == nil || !deviceController.running) {
        // Controller went away.  Make sure we are invalidated.
        [self invalidate];
    }

    if (_state == MTRServerEndpointStateDefunct) {
        MTR_LOG_ERROR("Cannot %@ on cluster %llx on no-longer-active endpoint", action, _clusterID.unsignedLongLongValue);
        return NO;
    }

    return YES;
}

- (void)updateMatterAccessGrants
{
    // Must have had ensureState called first.

    if (_state == MTRServerEndpointStateInitializing) {
        // _matterAccessGrants will be updated when we move to the
        // MTRServerEndpointStateActive state.
    } else {
        // Must be non-nil if ensureState already happened.
        MTRDeviceController * deviceController = _deviceController;
        NSSet * grants = [_accessGrants copy];
        [deviceController asyncDispatchToMatterQueue:^{
            self->_matterAccessGrants = grants;
        }
                                        errorHandler:nil];
    }
}

- (BOOL)addAccessGrant:(MTRAccessGrant *)accessGrant
{
    if (![self ensureState:@"add access grants"]) {
        return NO;
    }

    [_accessGrants addObject:accessGrant];

    [self updateMatterAccessGrants];

    return YES;
}

- (BOOL)removeAccessGrant:(MTRAccessGrant *)accessGrant;
{
    if (![self ensureState:@"remove access grants"]) {
        return NO;
    }

    [_accessGrants removeObject:accessGrant];

    [self updateMatterAccessGrants];

    return YES;
}

- (BOOL)addAttribute:(MTRServerAttribute *)attribute
{
    if (_state != MTRServerEndpointStateInitializing) {
        MTR_LOG_ERROR("Cannot add attribute on cluster %llx which is already in use", _clusterID.unsignedLongLongValue);
        return NO;
    }

    if (attribute.parentCluster.mClusterId != kInvalidClusterId) {
        MTR_LOG_ERROR("Cannot add attribute to cluster %llu; already added to cluster %" PRIu32, _clusterID.unsignedLongLongValue, attribute.parentCluster.mClusterId);
        return NO;
    }

    auto attributeID = attribute.attributeID.unsignedLongLongValue;
    if (attributeID == MTRAttributeIDTypeGlobalAttributeAttributeListID || attributeID == MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID || attributeID == MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID || attributeID == MTRAttributeIDTypeGlobalAttributeClusterRevisionID) {
        MTR_LOG_ERROR("Cannot add global attribute %llx on cluster %llx", attributeID, _clusterID.unsignedLongLongValue);
        return NO;
    }

    if ([_clusterID isEqual:@(MTRClusterIDTypeDescriptorID)]) {
        if (attributeID == MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID || attributeID == MTRAttributeIDTypeClusterDescriptorAttributeServerListID || attributeID == MTRAttributeIDTypeClusterDescriptorAttributeClientListID || attributeID == MTRAttributeIDTypeClusterDescriptorAttributePartsListID) {
            MTR_LOG_ERROR("Cannot add attribute with id %llx on descriptor cluster", attributeID);
            return NO;
        }
    }

    for (MTRServerAttribute * existingAttr in _attributes) {
        if (existingAttr.attributeID.unsignedLongLongValue == attributeID) {
            MTR_LOG_ERROR("Cannot add second attribute with ID %llx on cluster %llx", attributeID, _clusterID.unsignedLongLongValue);
            return NO;
        }
    }

    [_attributes addObject:attribute];
    attribute.parentCluster = app::ConcreteClusterPath(_parentEndpoint, static_cast<ClusterId>(_clusterID.unsignedLongLongValue));
    return YES;
}

- (BOOL)associateWithController:(MTRDeviceController *)controller
{
    if (_state != MTRServerEndpointStateInitializing) {
        MTR_LOG_ERROR("Cannot associate MTRServerCluster in state %lu with controller", static_cast<unsigned long>(_state));
        return NO;
    }

    for (MTRServerAttribute * attr in _attributes) {
        if (![attr associateWithController:controller]) {
            return NO;
        }
    }

    // Snapshot _matterAccessGrants now; after this point it will only be
    // updated on the Matter queue.
    _matterAccessGrants = [_accessGrants copy];
    _state = MTRServerEndpointStateActive;
    _deviceController = controller;

    return YES;
}

- (void)invalidate
{
    _state = MTRServerEndpointStateDefunct;

    for (MTRServerAttribute * attr in _attributes) {
        [attr invalidate];
    }

    _deviceController = nil;
}

- (NSSet<MTRAccessGrant *> *)accessGrants
{
    return [_accessGrants copy];
}

- (NSArray<MTRServerAttribute *> *)attributes
{
    return [_attributes copy];
}

- (void)setParentEndpoint:(EndpointId)endpoint
{
    _parentEndpoint = endpoint;
    // Update it on all the attributes, in case the attributes were added to us
    // before we were added to the endpoint.
    for (MTRServerAttribute * attr in _attributes) {
        attr.parentCluster = app::ConcreteClusterPath(endpoint, static_cast<ClusterId>(_clusterID.unsignedLongLongValue));
    }
}

- (id)copyWithZone:(NSZone *)zone
{
    return [[MTRServerCluster alloc] initInternalWithClusterID:_clusterID clusterRevision:_clusterRevision accessGrants:_accessGrants attributes:_attributes];
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    MTRServerCluster * other = object;

    // Note: ignoring _matterAccessGrants, which is just a possibly-delayed
    // snapshot of _accessGrants.
    return [_clusterID isEqual:other.clusterID] && [_clusterRevision isEqual:other.clusterRevision] && [_accessGrants isEqualToSet:other.accessGrants] && [_attributes isEqualToArray:other.attributes];
}

- (NSUInteger)hash
{
    return _clusterID.unsignedLongValue ^ _clusterRevision.unsignedShortValue ^ [_accessGrants hash] ^ [_attributes hash];
}

@end
