/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#include "operational-state-cluster-objects.h"
#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <clusters/OperationalState/Metadata.h>
#include <clusters/OvenCavityOperationalState/Metadata.h>
#include <clusters/RvcOperationalState/Metadata.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

static constexpr uint8_t DerivedClusterNumberSpaceStart = 0x40;
static constexpr uint8_t VendorNumberSpaceStart         = 0x80;

/**
 * OperationalStateCluster is the code-driven implementation of the Operational State cluster.
 *
 * It provides a base implementation for any derivation (e.g. RVC, OvenCavity).
 */
class OperationalStateCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<OperationalState::Attributes::CountdownTime::Id>;

    static constexpr uint8_t kMaxPhaseNameLength = 64;

    struct Config
    {
        OptionalAttributeSet optionalAttributes;
    };

    /**
     * Application delegate interface for OperationalStateCluster.
     *
     * Concrete delegates inherit from this and implement the pure-virtual methods.
     * The codegen backward-compat layer (OperationalState::Delegate in OperationalStateDelegate.h)
     * extends this with SetInstance/GetInstance for legacy Instance wrappers.
     */
    class Delegate
    {
    public:
        Delegate()          = default;
        virtual ~Delegate() = default;

        virtual app::DataModel::Nullable<uint32_t> GetCountdownTime() = 0;

        virtual CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) = 0;

        virtual CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) = 0;

        virtual void HandlePauseStateCallback(GenericOperationalError & err) = 0;

        virtual void HandleResumeStateCallback(GenericOperationalError & err) = 0;

        virtual void HandleStartStateCallback(GenericOperationalError & err) = 0;

        virtual void HandleStopStateCallback(GenericOperationalError & err) = 0;

        /**
         * Handle Command Callback in application: GoHome (RVC-specific).
         * Default implementation returns kUnknownEnumValue (unsupported). Override in RVC delegates.
         */
        virtual void HandleGoHomeCommandCallback(GenericOperationalError & err)
        {
            err.Set(to_underlying(ErrorStateEnum::kUnknownEnumValue));
        }
    };

    /**
     * Construct an OperationalState cluster for the given endpoint.
     *
     * @param endpointId        The endpoint on which this cluster exists.
     * @param delegate          Application delegate providing state lists and command callbacks.
     *                          The caller must ensure the delegate outlives this object.
     * @param config            Configuration including optional attributes to expose.
     */
    OperationalStateCluster(EndpointId endpointId, Delegate * delegate, const Config & config = {});

    ~OperationalStateCluster() override = default;

    // ---- Application-facing API ----

    CHIP_ERROR SetCurrentPhase(const DataModel::Nullable<uint8_t> & aPhase);
    CHIP_ERROR SetOperationalState(uint8_t aOpState);

    DataModel::Nullable<uint8_t> GetCurrentPhase() const { return mCurrentPhase; }
    uint8_t GetCurrentOperationalState() const { return mOperationalState; }
    void GetCurrentOperationalError(GenericOperationalError & error) const;

    void UpdateCountdownTimeFromDelegate() { UpdateCountdownTime(/* fromDelegate = */ true); }

    void OnOperationalErrorDetected(const Structs::ErrorStateStruct::Type & aError);
    void OnOperationCompletionDetected(uint8_t aCompletionErrorCode,
                                       const Optional<DataModel::Nullable<uint32_t>> & aTotalOperationalTime = NullOptional,
                                       const Optional<DataModel::Nullable<uint32_t>> & aPausedTime           = NullOptional);

    void ReportOperationalStateListChange();
    void ReportPhaseListChange();

    bool IsSupportedPhase(uint8_t aPhase);
    bool IsSupportedOperationalState(uint8_t aState);

    // ---- DefaultServerCluster overrides ----

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

protected:
    OperationalStateCluster(EndpointId endpointId, ClusterId clusterId, uint16_t revision, Delegate * delegate,
                            const Config & config = {});

    virtual bool IsDerivedClusterStatePauseCompatible(uint8_t aState) { return false; }
    virtual bool IsDerivedClusterStateResumeCompatible(uint8_t aState) { return false; }

    virtual std::optional<DataModel::ActionReturnStatus>
    HandleDerivedClusterCommand(const ConcreteCommandPath & path, chip::TLV::TLVReader & input, CommandHandler * handler)
    {
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }

    void UpdateCountdownTime(bool fromDelegate);
    void UpdateCountdownTimeFromClusterLogic() { UpdateCountdownTime(/* fromDelegate = */ false); }

    Delegate * GetDelegate() { return mDelegate; }

private:
    Delegate * mDelegate;
    // ClusterRevision is not a fixed constant for this class: it varies by the concrete (derived)
    // cluster. OperationalState and RvcOperationalState are at revision 3, while OvenCavityOperationalState
    // is at revision 2, so each derived class passes its own value through the protected constructor.
    const uint16_t mRevision;
    const Config mConfig;

    DataModel::Nullable<uint8_t> mCurrentPhase;
    uint8_t mOperationalState                 = 0;
    GenericOperationalError mOperationalError = to_underlying(ErrorStateEnum::kNoError);
    QuieterReportingAttribute<uint32_t> mCountdownTime{ DataModel::NullNullable };

    // Selects which of the two commands handled by each shared helper is being processed.
    enum class PauseOrResume : uint8_t
    {
        kPause,
        kResume
    };
    enum class StartOrStop : uint8_t
    {
        kStart,
        kStop
    };

    std::optional<DataModel::ActionReturnStatus> HandlePauseOrResumeState(const ConcreteCommandPath & path,
                                                                          chip::TLV::TLVReader & input, CommandHandler * handler,
                                                                          PauseOrResume action);
    std::optional<DataModel::ActionReturnStatus> HandleStartOrStopState(const ConcreteCommandPath & path,
                                                                        chip::TLV::TLVReader & input, CommandHandler * handler,
                                                                        StartOrStop action);
};

} // namespace OperationalState

} // namespace Clusters
} // namespace app
} // namespace chip
