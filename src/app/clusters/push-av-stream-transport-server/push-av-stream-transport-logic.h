#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/push-av-stream-transport-server/constants.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-delegate.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-storage.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <protocols/interaction_model/StatusCode.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

class PushAvStreamTransportServerLogic
{
public:
    PushAvStreamTransportServerLogic(EndpointId aEndpoint, BitFlags<Feature> aFeatures);
    ~PushAvStreamTransportServerLogic();

    void SetDelegate(EndpointId aEndpoint, PushAvStreamTransportDelegate * delegate)
    {
        mDelegate = delegate;
        mDelegate->SetEndpointId(aEndpoint);
    }

    enum class UpsertResultEnum : uint8_t
    {
        kInserted = 0x00,
        kUpdated  = 0x01,
    };

    struct PushAVStreamTransportDeallocateCallbackContext
    {
        PushAvStreamTransportServerLogic * instance;
        uint16_t connectionID;
    };

    EndpointId mEndpointId = kInvalidEndpointId;

    std::vector<std::shared_ptr<PushAVStreamTransportDeallocateCallbackContext>> mTimerContexts;

    BitFlags<Feature> mFeatures;

    std::vector<SupportedFormatStruct> mSupportedFormats;

    std::vector<TransportConfigurationStorage> mCurrentConnections;

    CHIP_ERROR Init();

    void Shutdown();

    bool HasFeature(Feature feature) const;

    Protocols::InteractionModel::Status
    ValidateIncomingTransportOptions(const Structs::TransportOptionsStruct::DecodableType & transportOptions);

    std::optional<DataModel::ActionReturnStatus>
    HandleAllocatePushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                const Commands::AllocatePushTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleDeallocatePushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                  const Commands::DeallocatePushTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleModifyPushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                              const Commands::ModifyPushTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleSetTransportStatus(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                             const Commands::SetTransportStatus::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleManuallyTriggerTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                   const Commands::ManuallyTriggerTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus> HandleFindTransport(CommandHandler & handler,
                                                                     const ConcreteCommandPath & commandPath,
                                                                     const Commands::FindTransport::DecodableType & commandData);

    // Send Push AV Stream Transport events
    Protocols::InteractionModel::Status
    GeneratePushTransportBeginEvent(const uint16_t connectionID, const TransportTriggerTypeEnum triggerType,
                                    const Optional<TriggerActivationReasonEnum> activationReason);
    Protocols::InteractionModel::Status GeneratePushTransportEndEvent(const uint16_t connectionID,
                                                                      const TransportTriggerTypeEnum triggerType,
                                                                      const Optional<TriggerActivationReasonEnum> activationReason);

private:
    PushAvStreamTransportDelegate * mDelegate = nullptr;

    /// Convenience method that returns if the internal delegate is null and will log
    /// an error if the check returns true
    bool IsNullDelegateWithLogging(EndpointId endpointIdForLogging);

    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();

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
};
} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
