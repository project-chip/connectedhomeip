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

static constexpr size_t kMaxUrlLength       = 2000u;
static constexpr size_t kMaxCENCKeyLength   = 16u;
static constexpr size_t kMaxCENCKeyIDLength = 16u;

using SupportedFormatStruct                   = Structs::SupportedFormatStruct::Type;
using CMAFContainerOptionsStruct              = Structs::CMAFContainerOptionsStruct::Type;
using ContainerOptionsStruct                  = Structs::ContainerOptionsStruct::Type;
using TransportZoneOptionsStruct              = Structs::TransportZoneOptionsStruct::Type;
using TransportTriggerOptionsStruct           = Structs::TransportTriggerOptionsStruct::Type;
using TransportMotionTriggerTimeControlStruct = Structs::TransportMotionTriggerTimeControlStruct::Type;
using TransportOptionsStruct                  = Structs::TransportOptionsStruct::Type;
using TransportConfigurationStruct            = Structs::TransportConfigurationStruct::Type;
using StreamUsageEnum                         = chip::app::Clusters::Globals::StreamUsageEnum;

enum class PushAvStreamTransportStatusEnum : uint8_t
{
    kBusy    = 0x00,
    kIdle    = 0x01,
    kUnknown = 0x02
};

struct TransportTriggerOptionsStorage : public TransportTriggerOptionsStruct
{
    TransportTriggerOptionsStorage(){};

    TransportTriggerOptionsStorage(const TransportTriggerOptionsStorage & aTransportTriggerOptionsStorage)
    {
        *this = aTransportTriggerOptionsStorage;
    }

    TransportTriggerOptionsStorage & operator=(const TransportTriggerOptionsStorage & aTransportTriggerOptionsStorage)
    {
        triggerType = aTransportTriggerOptionsStorage.triggerType;

        mTransportZoneOptions = aTransportTriggerOptionsStorage.mTransportZoneOptions;

        motionZones = aTransportTriggerOptionsStorage.motionZones;

        // Rebind motionZones to point to the copied vector if it was set
        if (motionZones.HasValue() == true && motionZones.Value().IsNull() == false)
        {
            motionZones.SetValue(DataModel::MakeNullable(
                DataModel::List<const TransportZoneOptionsStruct>(mTransportZoneOptions.data(), mTransportZoneOptions.size())));
        }

        motionSensitivity = aTransportTriggerOptionsStorage.motionSensitivity;
        motionTimeControl = aTransportTriggerOptionsStorage.motionTimeControl;
        maxPreRollLen     = aTransportTriggerOptionsStorage.maxPreRollLen;

        return *this;
    }

    TransportTriggerOptionsStorage & operator=(const Structs::TransportTriggerOptionsStruct::DecodableType aTransportTriggerOptions)
    {
        triggerType = aTransportTriggerOptions.triggerType;

        auto & motionZonesList = aTransportTriggerOptions.motionZones;

        if (triggerType == TransportTriggerTypeEnum::kMotion && motionZonesList.HasValue())
        {
            if (motionZonesList.Value().IsNull() == false)
            {
                auto iter = motionZonesList.Value().Value().begin();

                while (iter.Next())
                {
                    auto & transportZoneOption = iter.GetValue();
                    mTransportZoneOptions.push_back(transportZoneOption);
                }

                motionZones.SetValue(DataModel::MakeNullable(
                    DataModel::List<const TransportZoneOptionsStruct>(mTransportZoneOptions.data(), mTransportZoneOptions.size())));
            }
            else
            {
                motionZones.Value().SetNull();
            }
        }
        else
        {
            motionZones.ClearValue();
        }

        motionSensitivity = aTransportTriggerOptions.motionSensitivity;
        motionTimeControl = aTransportTriggerOptions.motionTimeControl;
        maxPreRollLen     = aTransportTriggerOptions.maxPreRollLen;

        return *this;
    }

    TransportTriggerOptionsStorage(Structs::TransportTriggerOptionsStruct::DecodableType triggerOptions)
    {
        triggerType = triggerOptions.triggerType;

        auto & motionZonesList = triggerOptions.motionZones;

        if (triggerType == TransportTriggerTypeEnum::kMotion && motionZonesList.HasValue())
        {
            if (motionZonesList.Value().IsNull() == false)
            {
                auto iter = motionZonesList.Value().Value().begin();

                while (iter.Next())
                {
                    auto & transportZoneOption = iter.GetValue();
                    mTransportZoneOptions.push_back(transportZoneOption);
                }

                motionZones.SetValue(DataModel::MakeNullable(
                    DataModel::List<const TransportZoneOptionsStruct>(mTransportZoneOptions.data(), mTransportZoneOptions.size())));
            }
            else
            {
                motionZones.Value().SetNull();
            }
        }
        else
        {
            motionZones.ClearValue();
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
    CMAFContainerOptionsStorage(){};

    CMAFContainerOptionsStorage(const CMAFContainerOptionsStorage & aCMAFContainerOptionsStorage)
    {
        *this = aCMAFContainerOptionsStorage;
    }

    CMAFContainerOptionsStorage & operator=(const CMAFContainerOptionsStorage & aCMAFContainerOptionsStorage)
    {
        chunkDuration = aCMAFContainerOptionsStorage.chunkDuration;

        std::memcpy(mCENCKeyBuffer, aCMAFContainerOptionsStorage.mCENCKeyBuffer, sizeof(mCENCKeyBuffer));

        std::memcpy(mCENCKeyIDBuffer, aCMAFContainerOptionsStorage.mCENCKeyIDBuffer, sizeof(mCENCKeyIDBuffer));

        CENCKey = aCMAFContainerOptionsStorage.CENCKey;

        CENCKeyID = aCMAFContainerOptionsStorage.CENCKeyID;

        if (CENCKey.HasValue())
        {
            CENCKey.SetValue(ByteSpan(mCENCKeyBuffer, aCMAFContainerOptionsStorage.CENCKey.Value().size()));
        }

        metadataEnabled = aCMAFContainerOptionsStorage.metadataEnabled;

        if (CENCKeyID.HasValue())
        {
            CENCKeyID.SetValue(ByteSpan(mCENCKeyIDBuffer, aCMAFContainerOptionsStorage.CENCKeyID.Value().size()));
        }

        return *this;
    }

    CMAFContainerOptionsStorage & operator=(const Structs::CMAFContainerOptionsStruct::Type aCMAFContainerOptions)
    {
        chunkDuration = aCMAFContainerOptions.chunkDuration;

        CENCKey = aCMAFContainerOptions.CENCKey;

        CENCKeyID = aCMAFContainerOptions.CENCKeyID;

        if (CENCKey.HasValue())
        {
            MutableByteSpan CENCKeyBuffer(mCENCKeyBuffer);
            CopySpanToMutableSpan(aCMAFContainerOptions.CENCKey.Value(), CENCKeyBuffer);
            CENCKey.SetValue(CENCKeyBuffer);
        }

        metadataEnabled = aCMAFContainerOptions.metadataEnabled;

        if (CENCKeyID.HasValue())
        {
            MutableByteSpan CENCKeyIDBuffer(mCENCKeyIDBuffer);
            CopySpanToMutableSpan(aCMAFContainerOptions.CENCKeyID.Value(), CENCKeyIDBuffer);
            CENCKeyID.SetValue(CENCKeyIDBuffer);
        }

        return *this;
    }

    CMAFContainerOptionsStorage(Structs::CMAFContainerOptionsStruct::Type CMAFContainerOptions)
    {

        chunkDuration = CMAFContainerOptions.chunkDuration;

        if (CMAFContainerOptions.CENCKey.HasValue())
        {
            MutableByteSpan CENCKeyBuffer(mCENCKeyBuffer);
            CopySpanToMutableSpan(CMAFContainerOptions.CENCKey.Value(), CENCKeyBuffer);
            CENCKey.SetValue(CENCKeyBuffer);
        }
        else
        {
            CENCKey.ClearValue();
        }

        metadataEnabled = CMAFContainerOptions.metadataEnabled;

        if (CMAFContainerOptions.CENCKey.HasValue())
        {
            MutableByteSpan CENCKeyIDBuffer(mCENCKeyIDBuffer);
            CopySpanToMutableSpan(CMAFContainerOptions.CENCKeyID.Value(), CENCKeyIDBuffer);
            CENCKeyID.SetValue(CENCKeyIDBuffer);
        }
        else
        {
            CENCKeyID.ClearValue();
        }
    }

private:
    uint8_t mCENCKeyBuffer[kMaxCENCKeyLength];
    uint8_t mCENCKeyIDBuffer[kMaxCENCKeyIDLength];
};

struct ContainerOptionsStorage : public ContainerOptionsStruct
{
    ContainerOptionsStorage(){};

    ContainerOptionsStorage(const ContainerOptionsStorage & aContainerOptionsStorage) { *this = aContainerOptionsStorage; }

    ContainerOptionsStorage & operator=(const ContainerOptionsStorage & aContainerOptionsStorage)
    {
        containerType        = aContainerOptionsStorage.containerType;
        CMAFContainerOptions = aContainerOptionsStorage.CMAFContainerOptions;

        if (containerType == ContainerFormatEnum::kCmaf)
        {
            mCMAFContainerStorage = aContainerOptionsStorage.mCMAFContainerStorage;
            CMAFContainerOptions.SetValue(mCMAFContainerStorage);
        }

        return *this;
    }

    ContainerOptionsStorage & operator=(const Structs::ContainerOptionsStruct::DecodableType aContainerOptions)
    {
        containerType = aContainerOptions.containerType;

        if (containerType == ContainerFormatEnum::kCmaf)
        {
            mCMAFContainerStorage = aContainerOptions.CMAFContainerOptions.Value();
            CMAFContainerOptions.SetValue(mCMAFContainerStorage);
        }
        else
        {
            CMAFContainerOptions.ClearValue();
        }

        return *this;
    }

    ContainerOptionsStorage(Structs::ContainerOptionsStruct::DecodableType containerOptions)
    {
        containerType = containerOptions.containerType;

        if (containerType == ContainerFormatEnum::kCmaf)
        {
            mCMAFContainerStorage = containerOptions.CMAFContainerOptions.Value();
            CMAFContainerOptions.SetValue(mCMAFContainerStorage);
        }
        else
        {
            CMAFContainerOptions.ClearValue();
        }
    }

private:
    CMAFContainerOptionsStorage mCMAFContainerStorage;
};

struct TransportOptionsStorage : public TransportOptionsStruct
{
    TransportOptionsStorage(){};

    TransportOptionsStorage(const TransportOptionsStorage & aTransportOptionsStorage) { *this = aTransportOptionsStorage; }

    TransportOptionsStorage & operator=(const TransportOptionsStorage & aTransportOptionsStorage)
    {
        streamUsage   = aTransportOptionsStorage.streamUsage;
        videoStreamID = aTransportOptionsStorage.videoStreamID;
        audioStreamID = aTransportOptionsStorage.audioStreamID;
        endpointID    = aTransportOptionsStorage.endpointID;

        // Deep copy the URL buffer
        std::memcpy(mUrlBuffer, aTransportOptionsStorage.mUrlBuffer, kMaxUrlLength);
        url = MutableCharSpan(mUrlBuffer, aTransportOptionsStorage.url.size());

        // Copy internal storage objects
        mTriggerOptionsStorage = aTransportOptionsStorage.mTriggerOptionsStorage;
        triggerOptions         = mTriggerOptionsStorage;

        ingestMethod = aTransportOptionsStorage.ingestMethod;

        mContainerOptionsStorage = aTransportOptionsStorage.mContainerOptionsStorage;
        containerOptions         = mContainerOptionsStorage;

        expiryTime = aTransportOptionsStorage.expiryTime;

        return *this;
    }

    TransportOptionsStorage(Structs::TransportOptionsStruct::DecodableType transportOptions)
    {
        streamUsage   = transportOptions.streamUsage;
        videoStreamID = transportOptions.videoStreamID;
        audioStreamID = transportOptions.audioStreamID;
        endpointID    = transportOptions.endpointID;

        MutableCharSpan urlBuffer(mUrlBuffer);
        CopyCharSpanToMutableCharSpanWithTruncation(transportOptions.url, urlBuffer);
        url = urlBuffer;

        mTriggerOptionsStorage = transportOptions.triggerOptions;
        triggerOptions         = mTriggerOptionsStorage;

        ingestMethod = transportOptions.ingestMethod;

        mContainerOptionsStorage = transportOptions.containerOptions;
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
    TransportConfigurationStorage(){}

    TransportConfigurationStorage(const TransportConfigurationStorage & aTransportConfigurationStorage)
    {
        *this = aTransportConfigurationStorage;
    }

    TransportConfigurationStorage & operator=(const TransportConfigurationStorage & aTransportConfigurationStorage)
    {
        connectionID    = aTransportConfigurationStorage.connectionID;
        transportStatus = aTransportConfigurationStorage.transportStatus;
        fabricIndex     = aTransportConfigurationStorage.fabricIndex;

        mTransportOptionsPtr = aTransportConfigurationStorage.mTransportOptionsPtr;

        if (mTransportOptionsPtr)
        {
            transportOptions.SetValue(*mTransportOptionsPtr);
        }
        else
        {
            transportOptions.ClearValue();
        }

        return *this;
    }

    TransportConfigurationStorage(const uint16_t aConnectionID, std::shared_ptr<TransportOptionsStorage> aTransportOptionsPtr)
    {
        connectionID    = aConnectionID;
        transportStatus = TransportStatusEnum::kInactive;
        /*Store the pointer to keep buffer alive*/
        mTransportOptionsPtr = aTransportOptionsPtr;
        /*Convert Storage type to base type*/
        if (mTransportOptionsPtr)
        {
            transportOptions.SetValue(*mTransportOptionsPtr);
        }
        else
        {
            transportOptions.ClearValue();
        }
    }
    std::shared_ptr<TransportOptionsStorage> GetTransportOptionsPtr() const { return mTransportOptionsPtr; }
    void SetTransportOptionsPtr(std::shared_ptr<TransportOptionsStorage> aTransportOptionsPtr)
    {
        mTransportOptionsPtr = aTransportOptionsPtr;
        if (mTransportOptionsPtr)
        {
            transportOptions.SetValue(*mTransportOptionsPtr);
        }
        else
        {
            transportOptions.ClearValue();
        }
    }

private:
    std::shared_ptr<TransportOptionsStorage> mTransportOptionsPtr;
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

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    /**
     *   @brief Handle Command Delegate for stream transport allocation with the provided transport configuration option.
     *
     *   @param transportOptions[in]        represent the configuration options of the transport to be allocated
     *
     *   @param connectionID[in]            Indicates the connectionID to allocate.
     *
     *   @return Success if the allocation is successful and a PushTransportConnectionID was produced; otherwise, the command SHALL
     *   be rejected with Failure.
     *

     *   The delegate is expected to process the transport following transport options: URL :  Validate the URL
     *   StreamUsage,VideoStreamID,AudioStreamID for selection of Stream.
     *
     *   Allocate the transport and map it to the connectionID.
     *
     *   On Success TransportConfigurationStruct is sent as response by the server.
     */
    virtual Protocols::InteractionModel::Status AllocatePushTransport(const TransportOptionsStruct & transportOptions,
                                                                      const uint16_t connectionID) = 0;
    /**
     *   @brief Handle Command Delegate for Stream transport deallocation for the
     *   provided connectionID.
     *
     *   @param connectionID[in]        Indicates the connectionID to deallocate.
     *
     *   @return Success if the transport deallocation is successful; otherwise, the delegate is expected to return status code BUSY
     *   if the transport is currently uploading.
     *
     */
    virtual Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID) = 0;
    /**
     *   @brief Handle Command Delegate for Stream transport modification.
     *
     *   @param connectionID [in]           Indicates the connectionID of the stream transport to modify.
     *
     *   @param transportOptions [out]      represents the Transport Options to modify.
     *
     *   The buffers storing URL, Trigger Options, Motion Zones, Container Options is owned by the PushAVStreamTransport Server.
     *
     *   The allocated buffers are cleared and reassigned on success of ModifyPushTransport and deallocated on success of
     *   DeallocatePushTransport.
     *
     *   @return Success if the stream transport modification is successful; otherwise, the command SHALL be rejected with Failure.
     */
    virtual Protocols::InteractionModel::Status
    ModifyPushTransport(const uint16_t connectionID, const Structs::TransportOptionsStruct::DecodableType transportOptions) = 0;

    /**
     *   @brief Handle Command Delegate for Stream transport modification.
     *
     *   @param connectionIDList [in]       represent the list of connectionIDs for which new transport status to apply.
     *   @param transportStatus  [in]       represents the updated status of the connection(s).
     *
     *  If the transportStatus field is set to Inactive(1).Disable transmissions on the transport.Remove any queued items for
     *  upload.Cancel any uploads currently in progress.
     *
     *  Emit GeneratePushTransportEndEvent on stop.
     *
     *  Else If the TransportStatus field is set to Active(0).Enable transmissions on the transport.
     *  If the transports trigger type is Continuous,begin Transmission immediately.
     *
     *  Else If the transports trigger type is Command or Motion, and the underlying trigger is currently active and still within
     *  the Time Control Bounds,begin Transmission immediately.
     *
     *  Emit GeneratePushTransportBeginEvent on start.
     *
     *  @return Success if the stream transport status is successfully set; otherwise, the command SHALL be rejected with Failure.
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
     *   The delegate is expected to begin transmission using the timeControl values.
     *
     *   Emitting of PushTransportBegin event is handled by the server when delegate returns success.
     *
     *   The delegate should emit PushTransportEnd Event using GeneratePushTransportEndEvent() API when timeControl values indicates
     *   end of transmission.
     *
     *   @return Success if the stream transport trigger is successful; otherwise, the command SHALL be rejected with Failure.
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
     * @return Status::Success if the stream usage is valid; and Status::ResourceExhausted otherwise.
     */

    virtual Protocols::InteractionModel::Status
    ValidateBandwidthLimit(StreamUsageEnum streamUsage, const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                           const Optional<DataModel::Nullable<uint16_t>> & audioStreamId) = 0;

    /**
     * @brief Assign an existing Video Stream as per the camera's resource management and stream priority policies using requested
     * stream usage.
     *
     * @param[in] streamUsage    The desired usage type for the stream (e.g. live view, recording, etc.).
     * @param[in] videoStreamId  Identifier for the requested video stream.
     *
     * @return Assign the selected videoStreamID and return Status::Success. Return Status::InvalidStream if there are no allocated
     * VideoStream.
     */
    virtual Protocols::InteractionModel::Status SelectVideoStream(StreamUsageEnum streamUsage, uint16_t & videoStreamId) = 0;
    /**
     * @brief Assign an existing Audio Stream as per the camera's resource management and stream priority policies using requested
     * stream usage.
     *
     * @param[in] streamUsage    The desired usage type for the stream (e.g. live view, recording, etc.).
     * @param[in] audioStreamId  Identifier for the requested audio stream.
     *
     * @return Assign the selected audioStreamID and return Status::Success. Return Status::InvalidStream if there are no allocated
     * audioStream.
     */

    virtual Protocols::InteractionModel::Status SelectAudioStream(StreamUsageEnum streamUsage, uint16_t & audioStreamId) = 0;
    /**
     * @brief Validate that the videoStream corresponding to the videoStreamID is allocated.
     *
     * @param[in] videoStreamId  Identifier for the requested video stream.
     *
     * @return Status::Success if there is an allocated video stream. Return Status::InvalidStream if there are no allocated
     * video stream with videoStreamID.
     */

    virtual Protocols::InteractionModel::Status ValidateVideoStream(uint16_t videoStreamId) = 0;

    /**
     * @brief Validate that the audioStream corresponding to the audioStreamID is allocated.
     *
     * @param[in] audioStreamId  Identifier for the requested audio stream.
     *
     * @return Status::Success if there is an allocated audio stream. Return Status::InvalidStream if there are no allocated
     * audio stream with audioStreamID.
     */

    virtual Protocols::InteractionModel::Status ValidateAudioStream(uint16_t audioStreamId) = 0;

    /**
     * @brief Gets the status of the transport
     *
     * @param[in] connectionID Indicates the connectionID of the stream transport to check status
     *
     * @return busy if transport is uploading else idle
     */
    virtual PushAvStreamTransportStatusEnum GetTransportBusyStatus(const uint16_t connectionID) = 0;

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
    virtual CHIP_ERROR LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections) = 0;

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    virtual CHIP_ERROR PersistentAttributesLoadedCallback() = 0;

    // Send Push AV Stream Transport events
    Protocols::InteractionModel::Status
    GeneratePushTransportBeginEvent(const uint16_t connectionID, const TransportTriggerTypeEnum triggerType,
                                    const Optional<TriggerActivationReasonEnum> activationReason);
    Protocols::InteractionModel::Status GeneratePushTransportEndEvent(const uint16_t connectionID,
                                                                      const TransportTriggerTypeEnum triggerType,
                                                                      const Optional<TriggerActivationReasonEnum> activationReason);

protected:
    EndpointId mEndpointId = 0;
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

private:
    enum class UpsertResultEnum : uint8_t
    {
        kInserted = 0x00,
        kUpdated  = 0x01,
    };

    struct PushAVStreamTransportDeallocateCallbackContext
    {
        PushAvStreamTransportServer * instance;
        uint16_t connectionID;
    };

    std::vector<std::shared_ptr<PushAVStreamTransportDeallocateCallbackContext>> mtimerContexts;

    PushAvStreamTransportDelegate & mDelegate;

    const BitFlags<Feature> mFeatures;

    // Attributes
    std::vector<SupportedFormatStruct> mSupportedFormats;
    /*Moved from TransportConfigurationStruct to TransportConfigurationStructWithFabricIndex
     * to perform fabric index checks
     */
    std::vector<TransportConfigurationStorage> mCurrentConnections;

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
    CHIP_ERROR ReadAndEncodeCurrentConnections(const AttributeValueEncoder::ListEncodeHelper & encoder, FabricIndex fabricIndex);
    CHIP_ERROR ReadAndEncodeSupportedFormats(const AttributeValueEncoder::ListEncodeHelper & encoder);

    // Helper functions
    uint16_t GenerateConnectionID();

    TransportConfigurationStorage * FindStreamTransportConnection(const uint16_t connectionID);

    TransportConfigurationStorage * FindStreamTransportConnectionWithinFabric(const uint16_t connectionID, FabricIndex fabricIndex);

    // Add/Remove Management functions for transport
    UpsertResultEnum UpsertStreamTransportConnection(const TransportConfigurationStorage & transportConfiguration);

    void RemoveStreamTransportConnection(const uint16_t connectionID);

    static void PushAVStreamTransportDeallocateCallback(chip::System::Layer *, void * callbackContext);

    UpsertResultEnum UpsertTimerAppState(std::shared_ptr<PushAVStreamTransportDeallocateCallbackContext> timerAppState);

    void RemoveTimerAppState(const uint16_t connectionID);

    /**
     * @brief Schedule deallocate with a given timeout
     *
     * @param endpointId    endpoint where DoorLockServer is running
     * @param timeoutSec    timeout in seconds
     */
    void ScheduleTransportDeallocate(uint16_t connectionID, uint32_t timeoutSec);

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
