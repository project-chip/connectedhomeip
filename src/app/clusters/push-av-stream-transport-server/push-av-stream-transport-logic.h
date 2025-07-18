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

class PushAvStreamTransportServerLogic
{
public:
    PushAvStreamTransportServerLogic(EndpointId aEndpoint, BitFlags<PushAvStreamTransport::Feature> aFeatures);
    ~PushAvStreamTransportServerLogic();

    void SetDelegate(EndpointId aEndpoint, PushAvStreamTransportDelegate * delegate)
    {
        mDelegate = delegate;
        if (mDelegate == nullptr)
        {
            ChipLogError(Zcl, "Push AV Stream Transport [ep=%d]: Trying to set delegate to null", aEndpoint);
            return;
        }
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

    BitFlags<PushAvStreamTransport::Feature> mFeatures;

    std::vector<PushAvStreamTransport::SupportedFormatStruct> mSupportedFormats;

    std::vector<PushAvStreamTransport::TransportConfigurationStorage> mCurrentConnections;

    CHIP_ERROR Init();

    void Shutdown();

    bool HasFeature(PushAvStreamTransport::Feature feature) const;

    Protocols::InteractionModel::Status ValidateIncomingTransportOptions(
        const PushAvStreamTransport::Structs::TransportOptionsStruct::DecodableType & transportOptions);

    std::optional<DataModel::ActionReturnStatus>
    HandleAllocatePushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                const PushAvStreamTransport::Commands::AllocatePushTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleDeallocatePushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                  const PushAvStreamTransport::Commands::DeallocatePushTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleModifyPushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                              const PushAvStreamTransport::Commands::ModifyPushTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleSetTransportStatus(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                             const PushAvStreamTransport::Commands::SetTransportStatus::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleManuallyTriggerTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                   const PushAvStreamTransport::Commands::ManuallyTriggerTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleFindTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                        const PushAvStreamTransport::Commands::FindTransport::DecodableType & commandData);

    // Send Push AV Stream Transport events
    Protocols::InteractionModel::Status
    GeneratePushTransportBeginEvent(const uint16_t connectionID, const PushAvStreamTransport::TransportTriggerTypeEnum triggerType,
                                    const Optional<PushAvStreamTransport::TriggerActivationReasonEnum> activationReason);
    Protocols::InteractionModel::Status
    GeneratePushTransportEndEvent(const uint16_t connectionID, const PushAvStreamTransport::TransportTriggerTypeEnum triggerType,
                                  const Optional<PushAvStreamTransport::TriggerActivationReasonEnum> activationReason);

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

    PushAvStreamTransport::TransportConfigurationStorage * FindStreamTransportConnection(const uint16_t connectionID);

    PushAvStreamTransport::TransportConfigurationStorage * FindStreamTransportConnectionWithinFabric(const uint16_t connectionID,
                                                                                                     FabricIndex fabricIndex);

    // Add/Remove Management functions for transport
    UpsertResultEnum
    UpsertStreamTransportConnection(const PushAvStreamTransport::TransportConfigurationStorage & transportConfiguration);

    void RemoveStreamTransportConnection(const uint16_t connectionID);

    static void PushAVStreamTransportDeallocateCallback(chip::System::Layer *, void * callbackContext);

    void RemoveTimerAppState(const uint16_t connectionID);

    /**
     * @brief Schedule deallocate with a given timeout
     *
     * @param endpointId    endpoint where DoorLockServer is running
     * @param timeoutSec    timeout in seconds
     */
    CHIP_ERROR ScheduleTransportDeallocate(uint16_t connectionID, uint32_t timeoutSec);
};

} // namespace Clusters
} // namespace app
} // namespace chip
