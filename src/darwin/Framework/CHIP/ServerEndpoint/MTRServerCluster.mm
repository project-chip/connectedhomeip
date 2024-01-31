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
    /**
     * The access grants our API consumer can manipulate directly.  Never
     * touched on the Matter queue.
     */
    NSMutableSet<MTRAccessGrant *> * _accessGrants;
    NSMutableArray<MTRServerAttribute *> * _attributes;
    MTRDeviceController * __weak _deviceController;
}

- (nullable instancetype)initWithClusterID:(NSNumber *)clusterID revision:(NSNumber *)revision
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
        MTR_LOG_ERROR("Should be using newDescriptorCluster to initialize an MTRServerCluster for Descriptor");
        return nil;
    }

    auto revisionValue = revision.unsignedLongLongValue;
    if (revisionValue < 1 || revisionValue > 0xFFFF) {
        MTR_LOG_ERROR("MTRServerCluster provided invalid cluster revision: 0x%llx", revisionValue);
        return nil;
    }

    return [self initInternalWithClusterID:clusterID revision:revision accessGrants:[NSSet set] attributes:@[]];
}

+ (MTRServerCluster *)newDescriptorCluster
{
    return [[MTRServerCluster alloc] initInternalWithClusterID:@(MTRClusterIDTypeDescriptorID) revision:@(app::Clusters::Descriptor::kClusterRevision) accessGrants:[NSSet set] attributes:@[]];
}

- (instancetype)initInternalWithClusterID:(NSNumber *)clusterID revision:(NSNumber *)revision accessGrants:(NSSet *)accessGrants attributes:(NSArray *)attributes
{
    if (!(self = [super init])) {
        return nil;
    }

    _clusterID = [clusterID copy];
    _clusterRevision = [revision copy];
    _accessGrants = [[NSMutableSet alloc] init];
    _attributes = [[NSMutableArray alloc] init];
    _matterAccessGrants = [NSSet set];
    _parentEndpoint = kInvalidEndpointId;

    for (MTRAccessGrant * grant in accessGrants) {
        // Since our state is MTRServerEndpointStateInitializing, we know this
        // will succeed.
        [self addAccessGrant:grant];
    }

    for (MTRServerAttribute * attr in attributes) {
        // Since our state is MTRServerEndpointStateInitializing and the initial
        // attribute array was valid, we know this will succeed.
        [self addAttribute:attr];
    }

    return self;
}

- (void)updateMatterAccessGrants
{
    MTRDeviceController * deviceController = _deviceController;
    if (deviceController == nil) {
        // _matterAccessGrants will be updated when we get bound to a controller.
        return;
    }

    NSSet * grants = [_accessGrants copy];
    [deviceController asyncDispatchToMatterQueue:^{
        self->_matterAccessGrants = grants;
    }
                                    errorHandler:nil];
}

- (void)addAccessGrant:(MTRAccessGrant *)accessGrant
{
    [_accessGrants addObject:accessGrant];

    [self updateMatterAccessGrants];
}

- (void)removeAccessGrant:(MTRAccessGrant *)accessGrant;
{
    [_accessGrants removeObject:accessGrant];

    [self updateMatterAccessGrants];
}

- (BOOL)addAttribute:(MTRServerAttribute *)attribute
{
    MTRDeviceController * deviceController = _deviceController;
    if (deviceController != nil) {
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
    MTRDeviceController * existingController = _deviceController;
    if (existingController != nil) {
#if MTR_PER_CONTROLLER_STORAGE_ENABLED
        MTR_LOG_ERROR("Cannot associate MTRServerCluster with controller %@; already associated with controller %@",
            controller.uniqueIdentifier, existingController.uniqueIdentifier);
#else
        MTR_LOG_ERROR("Cannot associate MTRServerCluster with controller; already associated with a different controller");
#endif
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
    _deviceController = controller;

    return YES;
}

- (void)invalidate
{
    for (MTRServerAttribute * attr in _attributes) {
        [attr invalidate];
    }

    _deviceController = nil;
}

- (NSArray<MTRAccessGrant *> *)accessGrants
{
    return [_accessGrants allObjects];
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

@end
