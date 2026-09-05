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
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/clusters/av-analysis-server/AvAnalysisCameraClient.h>
#include <app/clusters/av-analysis-server/AvAnalysisCameraInteraction.h>
#include <app/clusters/av-analysis-server/AvAnalysisStorage.h>
#include <app/clusters/av-analysis-server/AvAnalysisStreamTable.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <protocols/interaction_model/StatusCode.h>

#include <functional>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

class AvAnalysisDelegate;

// Callback type for notifying attribute changes
using MarkDirtyCallback = std::function<void(AttributeId)>;

class AvAnalysisServerLogic : public AvAnalysisCameraClient::Callback
{
public:
    /**
     * Creates a server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId               The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeatures                 The bitflags value that identifies which features are supported by this instance.
     * @param aSupportedAmbientContexts The set of Ambient Contexts that this server is capable of detecting
     * @param aMaxZones                 The maximum number of zones present on the server. Shall be Null if PerZoneSensitivity is
     * not set. Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param aMaxAnalysisStreamCount   The fixed value of the MaxAnalysisStreamCount attribute. Shall be non-zero if
     * RemoteContextDetection is set, and 0 otherwise.
     */
    AvAnalysisServerLogic(EndpointId aEndpointId, BitFlags<AvAnalysis::Feature> aFeatures,
                          const std::vector<Descriptor::Structs::SemanticTagStruct::Type> & aSupportedAmbientContexts,
                          DataModel::Nullable<uint8_t> aMaxZones, uint8_t aMaxAnalysisStreamCount = 0);
    ~AvAnalysisServerLogic();

    void SetDelegate(AvAnalysisDelegate * delegate)
    {
        mDelegate = delegate;
        if (mDelegate == nullptr)
        {
            ChipLogError(Zcl, "AvAnalysis: Trying to set delegate to null");
        }
    }

    void SetMarkDirtyCallback(MarkDirtyCallback callback) { mMarkDirtyCallback = std::move(callback); }

    /**
     * Sets the camera client used by a RemoteContextDetection instance to allocate/deallocate analysis
     * streams on the camera (not used with LocalContextDetection).
     */
    void SetCameraClient(AvAnalysisCameraClient * aCameraClient) { mCameraClient = aCameraClient; }

    void OnVideoStreamAllocated(Protocols::InteractionModel::Status aStatus, uint16_t aVideoStreamId) override;
    void OnVideoStreamDeallocated(Protocols::InteractionModel::Status aStatus, uint16_t aVideoStreamId) override;

    EndpointId mEndpointId = kInvalidEndpointId;

    BitFlags<AvAnalysis::Feature> mFeatures;

    // Definitions of class variables that represent the Cluster attributes
    const std::vector<Descriptor::Structs::SemanticTagStruct::Type> mSupportedAmbientContexts;
    std::vector<AvAnalysis::AmbientContextStorage> mActiveAmbientContextTriggers;
    uint8_t mMaxAnalysisStreamCount = 0;
    bool mTrackingEnabled           = false;
    DataModel::Nullable<uint8_t> mMaxZones;

    uint8_t GetCurrentAnalysisStreamCount() const { return mStreamTable.Count(); }

    CHIP_ERROR Init() { return CHIP_NO_ERROR; }

    CHIP_ERROR Startup(AttributePersistenceProvider & aAttributePersistenceProvider);

    // Handle any dynamic cleanup required prior to the destructor being called on an app shutdown.  To be invoked by
    // an app as part of its own shutdown sequence and prior to the destruction of the app/delegate.
    void Shutdown();

    bool HasFeature(AvAnalysis::Feature aFeature) const;

    // Returns the commands accepted depending on the Feature Flags that are set
    CHIP_ERROR AcceptedCommands(ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder);

    // Returns the commands generated depending on the Feature Flags that are set
    CHIP_ERROR GeneratedCommands(ReadOnlyBufferBuilder<CommandId> & builder);

    // Returns supported depending on the Feature Flags that are set
    CHIP_ERROR Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);

    // Helper functions for complex attributes
    CHIP_ERROR ReadAndEncodeSupportedAmbientContexts(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadAndEncodeActiveAmbientContextTriggers(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadAndEncodeAnalysisStreams(AttributeValueEncoder & aEncoder);

    // Attribute mutators
    CHIP_ERROR SetTrackingEnabled(bool aTrackingEnabled);

    // Command handlers
    std::optional<DataModel::ActionReturnStatus>
    HandleEnableContextTriggers(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                const AvAnalysis::Commands::EnableContextTriggers::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleDisableContextTriggers(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                 const AvAnalysis::Commands::DisableContextTriggers::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleEstablishAnalysisStream(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                  const AvAnalysis::Commands::EstablishAnalysisStream::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleActivateAnalysisStream(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                 const AvAnalysis::Commands::ActivateAnalysisStream::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleDeactivateAnalysisStream(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                   const AvAnalysis::Commands::DeactivateAnalysisStream::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleRemoveAnalysisStream(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                               const AvAnalysis::Commands::RemoveAnalysisStream::DecodableType & commandData);

    // Active context tracking and events
    CHIP_ERROR AnalysisSessionStart(uint16_t & aSessionId, const DataModel::Nullable<std::vector<uint16_t>> & aZoneList,
                                    ServerClusterContext * aContext);

    CHIP_ERROR InitialTriggeringContextDetected(uint16_t aSessionId,
                                                const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aTriggeringContext,
                                                ServerClusterContext * aContext);

    CHIP_ERROR NewContextDetected(uint16_t aSessionId, const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aNewContext,
                                  ServerClusterContext * aContext);

    CHIP_ERROR ContextNoLongerDetected(uint16_t aSessionId,
                                       const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aOldContext,
                                       ServerClusterContext * aContext);

    CHIP_ERROR AnalysisSessionEnd(uint16_t aSessionId, ServerClusterContext * aContext);

private:
    AvAnalysisDelegate * mDelegate                               = nullptr;
    AvAnalysisCameraClient * mCameraClient                       = nullptr;
    AttributePersistenceProvider * mAttributePersistenceProvider = nullptr;
    uint16_t mNextAnalysisSessionID                              = 0;
    std::vector<AvAnalysis::ActiveAmbientContextSession> mActiveSessions;

    // Backing store for the AnalysisStreams attribute; only initialized when RemoteContextDetection is set.
    AvAnalysis::AnalysisStreamTable mStreamTable;

    // The single camera-bound interaction that may be in flight
    AvAnalysis::CameraInteraction mCameraInteraction;

    /**
     * Applies a state transition to a stream entry and reports the AnalysisStreams attribute change.
     */
    void SetStreamState(AvAnalysis::AnalysisStreamEntry & aEntry, AvAnalysis::AnalysisStreamStateEnum aState);

    MarkDirtyCallback mMarkDirtyCallback;

    /**
     * Helper function for attribute handlers to mark the attribute as dirty
     */
    void MarkDirty(AttributeId aAttributeId);

    /**
     * Command sub-handlers
     */
    std::optional<DataModel::ActionReturnStatus>
    HandleLocalEnableContextTriggers(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                     const AvAnalysis::Commands::EnableContextTriggers::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleRemoteEnableContextTriggers(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                      const AvAnalysis::Commands::EnableContextTriggers::DecodableType & commandData);

    /*
     * Command and event handler helper methods
     */
    bool ZoneIDListContains(const DataModel::DecodableList<uint16_t> list, uint16_t value);
    bool IsContextPartOfActiveContextTriggers(const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aContext);

    /**
     * Helper functions to handle persistent data and the KVS.
     */
    CHIP_ERROR StoreActiveAmbientContextTriggers();
    CHIP_ERROR LoadActiveAmbientContextTriggers();
    CHIP_ERROR StoreAnalysisStreams();
    CHIP_ERROR LoadAnalysisStreams();
    CHIP_ERROR StoreTrackingEnabled();
    CHIP_ERROR LoadTrackingEnabled();
    void LoadPersistentAttributes();
};

} // namespace Clusters
} // namespace app
} // namespace chip
