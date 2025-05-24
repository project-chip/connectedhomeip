/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <protocols/interaction_model/StatusCode.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

static constexpr size_t kMaxUrlLength = 2000u;

using SupportedFormatStruct                   = Structs::SupportedFormatStruct::Type;
using CMAFContainerOptionsStruct              = Structs::CMAFContainerOptionsStruct::Type;
using ContainerOptionsStruct                  = Structs::ContainerOptionsStruct::Type;
using TransportZoneOptionsStruct              = Structs::TransportZoneOptionsStruct::Type;
using TransportTriggerOptionsStruct           = Structs::TransportTriggerOptionsStruct::Type;
using TransportMotionTriggerTimeControlStruct = Structs::TransportMotionTriggerTimeControlStruct::Type;
using TransportOptionsStruct                  = Structs::TransportOptionsStruct::Type;
using TransportConfigurationStruct            = Structs::TransportConfigurationStruct::Type;

enum class PushAvStreamTransportStatusEnum : uint8_t
{
    kBusy    = 0x00,
    kIdle    = 0x01,
    kUnknown = 0x02
};

struct TransportTriggerOptionsStorage : public TransportTriggerOptionsStruct
{
    TransportTriggerOptionsStorage() {};

    TransportTriggerOptionsStorage(Structs::TransportTriggerOptionsStruct::DecodableType triggerOptions,
                                   const BitFlags<Feature> features)
    {
        triggerType = triggerOptions.triggerType;

        if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion && triggerOptions.motionZones.HasValue())
        {
            if (triggerOptions.motionZones.Value().IsNull() == false)
            {
                auto & motionZonesList = triggerOptions.motionZones;
                auto iter              = motionZonesList.Value().Value().begin();

                while (iter.Next())
                {
                    auto & transportZoneOption = iter.GetValue();
                    mTransportZoneOptions.push_back(transportZoneOption);
                }

                motionZones.SetValue(DataModel::Nullable<DataModel::List<const TransportZoneOptionsStruct>>(
                    Span<TransportZoneOptionsStruct>(mTransportZoneOptions.data(), mTransportZoneOptions.size())));
            }
        }
        else
        {
            motionZones.Missing();
        }

        motionSensitivity = triggerOptions.motionSensitivity;
        motionTimeControl = triggerOptions.motionTimeControl;
        maxPreRollLen     = triggerOptions.maxPreRollLen;
    }

private:
    std::vector<TransportZoneOptionsStruct> mTransportZoneOptions;
};

struct CMAFContainerOptionsStorage : public CMAFContainerOptionsStruct
{
    CMAFContainerOptionsStorage() {};

    CMAFContainerOptionsStorage(Optional<Structs::CMAFContainerOptionsStruct::Type> CMAFContainerOptions,
                                const BitFlags<Feature> features)
    {
        if (CMAFContainerOptions.HasValue() == true)
        {
            chunkDuration = CMAFContainerOptions.Value().chunkDuration;

            if (CMAFContainerOptions.Value().CENCKey.HasValue())
            {
                MutableByteSpan CENCKeyBuffer(mCENCKeyBuffer);
                CopySpanToMutableSpan(CMAFContainerOptions.Value().CENCKey.Value(), CENCKeyBuffer);
                CENCKey.SetValue(CENCKeyBuffer);
            }

            if (features.Has(Feature::kMetadata) && CMAFContainerOptions.HasValue())
            {
                metadataEnabled = CMAFContainerOptions.Value().metadataEnabled;
            }
            else
            {
                metadataEnabled.Missing();
            }

            if (CMAFContainerOptions.Value().CENCKey.HasValue())
            {
                MutableByteSpan CENCKeyIDBuffer(mCENCKeyIDBuffer);
                CopySpanToMutableSpan(CMAFContainerOptions.Value().CENCKeyID.Value(), CENCKeyIDBuffer);
                CENCKeyID.SetValue(CENCKeyIDBuffer);
            }
        }
    }

private:
    uint8_t mCENCKeyBuffer[16];
    uint8_t mCENCKeyIDBuffer[16];
};

struct ContainerOptionsStorage : public ContainerOptionsStruct
{
    ContainerOptionsStorage() {};

    ContainerOptionsStorage(Structs::ContainerOptionsStruct::DecodableType containerOptions, const BitFlags<Feature> features)
    {
        containerType = containerOptions.containerType;

        if (containerType == ContainerFormatEnum::kCmaf)
        {
            mCMAFContainerStorage = CMAFContainerOptionsStorage(containerOptions.CMAFContainerOptions, features);
            CMAFContainerOptions.SetValue(mCMAFContainerStorage);
        }
        else
        {
            CMAFContainerOptions.Missing();
        }
    }

private:
    CMAFContainerOptionsStorage mCMAFContainerStorage;
};

struct TransportOptionsStorage : public TransportOptionsStruct
{
    TransportOptionsStorage() {};

    TransportOptionsStorage(Structs::TransportOptionsStruct::DecodableType transportOptions, const BitFlags<Feature> features)
    {
        streamUsage   = transportOptions.streamUsage;
        videoStreamID = transportOptions.videoStreamID;
        audioStreamID = transportOptions.audioStreamID;
        endpointID    = transportOptions.endpointID;

        MutableCharSpan urlBuffer(mUrlBuffer);
        CopyCharSpanToMutableCharSpanWithTruncation(transportOptions.url, urlBuffer);
        url = urlBuffer;

        mTriggerOptionsStorage = TransportTriggerOptionsStorage(transportOptions.triggerOptions, features);
        triggerOptions         = mTriggerOptionsStorage;

        ingestMethod = transportOptions.ingestMethod;

        mContainerOptionsStorage = ContainerOptionsStorage(transportOptions.containerOptions, features);
        containerOptions         = mContainerOptionsStorage;

        expiryTime = transportOptions.expiryTime;
    }

private:
    char mUrlBuffer[kMaxUrlLength];
    TransportTriggerOptionsStorage mTriggerOptionsStorage;
    ContainerOptionsStorage mContainerOptionsStorage;
};

struct TransportConfigurationStorage : public TransportConfigurationStruct
{
    TransportConfigurationStorage() {}

    TransportConfigurationStorage(const uint16_t aConnectionID, std::shared_ptr<TransportOptionsStorage> aTransportOptionsPtr)
    {
        connectionID    = aConnectionID;
        transportStatus = TransportStatusEnum::kInactive;
        /*Store the pointer to keep buffer alive*/
        mTransportOptionsPtr = aTransportOptionsPtr;
        /*Convert Storage type to base type*/
        transportOptions.SetValue(*aTransportOptionsPtr);
    }
    std::shared_ptr<TransportOptionsStorage> GetTransportOptionsPtr() const { return mTransportOptionsPtr; }

private:
    std::shared_ptr<TransportOptionsStorage> mTransportOptionsPtr;
};

struct TransportConfigurationStorageWithFabricIndex
{
    TransportConfigurationStorage transportConfiguration;
    FabricIndex fabricIndex;
};

/** @brief
 *  Defines interfaces for implementing application-specific logic for various aspects of the PushAvStreamTransport Delegate.
 *  Specifically, it defines interfaces for the command handling and loading of the allocated streams.
 */
class PushAvStreamTransportDelegate
{
public:
    PushAvStreamTransportDelegate() = default;

    virtual ~PushAvStreamTransportDelegate() = default;

    /**
     *   @brief Handle Command Delegate for stream transport allocation with the provided transport configuration option.
     *
     *   @param transportOptions[in]        represent the configuration options of the transport to be allocated
     *
     *   @param connectionID[in]            Indicates the connectionID to allocate.
     *
     *   @return Success if the allocation is successful and a PushTransportConnectionID was produced; otherwise, the command SHALL
     *   be rejected with an appropriate error. The delegate is expected to process the transport options, allocate the transport
     * and map it to the connectionID. On Success TransportConfigurationStruct is sent as response by the server.
     */
    virtual Protocols::InteractionModel::Status AllocatePushTransport(const TransportOptionsStruct & transportOptions,
                                                                      const uint16_t connectionID) = 0;
    /**
     *   @brief Handle Command Delegate for Stream transport deallocation for the
     *   provided connectionID.
     *
     *   @param connectionID[in]        Indicates the connectionID to deallocate.
     *
     *   @return Success if the transport deallocation is successful; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     *
     */
    virtual Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID) = 0;
    /**
     *   @brief Handle Command Delegate for Stream transport modification.
     *
     *   @param connectionID [in]           Indicates the connectionID of the stream transport to modify.
     *
     *   @param transportOptions [out]      represents the Trigger Options to modify.
     *
     *   @return Success if the stream transport modification is successful; otherwise, the command SHALL be rejected with an
     * appropriate error.
     */
    virtual Protocols::InteractionModel::Status ModifyPushTransport(const uint16_t connectionID,
                                                                    const TransportOptionsStruct & transportOptions) = 0;

    /**
     *   @brief Handle Command Delegate for Stream transport modification.
     *
     *   @param connectionIDList [in]       represent the list of connectionIDs for which new transport status to apply.
     *   @param transportStatus  [in]       represents the updated status of the connection(s).
     *
     *   @return Success if the stream transport status is successfully set; otherwise, the command SHALL be rejected with an
     * appropriate error.
     */
    virtual Protocols::InteractionModel::Status SetTransportStatus(const std::vector<uint16_t> connectionIDList,
                                                                   TransportStatusEnum transportStatus) = 0;
    /**
     *   @brief Handle Command Delegate to request the Node to manually start the specified push transport.
     *
     *   @param connectionID  [in]          Indicates the connectionID of the stream transport to set trigger for.
     *
     *   @param activationReason [in]       Provide information as to why the transport was started or stopped.
     *
     *   @param timeControl   [in]          Configuration to control the life cycle of a triggered transport.
     *
     *   @return Success if the stream transport trigger is successful; otherwise, the command SHALL be rejected with an
appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status
    ManuallyTriggerTransport(const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
                             const Optional<Structs::TransportMotionTriggerTimeControlStruct::Type> & timeControl) = 0;

    /**
     * @brief Validates the url
     * @param[in] url The url to validate
     *
     * @return boolean value true if the url is valid else false.
     */
    virtual bool ValidateUrl(std::string url) = 0;

    /**
     * @brief Validates the bandwidth requirement against the camera's resource management
     *
     * The implementation SHALL ensure:
     *  - The requested stream usage (streamUsage) is allowed given the current allocation of
     *    camera resources (e.g. CPU, memory, network bandwidth).
     *
     * @param[in] streamUsage    The desired usage type for the stream (e.g. live view, recording, etc.).
     * @param[in] videoStreamId  Optional identifier for the requested video stream.
     * @param[in] audioStreamId  Optional identifier for the requested audio stream.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR if the stream usage is valid; an appropriate error code otherwise.
     */

    virtual CHIP_ERROR ValidateBandwidthLimit(StreamUsageEnum streamUsage,
                                              const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                              const Optional<DataModel::Nullable<uint16_t>> & audioStreamId) = 0;

    /**
     * @brief Validates the requested stream usage against the camera's resource management
     *        and stream priority policies.
     *
     * The implementation SHALL ensure:
     *  - The requested stream usage (streamUsage) is allowed given the current allocation of
     *    camera resources (e.g. CPU, memory, network bandwidth) and the prioritized stream list.
     *
     * @param[in] streamUsage    The desired usage type for the stream (e.g. live view, recording, etc.).
     * @param[in] videoStreamId  Optional identifier for the requested video stream.
     * @param[in] audioStreamId  Optional identifier for the requested audio stream.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR if the stream usage is valid; an appropriate error code otherwise.
     */
    virtual CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage,
                                           const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                           const Optional<DataModel::Nullable<uint16_t>> & audioStreamId) = 0;

    /**
     * @brief Gets the status of the transport
     *
     * @param[in] connectionID Indicates the connectionID of the stream transport to check status
     *
     * @return busy if transport is active else idle
     */
    virtual PushAvStreamTransportStatusEnum GetTransportStatus(const uint16_t connectionID) = 0;

    /**
     *   @brief Delegate callback for notifying change in an attribute.
     *
     */
    virtual void OnAttributeChanged(AttributeId attributeId) = 0;

    /**
     *  @brief
     *  Delegate functions to load the allocated transport connections.
     *  The delegate application is responsible for creating and persisting these connections ( based on the Allocation commands ).
     *  These Load APIs would be used to load the pre-allocated transport connections context information into the cluster server
     *  list, at initialization. Once loaded, the cluster server would be serving Reads on these attributes. The list is updatable
     *  via the Add/Remove functions for the respective transport connections.
     *
     * @note
     *
     * The required buffers are managed by TransportConfigurationStorage, the delegate function is expected to populate the vector
     * correctly.
     */
    virtual CHIP_ERROR LoadCurrentConnections(std::vector<TransportConfigurationStorageWithFabricIndex> & currentConnections) = 0;

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    virtual CHIP_ERROR PersistentAttributesLoadedCallback() = 0;
};

class PushAvStreamTransportServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * Creates a Push AV Stream Transport server instance. The Init() function needs to be called for this instance to be registered
     * and called by the interaction model at the appropriate times.
     * @param aDelegate    A reference to the delegate to be used by this server.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeatures   The bitflags value that identifies which features are supported by this instance.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    PushAvStreamTransportServer(PushAvStreamTransportDelegate & delegate, EndpointId endpointId, const BitFlags<Feature> aFeatures);

    ~PushAvStreamTransportServer() override;

    /**
     * @brief Initialise the Push AV Stream Transport server instance.
     * This function must be called after defining an PushAvStreamTransportServer class object.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap or if the
     * CommandHandler or AttributeHandler registration fails, else returns CHIP_NO_ERROR.
     * This method also checks if the feature setting is valid, if invalid it will return CHIP_ERROR_INVALID_ARGUMENT.
     */
    CHIP_ERROR Init();

    /**
     * @brief
     *   Unregisters the command handler and attribute interface, releasing resources.
     */
    void Shutdown();

    bool HasFeature(Feature feature) const;

    // Attribute Getters

private:
    enum class UpsertResultEnum : uint8_t
    {
        kInserted = 0x00,
        kUpdated  = 0x01,
    };

    PushAvStreamTransportDelegate & mDelegate;

    const BitFlags<Feature> mFeatures;

    // Attributes
    std::vector<SupportedFormatStruct> mSupportedFormats;
    /*Moved from TransportConfigurationStruct to TransportConfigurationStructWithFabricIndex
     * to perform fabric index checks
     */
    std::vector<TransportConfigurationStorageWithFabricIndex> mCurrentConnections;

    /**
     * IM-level implementation of read
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();

    // Helpers to read list items via delegate APIs
    CHIP_ERROR ReadAndEncodeCurrentConnections(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadAndEncodeSupportedFormats(const AttributeValueEncoder::ListEncodeHelper & encoder);

    // Helper functions
    uint16_t GenerateConnectionID();
    TransportConfigurationStorageWithFabricIndex * FindStreamTransportConnection(const uint16_t connectionID);
    // Add/Remove Management functions for transport
    UpsertResultEnum UpsertStreamTransportConnection(const TransportConfigurationStorageWithFabricIndex & transportConfiguration);

    void RemoveStreamTransportConnection(const uint16_t connectionID);

    /**
     * @brief Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;

    void HandleAllocatePushTransport(HandlerContext & ctx, const Commands::AllocatePushTransport::DecodableType & req);

    void HandleDeallocatePushTransport(HandlerContext & ctx, const Commands::DeallocatePushTransport::DecodableType & req);

    void HandleModifyPushTransport(HandlerContext & ctx, const Commands::ModifyPushTransport::DecodableType & req);

    void HandleSetTransportStatus(HandlerContext & ctx, const Commands::SetTransportStatus::DecodableType & req);

    void HandleManuallyTriggerTransport(HandlerContext & ctx, const Commands::ManuallyTriggerTransport::DecodableType & req);

    void HandleFindTransport(HandlerContext & ctx, const Commands::FindTransport::DecodableType & req);
};

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
