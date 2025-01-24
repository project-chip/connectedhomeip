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
#import <Matter/MTRClusterConstants.h>
#import <Matter/MTRServerEndpoint.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>
#include <platform/LockTracker.h>

// TODO: These af-types.h and att-storage.h and attribute-storage.h and
// endpoint-config-api.h and probably CodeUtils.h bits are a hack that should
// eventually go away.
#include <app/util/af-types.h>
#include <app/util/att-storage.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/CodeUtils.h>

using namespace chip;

MTR_DIRECT_MEMBERS
@implementation MTRServerEndpoint {
    /**
     * The access grants our API consumer can manipulate directly.  Never
     * touched on the Matter queue.
     */
    NSMutableSet<MTRAccessGrant *> * _accessGrants;
    NSMutableArray<MTRServerCluster *> * _serverClusters;
    MTRDeviceController * __weak _deviceController;
    std::unique_ptr<EmberAfCluster[]> _matterClusterMetadata;
    EmberAfEndpointType _matterEndpointMetadata;
    std::unique_ptr<EmberAfDeviceType[]> _matterDeviceTypes;
    std::unique_ptr<DataVersion[]> _matterDataVersions;

    // _endpointIndex has a value only when we have the endpoint configured.
    std::optional<uint16_t> _endpointIndex;
}

- (nullable instancetype)initWithEndpointID:(NSNumber *)endpointID deviceTypes:(NSArray<MTRDeviceTypeRevision *> *)deviceTypes
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

+ (MTRServerEndpoint *)rootNodeEndpoint
{
    return [[MTRServerEndpoint alloc] initInternalWithEndpointID:@(kRootEndpointId) deviceTypes:@[] accessGrants:[NSSet set] clusters:@[]];
}

- (instancetype)initInternalWithEndpointID:(NSNumber *)endpointID deviceTypes:(NSArray<MTRDeviceTypeRevision *> *)deviceTypes accessGrants:(NSSet *)accessGrants clusters:(NSArray *)clusters
{
    if (!(self = [super init])) {
        return nil;
    }

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

- (BOOL)addServerCluster:(MTRServerCluster *)serverCluster
{
    MTRDeviceController * deviceController = _deviceController;
    if (deviceController != nil) {
        MTR_LOG_ERROR("Cannot add cluster on endpoint %llu which is already in use", _endpointID.unsignedLongLongValue);
        return NO;
    }

    for (MTRServerCluster * existingCluster in _serverClusters) {
        if ([existingCluster.clusterID isEqual:serverCluster.clusterID]) {
            MTR_LOG_ERROR("Cannot add second cluster with ID " ChipLogFormatMEI " on endpoint %llu", ChipLogValueMEI(serverCluster.clusterID.unsignedLongLongValue), _endpointID.unsignedLongLongValue);
            return NO;
        }
    }

    if (![serverCluster addToEndpoint:static_cast<EndpointId>(_endpointID.unsignedLongLongValue)]) {
        return NO;
    }
    [_serverClusters addObject:serverCluster];

    return YES;
}

#define MTR_DECLARE_LIST_ATTRIBUTE(attrID) \
    DECLARE_DYNAMIC_ATTRIBUTE(attrID, ARRAY, 0, 0)

static constexpr EmberAfAttributeMetadata sDescriptorAttributesMetadata[] = {
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID, ARRAY, 0, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeClusterDescriptorAttributeServerListID, ARRAY, 0, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeClusterDescriptorAttributeClientListID, ARRAY, 0, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeClusterDescriptorAttributePartsListID, ARRAY, 0, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeGlobalAttributeFeatureMapID, BITMAP32, 4, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeGlobalAttributeClusterRevisionID, INT16U, 2, 0),
};

#undef MTR_DECLARE_LIST_ATTRIBUTE

- (BOOL)associateWithController:(nullable MTRDeviceController *)controller
{
    MTRDeviceController * existingController = _deviceController;
    if (existingController != nil) {
        MTR_LOG_ERROR("Cannot associate MTRServerEndpoint with controller %@; already associated with controller %@",
            controller.uniqueIdentifier, existingController.uniqueIdentifier);
        return NO;
    }

    // After this point we have to make sure we clean up on any failures.
    if (![self finishAssociationWithController:controller]) {
        [self invalidate];
        return NO;
    }

    return YES;
}

- (BOOL)finishAssociationWithController:(nullable MTRDeviceController *)controller
{
    for (MTRServerCluster * cluster in _serverClusters) {
        if (![cluster associateWithController:controller]) {
            return NO;
        }
    }

    // Snapshot _matterAccessGrants now; after this point it will only be
    // updated on the Matter queue.
    _matterAccessGrants = [_accessGrants copy];

    // _serverClusters shouldn't be able to change anymore, so we can now
    // construct our EmberAfCluster array.
    size_t clusterCount = _serverClusters.count;

    // Figure out whether we need to synthesize a Descriptor cluster.
    bool needsDescriptor = true;
    for (MTRServerCluster * cluster in _serverClusters) {
        if ([cluster.clusterID isEqual:@(MTRClusterIDTypeDescriptorID)]) {
            needsDescriptor = false;
            break;
        }
    }

    if (needsDescriptor) {
        ++clusterCount;
    }

    if (clusterCount >= 0xFF) {
        // The ember bits don't allow this many clusters (they use 0xFF to mean
        // "no such cluster" in various places.
        MTR_LOG_ERROR("Unable to create endpoint with %llu clusters; it's too many",
            static_cast<unsigned long long>(clusterCount));
        return NO;
    }

    _matterClusterMetadata = std::make_unique<EmberAfCluster[]>(clusterCount);
    // std::make_unique never returns null; it will try to throw an exception
    // and likely crash on OOM.

    size_t clusterIndex = 0;
    for (; clusterIndex < _serverClusters.count; ++clusterIndex) {
        auto * cluster = _serverClusters[clusterIndex];
        auto & metadata = _matterClusterMetadata[clusterIndex];

        metadata.clusterId = static_cast<ClusterId>(cluster.clusterID.unsignedLongLongValue);

        auto attrMetadata = cluster.matterAttributeMetadata;
        metadata.attributes = attrMetadata.data();
        // This cast is safe because clusters check for this constraint on
        // number of attributes.
        metadata.attributeCount = static_cast<uint16_t>(attrMetadata.size());

        metadata.clusterSize = 0; // All our attributes are external.

        metadata.mask = CLUSTER_MASK_SERVER;

        metadata.functions = nullptr; // None of our clusters, including Descriptor, uses these.

        metadata.acceptedCommandList = cluster.matterAcceptedCommands;
        metadata.generatedCommandList = cluster.matterGeneratedCommands;

        metadata.eventList = nullptr;
        metadata.eventCount = 0;
    }

    if (needsDescriptor) {
        auto & metadata = _matterClusterMetadata[clusterIndex];

        metadata.clusterId = MTRClusterIDTypeDescriptorID;

        metadata.attributes = sDescriptorAttributesMetadata;
        metadata.attributeCount = ArraySize(sDescriptorAttributesMetadata);

        metadata.clusterSize = 0; // All our attributes are external.

        metadata.mask = CLUSTER_MASK_SERVER;

        metadata.functions = nullptr; // Descriptor does not use these.

        metadata.acceptedCommandList = nullptr;
        metadata.generatedCommandList = nullptr;

        metadata.eventList = nullptr;
        metadata.eventCount = 0;

        ++clusterIndex;
    }

    _matterEndpointMetadata.cluster = _matterClusterMetadata.get();
    // Cast is safe, because we did a range check above.
    _matterEndpointMetadata.clusterCount = static_cast<decltype(_matterEndpointMetadata.clusterCount)>(clusterCount);
    _matterEndpointMetadata.endpointSize = 0; // All our attributes are external.

    _matterDeviceTypes = std::make_unique<EmberAfDeviceType[]>(_deviceTypes.count);
    for (size_t index = 0; index < _deviceTypes.count; ++index) {
        auto * deviceType = _deviceTypes[index];
        auto & matterType = _matterDeviceTypes[index];

        matterType.deviceTypeId = static_cast<DeviceTypeId>(deviceType.deviceTypeID.unsignedLongLongValue);
        // TODO: The spec allows 16-bit revisions, but DeviceTypeEntry only
        // supports 8-bit....
        matterType.deviceTypeRevision = static_cast<uint8_t>(deviceType.deviceTypeRevision.unsignedLongLongValue);
    }

    _matterDataVersions = std::make_unique<DataVersion[]>(clusterCount);

    _deviceController = controller;

    MTR_LOG("Associated %@, cluster count %llu, with controller %@",
        self, static_cast<unsigned long long>(clusterCount), controller);

    return YES;
}

- (void)registerMatterEndpoint
{
    assertChipStackLockedByCurrentThread();

    static_assert(FIXED_ENDPOINT_COUNT == 0, "Indexing will be off");

    // We can't use emberAfEndpointCount here, because that returns just the
    // count of fixed endpoints up until the first call to
    // emberAfSetDynamicEndpoint().
    uint16_t possibleEndpointCount = MAX_ENDPOINT_COUNT;
    uint16_t index = 0;
    for (; index < possibleEndpointCount; ++index) {
        if (emberAfEndpointFromIndex(index) == kInvalidEndpointId) {
            break;
        }
    }

    if (index == possibleEndpointCount) {
        // Something is very broken.  We shouldn't have this many endpoints!
        MTR_LOG_ERROR("We somehow ran out of endpoint slots.");
        return;
    }

    auto status = emberAfSetDynamicEndpoint(index, static_cast<EndpointId>(_endpointID.unsignedLongLongValue),
        &_matterEndpointMetadata,
        Span<DataVersion>(_matterDataVersions.get(), _matterEndpointMetadata.clusterCount),
        Span<EmberAfDeviceType>(_matterDeviceTypes.get(), _deviceTypes.count));
    if (status != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Unexpected failure to define our Matter endpoint");
    }

    _endpointIndex.emplace(index);

    for (MTRServerCluster * cluster in _serverClusters) {
        [cluster registerMatterCluster];
    }
}

- (void)unregisterMatterEndpoint
{
    assertChipStackLockedByCurrentThread();

    if (_endpointIndex.has_value()) {
        emberAfClearDynamicEndpoint(_endpointIndex.value());
        _endpointIndex.reset();
    }

    for (MTRServerCluster * cluster in _serverClusters) {
        [cluster unregisterMatterCluster];
    }
}

- (void)invalidate
{
    // Undo any work associateWithController did.
    for (MTRServerCluster * cluster in _serverClusters) {
        [cluster invalidate];
    }

    // We generally promise to only touch _matterAccessGrants on the Matter
    // queue after associateWithController succeeds, but we are no longer being
    // looked at from that queue, so it's safe to reset it here.
    _matterAccessGrants = [NSSet set];
    _matterEndpointMetadata.cluster = nullptr;
    _matterEndpointMetadata.clusterCount = 0;
    _matterClusterMetadata.reset();
    _matterDeviceTypes.reset();
    _matterDataVersions.reset();
    _deviceController = nil;
}

- (NSArray<MTRAccessGrant *> *)matterAccessGrantsForCluster:(NSNumber *)clusterID
{
    assertChipStackLockedByCurrentThread();

    NSMutableArray<MTRAccessGrant *> * grants = [[_matterAccessGrants allObjects] mutableCopy];
    for (MTRServerCluster * cluster in _serverClusters) {
        if ([cluster.clusterID isEqual:clusterID]) {
            [grants addObjectsFromArray:cluster.matterAccessGrants];
        }
    }

    return [grants copy];
}

- (NSArray<MTRAccessGrant *> *)accessGrants
{
    return [_accessGrants allObjects];
}

- (NSArray<MTRServerCluster *> *)serverClusters
{
    return [_serverClusters copy];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRServerEndpoint id %u>", static_cast<EndpointId>(_endpointID.unsignedLongLongValue)];
}

@end
