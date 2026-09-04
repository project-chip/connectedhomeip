/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <app/clusters/descriptor/DescriptorCluster.h>
#include <app/util/basic-types.h>
#include <clusters/Descriptor/Structs.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.h>
#include <device/api/allocator/EndpointIdAllocator.h>
#include <lib/support/Span.h>

#include <cstdint>

namespace chip::app {

// Constants for semantic tags. Defined in "Matter Standard Namespaces"
//
// TODO: Generate standard namespace ID constants from spec and place them in libCHIP.
//       We need:
//          - namespace id generation
//          - tag value generation
//       Specification describes these in `src/namespaces`
namespace CommonNamespace {

constexpr uint16_t kCompassDirectionId          = 0x02;
constexpr uint16_t kCompassLocationId           = 0x03;
constexpr uint16_t kDirectionId                 = 0x04;
constexpr uint16_t kLevelId                     = 0x05;
constexpr uint16_t kLocationId                  = 0x06; // Clusters::Globals::LocationTag
constexpr uint16_t kNumberId                    = 0x07;
constexpr uint16_t kPositionId                  = 0x08; // Clusters::Globals::PositionTag
constexpr uint16_t kElectricalMeasurementId     = 0x0A;
constexpr uint16_t kCommodityTariffChronologyId = 0x0B;
constexpr uint16_t kCommodityTariffDirectionId  = 0x0D;
constexpr uint16_t kLaundryId                   = 0x0E;
constexpr uint16_t kPowerSourceId               = 0x0F;
constexpr uint16_t kCommonAreaId                = 0x10; // Clusters::Globals::AreaTypeTag
constexpr uint16_t kCommonLandmarkId            = 0x11; // Clusters::Globals::LandmarkTag
constexpr uint16_t kRelativePositionId          = 0x12; // Clusters::Globals::RelativePositionTag

} // namespace CommonNamespace

/// Structural configuration for registering an endpoint within a node hierarchy.
///
/// Encapsulates parent-child relationship, family composition pattern, and static semantic tags.
struct EndpointComposition
{
    using SemanticTag = Clusters::Globals::Structs::SemanticTagStruct::Type;

    EndpointId parentId                           = kInvalidEndpointId;
    DataModel::EndpointCompositionPattern pattern = DataModel::EndpointCompositionPattern::kTree;
    Span<const SemanticTag> tagList               = {};

    constexpr EndpointComposition() = default;
    constexpr EndpointComposition(
        EndpointId parent, DataModel::EndpointCompositionPattern compositionPattern = DataModel::EndpointCompositionPattern::kTree,
        Span<const SemanticTag> tags = {}) :
        parentId(parent),
        pattern(compositionPattern), tagList(tags)
    {}
    static constexpr EndpointComposition WithParent(EndpointId parent) { return EndpointComposition(parent); }
};

/// A device is an entity that maintains some cluster functionality.
class DeviceInterface : public EndpointInterface
{
public:
    virtual ~DeviceInterface() = default;

    /// Register relevant clusters on the given endpoint. This must only
    /// be called once after starting up a device for the first time. This function
    /// will create/instantiate all clusters on the device and complete endpoint registration.
    /// It should return error if there's any failure when adding the device's clusters to the provider.
    /// An empty or default EndpointComposition represents that there is no parent or static tags.
    virtual CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                EndpointComposition composition = {}) = 0;

    /// Removes a device's clusters from the given provider. This
    /// must only be called when register has succeeded before. Expected
    /// usage of this function is for when the device is no longer needed
    /// (for example, on shutdown), to destroy the device's clusters.
    virtual void Unregister(CodeDrivenDataModelProvider & provider) = 0;

    // Endpoint interface implementation
    CHIP_ERROR DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const override;
    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

protected:
    /// The caller creating a DeviceInterface MUST ensure that the underlying data for the Span of
    /// deviceTypes remains valid for the entire lifetime of the DeviceInterface object instance.
    DeviceInterface(Span<const DataModel::DeviceTypeEntry> deviceTypes) :
        mDeviceTypes(deviceTypes), mEndpointRegistration(*this, {})
    {}

    /// Internal method to register the descriptor cluster for this device on the given endpoint
    ///
    /// virtual in case subclasses want to track extra data (e.g. endpoint id)
    virtual CHIP_ERROR RegisterDescriptor(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                          EndpointComposition composition = {});

    /// Reverse of `RegisterDescrioptor`
    virtual void UnregisterDescriptor(EndpointId endpoint, CodeDrivenDataModelProvider & provider);

    Span<const DataModel::DeviceTypeEntry> mDeviceTypes;
    EndpointInterfaceRegistration mEndpointRegistration;

    // Common clusters..
    LazyRegisteredServerCluster<Clusters::DescriptorCluster> mDescriptorCluster;
};

/**
 * RAII guard to automatically rollback/unregister a device if registration fails midway.
 *
 * Instantiate this guard at the start of `Register`. If the registration succeeds,
 * call `Commit()`. If the guard is destroyed without being committed (e.g., due to
 * an early return on error), its destructor will automatically call `Unregister` on the device.
 */
class DeviceRegistrationTransaction
{
public:
    DeviceRegistrationTransaction(DeviceInterface & device, CodeDrivenDataModelProvider & provider) :
        mDevice(device), mProvider(provider)
    {}

    ~DeviceRegistrationTransaction()
    {
        if (!mCommitted)
        {
            mDevice.Unregister(mProvider);
        }
    }

    /// Mark the registration as successful, preventing rollback on destruction.
    void Commit() { mCommitted = true; }

private:
    DeviceInterface & mDevice;
    CodeDrivenDataModelProvider & mProvider;
    bool mCommitted = false;
};

} // namespace chip::app
