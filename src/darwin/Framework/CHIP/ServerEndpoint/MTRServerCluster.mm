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
#import "MTRUnfairLock.h"
#import "NSDataSpanConversion.h"

#import <Matter/MTRClusterConstants.h>
#import <Matter/MTRServerCluster.h>

#include <app/AttributeAccessInterface.h>
#include <app/clusters/descriptor/descriptor.h>
#include <app/data-model/PreEncodedValue.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#import <os/lock.h>
#include <protocols/interaction_model/StatusCode.h>

// TODO: These attribute-*.h and AttributeAccessInterfaceRegistry.h bits are a hack that should eventually go away.
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;

class MTRServerAttributeAccessInterface : public AttributeAccessInterface {
public:
    MTRServerAttributeAccessInterface(EndpointId aEndpointID, ClusterId aClusterID, NSArray<MTRServerAttribute *> * aAttributes,
        NSNumber * aClusterRevision)
        : AttributeAccessInterface(MakeOptional(aEndpointID), aClusterID)
        , mAttributes(aAttributes)
        , mClusterRevision(aClusterRevision)
    {
    }

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    NSArray<MTRServerAttribute *> * mAttributes;
    NSNumber * mClusterRevision;
};

MTR_DIRECT_MEMBERS
@implementation MTRServerCluster {
    /**
     * The access grants our API consumer can manipulate directly.  Never
     * touched on the Matter queue.
     */
    NSMutableSet<MTRAccessGrant *> * _accessGrants;
    NSMutableArray<MTRServerAttribute *> * _attributes;
    MTRDeviceController * __weak _deviceController;

    std::unique_ptr<MTRServerAttributeAccessInterface> _attributeAccessInterface;
    // We can't use something like std::unique_ptr<EmberAfAttributeMetadata[]>
    // because EmberAfAttributeMetadata does not have a default constructor, so
    // we can't alloc and then initialize later.
    std::vector<EmberAfAttributeMetadata> _matterAttributeMetadata;

    std::unique_ptr<CommandId[]> _matterAcceptedCommandList;
    std::unique_ptr<CommandId[]> _matterGeneratedCommandList;

    NSSet<MTRAccessGrant *> * _matterAccessGrants;

    chip::EndpointId _parentEndpoint;

    // _acceptedCommands and _generatedCommands are touched directly by our API
    // consumer.
    NSArray<NSNumber *> * _acceptedCommands;
    NSArray<NSNumber *> * _generatedCommands;

    /**
     * _lock always protects access to all our mutable ivars (the ones that are
     * modified after init).
     */
    os_unfair_lock _lock;
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
    return [[MTRServerCluster alloc] initInternalWithClusterID:@(MTRClusterIDTypeDescriptorID) revision:@(Clusters::Descriptor::kClusterRevision) accessGrants:[NSSet set] attributes:@[]];
}

- (instancetype)initInternalWithClusterID:(NSNumber *)clusterID revision:(NSNumber *)revision accessGrants:(NSSet *)accessGrants attributes:(NSArray *)attributes
{
    if (!(self = [super init])) {
        return nil;
    }

    _lock = OS_UNFAIR_LOCK_INIT;
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
    os_unfair_lock_assert_owner(&_lock);

    MTRDeviceController * deviceController = _deviceController;
    if (deviceController == nil) {
        // _matterAccessGrants will be updated when we get bound to a controller.
        return;
    }

    NSSet * grants = [_accessGrants copy];
    [deviceController asyncDispatchToMatterQueue:^{
        std::lock_guard lock(self->_lock);
        self->_matterAccessGrants = grants;
    }
                                    errorHandler:nil];
}

- (void)addAccessGrant:(MTRAccessGrant *)accessGrant
{
    std::lock_guard lock(self->_lock);

    [_accessGrants addObject:accessGrant];

    [self updateMatterAccessGrants];
}

- (void)removeAccessGrant:(MTRAccessGrant *)accessGrant;
{
    std::lock_guard lock(self->_lock);

    [_accessGrants removeObject:accessGrant];

    [self updateMatterAccessGrants];
}

- (NSArray<MTRAccessGrant *> *)matterAccessGrants
{
    std::lock_guard lock(self->_lock);

    return [_matterAccessGrants allObjects];
}

- (BOOL)addAttribute:(MTRServerAttribute *)attribute
{
    std::lock_guard lock(self->_lock);

    MTRDeviceController * deviceController = _deviceController;
    if (deviceController != nil) {
        MTR_LOG_ERROR("Cannot add attribute on cluster %llx which is already in use", _clusterID.unsignedLongLongValue);
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

    if (![attribute addToCluster:ConcreteClusterPath(_parentEndpoint, static_cast<ClusterId>(_clusterID.unsignedLongLongValue))]) {
        return NO;
    }

    [_attributes addObject:attribute];
    return YES;
}

static constexpr EmberAfAttributeMetadata sDescriptorAttributesMetadata[] = {
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID, ARRAY, 0, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeClusterDescriptorAttributeServerListID, ARRAY, 0, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeClusterDescriptorAttributeClientListID, ARRAY, 0, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeClusterDescriptorAttributePartsListID, ARRAY, 0, 0),
};

- (BOOL)associateWithController:(nullable MTRDeviceController *)controller
{
    std::lock_guard lock(self->_lock);

    MTRDeviceController * existingController = _deviceController;
    if (existingController != nil) {
        MTR_LOG_ERROR("Cannot associate MTRServerCluster with controller %@; already associated with controller %@",
            controller.uniqueIdentifier, existingController.uniqueIdentifier);
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

    // _attributes shouldn't be able to change anymore, so we can now construct
    // our EmberAfAttributeMetadata array.
    size_t attributeCount = _attributes.count;

    // Figure out whether we need to synthesize a FeatureMap attribute.
    bool needsFeatureMap = true;
    for (MTRServerAttribute * attr in _attributes) {
        if ([attr.attributeID isEqual:@(MTRClusterGlobalAttributeFeatureMapID)]) {
            needsFeatureMap = false;
            break;
        }
    }

    bool needsDescriptorAttributes = [_clusterID isEqual:@(MTRClusterIDTypeDescriptorID)];

    if (needsFeatureMap) {
        ++attributeCount;
    }

    if (needsDescriptorAttributes) {
        attributeCount += MATTER_ARRAY_SIZE(sDescriptorAttributesMetadata);
    }

    // And add one for ClusterRevision
    ++attributeCount;

    if (attributeCount >= UINT16_MAX) {
        MTR_LOG_ERROR("Unable to have %llu attributes in a single cluster (clusterID: " ChipLogFormatMEI ")",
            static_cast<unsigned long long>(attributeCount), ChipLogValueMEI(_clusterID.unsignedLongLongValue));
        return NO;
    }

    size_t attrIndex = 0;
    for (; attrIndex < _attributes.count; ++attrIndex) {
        auto * attr = _attributes[attrIndex];
        _matterAttributeMetadata.emplace_back(EmberAfAttributeMetadata(DECLARE_DYNAMIC_ATTRIBUTE(static_cast<AttributeId>(attr.attributeID.unsignedLongLongValue),
            // The type does not actually matter, since we plan to
            // handle this entirely via AttributeAccessInterface.
            // Claim Array because that one will keep random IM
            // code from trying to do things with the attribute
            // store.
            ARRAY,
            // Size in bytes does not matter, since we plan to
            // handle this entirely via AttributeAccessInterface.
            0,
            // MATTER_ATTRIBUTE_FLAG_NULLABLE is not relevant because we
            // are handling this all via AttributeAccessInterface.
            0)));
    }

    if (needsFeatureMap) {
        _matterAttributeMetadata.emplace_back(EmberAfAttributeMetadata(DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeGlobalAttributeFeatureMapID,
            BITMAP32, 4, 0)));
        ++attrIndex;
    }

    if (needsDescriptorAttributes) {
        for (auto & data : sDescriptorAttributesMetadata) {
            _matterAttributeMetadata.emplace_back(data);
            ++attrIndex;
        }
    }

    // Add our ClusterRevision bit.
    _matterAttributeMetadata.emplace_back(EmberAfAttributeMetadata(DECLARE_DYNAMIC_ATTRIBUTE(MTRAttributeIDTypeGlobalAttributeClusterRevisionID,
        INT16U, 2, 0)));
    ++attrIndex;

    _attributeAccessInterface = std::make_unique<MTRServerAttributeAccessInterface>(_parentEndpoint,
        static_cast<ClusterId>(_clusterID.unsignedLongLongValue),
        _attributes,
        _clusterRevision);
    // _attributeAccessInterface needs to be registered on the Matter queue; that will happen later.

    _matterAcceptedCommandList = [MTRServerCluster makeMatterCommandList:_acceptedCommands];
    _matterGeneratedCommandList = [MTRServerCluster makeMatterCommandList:_generatedCommands];

    _deviceController = controller;

    MTR_LOG("Associated %@, attribute count %llu, with controller %@", [self _descriptionWhileLocked],
        static_cast<unsigned long long>(attributeCount), controller);

    return YES;
}

- (void)invalidate
{
    std::lock_guard lock(_lock);

    // Undo any work associateWithController did.
    for (MTRServerAttribute * attr in _attributes) {
        [attr invalidate];
    }

    // We generally promise to only touch _matterAccessGrants on the Matter
    // queue after associateWithController succeeds, but we are no longer being
    // looked at from that queue, so it's safe to reset it here.
    _matterAccessGrants = [NSSet set];
    _matterAttributeMetadata.clear();
    _attributeAccessInterface.reset();
    _matterAcceptedCommandList.reset();
    _matterGeneratedCommandList.reset();

    _deviceController = nil;
}

- (void)registerMatterCluster
{
    assertChipStackLockedByCurrentThread();

    std::lock_guard lock(_lock);

    if (!AttributeAccessInterfaceRegistry::Instance().Register(_attributeAccessInterface.get())) {
        // This should only happen if we somehow managed to register an
        // AttributeAccessInterface for the same (endpoint, cluster) pair.
        MTR_LOG_ERROR("Could not register AttributeAccessInterface for endpoint %u, cluster 0x%llx",
            _parentEndpoint, _clusterID.unsignedLongLongValue);
    }
}

- (void)unregisterMatterCluster
{
    assertChipStackLockedByCurrentThread();

    std::lock_guard lock(_lock);

    if (_attributeAccessInterface != nullptr) {
        AttributeAccessInterfaceRegistry::Instance().Unregister(_attributeAccessInterface.get());
    }
}

- (NSArray<MTRAccessGrant *> *)accessGrants
{
    std::lock_guard lock(_lock);

    return [_accessGrants allObjects];
}

- (NSArray<MTRServerAttribute *> *)attributes
{
    std::lock_guard lock(_lock);

    return [_attributes copy];
}

- (BOOL)addToEndpoint:(chip::EndpointId)endpoint
{
    std::lock_guard lock(_lock);

    if (_parentEndpoint != kInvalidEndpointId) {
        MTR_LOG_ERROR("Cannot add cluster " ChipLogFormatMEI " to endpoint %" PRIu32 "; already added to endpoint %" PRIu32,
            ChipLogValueMEI(_clusterID.unsignedLongLongValue), endpoint, _parentEndpoint);
        return NO;
    }

    _parentEndpoint = endpoint;
    // Update it on all the attributes, in case the attributes were added to us
    // before we were added to the endpoint.
    for (MTRServerAttribute * attr in _attributes) {
        [attr updateParentCluster:ConcreteClusterPath(endpoint, static_cast<ClusterId>(_clusterID.unsignedLongLongValue))];
    }
    return YES;
}

- (chip::EndpointId)parentEndpoint
{
    std::lock_guard lock(_lock);
    return _parentEndpoint;
}

- (Span<const EmberAfAttributeMetadata>)matterAttributeMetadata
{
    // This is always called after our _matterAttributeMetadata has been set up
    // by associateWithController.
    std::lock_guard lock(_lock);
    return Span<const EmberAfAttributeMetadata>(_matterAttributeMetadata.data(), _matterAttributeMetadata.size());
}

- (void)setAcceptedCommands:(NSArray<NSNumber *> *)acceptedCommands
{
    std::lock_guard lock(_lock);
    _acceptedCommands = [acceptedCommands copy];
}

- (NSArray<NSNumber *> *)acceptedCommands
{
    std::lock_guard lock(_lock);
    return [_acceptedCommands copy];
}

- (void)setGeneratedCommands:(NSArray<NSNumber *> *)generatedCommands
{
    std::lock_guard lock(_lock);
    _generatedCommands = [generatedCommands copy];
}

- (NSArray<NSNumber *> *)generatedCommands
{
    std::lock_guard lock(_lock);
    return [_generatedCommands copy];
}

- (CommandId *)matterAcceptedCommands
{
    std::lock_guard lock(_lock);
    return _matterAcceptedCommandList.get();
}

- (CommandId *)matterGeneratedCommands
{
    std::lock_guard lock(_lock);
    return _matterGeneratedCommandList.get();
}

+ (std::unique_ptr<CommandId[]>)makeMatterCommandList:(NSArray<NSNumber *> * _Nullable)commandList
{
    if (commandList.count == 0) {
        return nullptr;
    }

    // Lists of accepted/generated commands are terminated by kInvalidClusterId.
    auto matterCommandList = std::make_unique<CommandId[]>(commandList.count + 1);
    for (size_t index = 0; index < commandList.count; ++index) {
        matterCommandList[index] = static_cast<CommandId>(commandList[index].unsignedLongLongValue);
    }
    matterCommandList[commandList.count] = kInvalidClusterId;
    return matterCommandList;
}

- (NSString *)description
{
    std::lock_guard lock(_lock);
    return [self _descriptionWhileLocked];
}

- (NSString *)_descriptionWhileLocked
{
    os_unfair_lock_assert_owner(&_lock);
    return [NSString stringWithFormat:@"<MTRServerCluster endpoint %u, id " ChipLogFormatMEI ">",
                     _parentEndpoint, ChipLogValueMEI(_clusterID.unsignedLongLongValue)];
}

@end

CHIP_ERROR MTRServerAttributeAccessInterface::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    using DataModel::PreEncodedValue;

    // Find the right attribute in our list.
    MTRServerAttribute * foundAttr = nil;
    for (MTRServerAttribute * attr in mAttributes) {
        if ([attr.attributeID isEqual:@(aPath.mAttributeId)]) {
            foundAttr = attr;
            break;
        }
    }

    if (foundAttr) {
        id value = foundAttr.serializedValue;
        if (![value isKindOfClass:NSArray.class]) {
            // It's a single value, so NSData.
            NSData * data = value;
            return aEncoder.Encode(PreEncodedValue(AsByteSpan(data)));
        }

        // It's a list of data values.
        NSArray<NSData *> * dataList = value;
        return aEncoder.EncodeList([dataList](const auto & itemEncoder) {
            for (NSData * item in dataList) {
                ReturnErrorOnFailure(itemEncoder.Encode(PreEncodedValue(AsByteSpan(item))));
            }
            return CHIP_NO_ERROR;
        });
    }

    // This must be the FeatureMap attribute we synthesized.
    if (aPath.mAttributeId == MTRAttributeIDTypeGlobalAttributeFeatureMapID) {
        // Feature map defaults to 0.
        constexpr uint32_t defaultFeatureMap = 0;
        return aEncoder.Encode(defaultFeatureMap);
    }

    if (aPath.mAttributeId == MTRAttributeIDTypeGlobalAttributeClusterRevisionID) {
        return aEncoder.Encode(mClusterRevision.unsignedLongLongValue);
    }

    // Note: This code is not reached for the descriptor cluster, which uses its own AttributeAccessInterface.

    return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
}
