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

using MetadataOptionsStruct                   = Structs::MetadataOptionsStruct::Type;
using CMAFContainerOptionsStruct              = Structs::CMAFContainerOptionsStruct::Type;
using ContainerOptionsStruct                  = Structs::ContainerOptionsStruct::Type;
using TransportZoneOptionsStruct              = Structs::TransportZoneOptionsStruct::Type;
using TransportTriggerOptionsStruct           = Structs::TransportTriggerOptionsStruct::Type;
using TransportMotionTriggerTimeControlStruct = Structs::TransportMotionTriggerTimeControlStruct::Type;
using TransportOptionsStruct                  = Structs::TransportOptionsStruct::Type;
using TransportConfigurationStruct            = Structs::TransportConfigurationStruct::Type;

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
     *   @param connectionID[in]       Indicates the connectionID to allocate.
     *
     *   @param transportOptions[in]        represent the configuration options of the transport to be allocated
     *
     *   @param outTransportStatus[out]        represent the status of the transport allocation
     *
     *   @return Success if the allocation is successful and a PushTransportConnectionID was
     *   produced; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     */
    virtual Protocols::InteractionModel::Status AllocatePushTransport(uint16_t connectionID,
                                                                      const TransportOptionsStruct & transportOptions,
                                                                      TransportStatusEnum & outTransportStatus) = 0;
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
     *   @param outTransportOptions [out]      represents the Trigger Options to modify.
     *
     *   @return Success if the stream transport modification is successful; otherwise, the command SHALL be rejected with an
     * appropriate error.
     */
    virtual Protocols::InteractionModel::Status ModifyPushTransport(const uint16_t connectionID,
                                                                    const TransportOptionsStruct & outTransportOptions) = 0;

    /**
     *   @brief Handle Command Delegate for Stream transport modification.
     *
     *   @param connectionID [in]          Indicates the connectionID of the stream transport to set status for.
     *
     *   @param transportStatus [in]       represent the new transport status to apply.
     *
     *   @return Success if the stream transport status is successfully set; otherwise, the command SHALL be rejected with an
     * appropriate error.
     */
    virtual Protocols::InteractionModel::Status SetTransportStatus(const uint16_t connectionID,
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
                             const TransportMotionTriggerTimeControlStruct & timeControl) = 0;

    /**
     *   @brief Handle Command Delegate to get the Stream Options Configuration for the specified push transport.
     *
     *   @param connectionID  [in]     Indicates the allocated connectionID to get the Stream Options Configuration of.
     *
     *   @param outtransportConfigurations  [out]     Single item list of mapped transport configuration or list if connectionID is
     * NULL.
     *
     *   @return Success if the transport is already allocated; otherwise, the command SHALL be rejected with an appropriate
     *   error.
     *
     */
    virtual Protocols::InteractionModel::Status
    FindTransport(const Optional<DataModel::Nullable<uint16_t>> & connectionID,
                  DataModel::List<TransportConfigurationStruct> & outtransportConfigurations) = 0;

    /**
     *   @brief Delegate callback for notifying change in an attribute.
     *
     */
    virtual void OnAttributeChanged(AttributeId attributeId) = 0;

    /**
     *  Delegate functions to load the allocated transport connections.
     *  The delegate application is responsible for creating and persisting these connections ( based on the Allocation commands ).
     *  These Load APIs would be used to load the pre-allocated transport connections context information into the cluster server
     * list, at initialization. Once loaded, the cluster server would be serving Reads on these attributes. The list is updatable
     * via the Add/Remove functions for the respective transport connections.
     */
    virtual Protocols::InteractionModel::Status LoadCurrentConnections(std::vector<uint16_t> & currentConnections) = 0;

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    virtual Protocols::InteractionModel::Status PersistentAttributesLoadedCallback() = 0;

};

class PushAvStreamTransportServer : private AttributeAccessInterface, private CommandHandlerInterface
{
public:
    /**
     * Creates a Push AV Stream Transport server instance. The Init() function needs to be called for this instance to be registered
     * and called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aDelegate A reference to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    PushAvStreamTransportServer(EndpointId endpointId, PushAvStreamTransportDelegate & delegate, const BitFlags<Feature> aFeatures,
                                PersistentStorageDelegate & aPersistentStorage);

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
    BitMask<SupportedContainerFormatsBitmap> GetSupportedContainerFormats() const { return mSupportedContainerFormats; }
    BitMask<SupportedIngestMethodsBitmap> GetSupportedIngestMethods() const { return mSupportedIngestMethods; }

private:
    PushAvStreamTransportDelegate & mDelegate;
    const BitFlags<Feature> mFeature;

    // Attributes
    BitMask<SupportedContainerFormatsBitmap> mSupportedContainerFormats;
    BitMask<SupportedIngestMethodsBitmap> mSupportedIngestMethods;
    // lists
    std::vector<uint16_t> mCurrentConnections;

    // Utility function to set and persist attributes
    template <typename T>
    CHIP_ERROR SetAttributeIfDifferent(T & currentValue, const T & newValue, AttributeId attributeId, bool shouldPersist = true)
    {
        if (currentValue != newValue)
        {
            currentValue = newValue;
            auto path    = ConcreteAttributePath(mEndpointId, PushAvStreamTransport::Id, attributeId);
            if (shouldPersist)
            {
                ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(path, currentValue));
            }
            mDelegate.OnAttributeChanged(attributeId);
            MatterReportingAttributeChangeCallback(path);
        }
        return CHIP_NO_ERROR;
    }

    /**
     * IM-level implementation of read
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * IM-level implementation of write
     * @return appropriately mapped CHIP_ERROR if applicable
     */
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();

    // Helpers to read list items via delegate APIs
    CHIP_ERROR ReadAndEncodeCurrentConnections(const AttributeValueEncoder::ListEncodeHelper & encoder);

    // Helper functions
    bool FindStreamTransportConnection(const uint16_t connectionID);
    uint16_t GenerateConnectionID();

    // Add/Remove Management functions for transport
    CHIP_ERROR AddStreamTransportConnection(const uint16_t transportConnectionId);

    CHIP_ERROR RemoveStreamTransportConnection(const uint16_t transportConnectionId);

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
