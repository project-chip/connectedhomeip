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
#import "MTRServerCluster_Internal.h"
#import "MTRServerEndpoint_Internal.h"
#import <Matter/MTRServerEndpoint.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>

using namespace chip;

MTR_DIRECT_MEMBERS
@implementation MTRServerEndpoint {
    MTRServerEndpointState _state;
    /**
     * The access grants our API consumer can manipulate directly.  Never
     * touched on the Matter queue.
     */
    NSMutableSet<MTRAccessGrant *> * _accessGrants;
    NSMutableArray<MTRServerCluster *> * _serverClusters;
    MTRDeviceController * __weak _deviceController;
}

- (nullable instancetype)initWithEndpointID:(NSNumber *)endpointID deviceTypes:(NSArray<MTRDeviceType *> *)deviceTypes
{
    auto endpointIDValue = endpointID.unsignedLongLongValue;
    if (!CanCastTo<EndpointId>(endpointIDValue)) {
        MTR_LOG_ERROR("MTRServerEndpoint provided too-large endpoint ID: 0x%llx", endpointIDValue);
        return nil;
    }

    auto id = static_cast<EndpointId>(endpointIDValue);
    if (!IsValidEndpointId(id)) {
        MTR_LOG_ERROR("MTRServerEndpoint provided invalid endpoint ID: 0x%x", id);
        return nil;
    }

    if (id == kRootEndpointId) {
        // We don't allow this; we use that endpoint for our own purposes in
        // Matter.framework.
        MTR_LOG_ERROR("MTRServerEndpoint provided invalid endpoint ID: 0x%x", id);
        return nil;
    }

    if (deviceTypes.count == 0) {
        MTR_LOG_ERROR("MTRServerEndpoint needs a non-empty list of device types");
        return nil;
    }

    return [self initInternalWithEndpointID:endpointID deviceTypes:deviceTypes accessGrants:[NSSet set] clusters:@[]];
}

- (instancetype)initInternalWithEndpointID:(NSNumber *)endpointID deviceTypes:(NSArray<MTRDeviceType *> *)deviceTypes accessGrants:(NSSet *)accessGrants clusters:(NSArray *)clusters
{
    if (!(self = [super init])) {
        return nil;
    }

    _state = MTRServerEndpointStateInitializing;
    _endpointID = [endpointID copy];
    _deviceTypes = [deviceTypes copy];
    _accessGrants = [[NSMutableSet alloc] init];
    _serverClusters = [[NSMutableArray alloc] init];
    _matterAccessGrants = [NSSet set];

    for (MTRAccessGrant * grant in accessGrants) {
        // Since our state is MTRServerEndpointStateInitializing, we know this
        // will succeed.
        [self addAccessGrant:[grant copy]];
    }

    for (MTRServerCluster * cluster in clusters) {
        // Since our state is MTRServerEndpointStateInitializing and the initial
        // cluster array was valid, we know this will succeed.
        [self addServerCluster:[cluster copy]];
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
        MTR_LOG_ERROR("Cannot %@ on no-longer-active endpoint %llu", action, _endpointID.unsignedLongLongValue);
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

- (BOOL)addServerCluster:(MTRServerCluster *)serverCluster
{
    if (_state != MTRServerEndpointStateInitializing) {
        MTR_LOG_ERROR("Cannot add cluster on endpoint %llu which is already in use", _endpointID.unsignedLongLongValue);
        return NO;
    }

    if (serverCluster.parentEndpoint != kInvalidEndpointId) {
        MTR_LOG_ERROR("Cannot add cluster to endpoint %llu; already added to endpoint %" PRIu32, _endpointID.unsignedLongLongValue, serverCluster.parentEndpoint);
        return NO;
    }

    for (MTRServerCluster * existingCluster in _serverClusters) {
        if ([existingCluster.clusterID isEqual:serverCluster.clusterID]) {
            MTR_LOG_ERROR("Cannot add second cluster with ID %llx on endpoint %llu", serverCluster.clusterID.unsignedLongLongValue, _endpointID.unsignedLongLongValue);
            return NO;
        }
    }

    [_serverClusters addObject:serverCluster];
    serverCluster.parentEndpoint = static_cast<EndpointId>(_endpointID.unsignedLongLongValue);
    return YES;
}

- (BOOL)associateWithController:(MTRDeviceController *)controller
{
    if (_state != MTRServerEndpointStateInitializing) {
        MTR_LOG_ERROR("Cannot associate MTRServerEndpoint in state %lu with controller", static_cast<unsigned long>(_state));
        return NO;
    }

    for (MTRServerCluster * cluster in _serverClusters) {
        if (![cluster associateWithController:controller]) {
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

    for (MTRServerCluster * cluster in _serverClusters) {
        [cluster invalidate];
    }

    _deviceController = nil;
}

- (NSSet<MTRAccessGrant *> *)accessGrants
{
    return [_accessGrants copy];
}

- (NSArray<MTRServerCluster *> *)serverClusters
{
    return [_serverClusters copy];
}

- (id)copyWithZone:(NSZone *)zone
{
    return [[MTRServerEndpoint alloc] initInternalWithEndpointID:_endpointID deviceTypes:_deviceTypes accessGrants:_accessGrants clusters:_serverClusters];
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    MTRServerEndpoint * other = object;

    // Note: ignoring _matterAccessGrants, which is just a possibly-delayed
    // snapshot of _accessGrants.
    return [_endpointID isEqual:other.endpointID] && [_deviceTypes isEqual:other.deviceTypes] && [_accessGrants isEqualToSet:other.accessGrants] && [_serverClusters isEqualToArray:other.serverClusters];
}

- (NSUInteger)hash
{
    return _endpointID.unsignedShortValue ^ [_deviceTypes hash] ^ [_accessGrants hash] ^ [_serverClusters hash];
}

@end
