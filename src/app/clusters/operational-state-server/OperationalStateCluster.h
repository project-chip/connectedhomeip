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

#include "OperationalStateDelegate.h"
#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <clusters/OperationalState/Metadata.h>
#include <clusters/OvenCavityOperationalState/Metadata.h>
#include <clusters/RvcOperationalState/Metadata.h>

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
     * Construct an OperationalState cluster for the given endpoint.
     *
     * @param endpointId        The endpoint on which this cluster exists.
     * @param delegate          Application delegate providing state lists and command callbacks.
     *                          The caller must ensure the delegate outlives this object.
     * @param config            Configuration including optional attributes to expose.
     */
    OperationalStateCluster(EndpointId endpointId, Delegate * delegate, const Config & config = {});

    ~OperationalStateCluster() override;

    // ---- Application-facing API (preserved from legacy Instance) ----

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
    /**
     * Constructor for derived cluster variants (RVC, OvenCavity) with a different cluster ID
     * and spec revision.
     */
    OperationalStateCluster(EndpointId endpointId, ClusterId clusterId, uint32_t revision, Delegate * delegate,
                            const Config & config = {});

    virtual bool IsDerivedClusterStatePauseCompatible(uint8_t aState) { return false; }
    virtual bool IsDerivedClusterStateResumeCompatible(uint8_t aState) { return false; }

    /**
     * Override to handle commands that are specific to a derived cluster (e.g. GoHome for RVC).
     * Return UnsupportedCommand for unrecognised commands.
     */
    virtual std::optional<DataModel::ActionReturnStatus>
    HandleDerivedClusterCommand(const DataModel::InvokeRequest & request, chip::TLV::TLVReader & input, CommandHandler * handler)
    {
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }

    void UpdateCountdownTime(bool fromDelegate);
    void UpdateCountdownTimeFromClusterLogic() { UpdateCountdownTime(/* fromDelegate = */ false); }

private:
    Delegate * mDelegate;
    const uint32_t mRevision;
    const Config mConfig;

    DataModel::Nullable<uint8_t> mCurrentPhase;
    uint8_t mOperationalState                 = 0;
    GenericOperationalError mOperationalError = to_underlying(ErrorStateEnum::kNoError);
    QuieterReportingAttribute<uint32_t> mCountdownTime{ DataModel::NullNullable };

    std::optional<DataModel::ActionReturnStatus> HandlePauseState(const DataModel::InvokeRequest & request,
                                                                  chip::TLV::TLVReader & input, CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleStopState(const DataModel::InvokeRequest & request,
                                                                 chip::TLV::TLVReader & input, CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleStartState(const DataModel::InvokeRequest & request,
                                                                  chip::TLV::TLVReader & input, CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleResumeState(const DataModel::InvokeRequest & request,
                                                                   chip::TLV::TLVReader & input, CommandHandler * handler);
};

/**
 * Legacy wrapper around OperationalStateCluster for backwards compatibility with existing applications
 * that construct Instance objects directly and call Init().
 *
 * NEW CODE should use OperationalStateCluster directly.
 *
 * InstanceBase holds all state and methods. The concrete Instance class (below) additionally
 * owns the cluster storage inline — no heap allocation.
 * Derived cluster Instances (RvcOperationalState::Instance, OvenCavityOperationalState::Instance)
 * inherit InstanceBase directly and supply their own cluster storage.
 */
class InstanceBase
{
public:
    virtual ~InstanceBase();

    CHIP_ERROR Init();
    void Shutdown();

    // Forwarding API — delegates to the underlying OperationalStateCluster.
    CHIP_ERROR SetCurrentPhase(const DataModel::Nullable<uint8_t> & aPhase) { return Cluster().SetCurrentPhase(aPhase); }
    CHIP_ERROR SetOperationalState(uint8_t aOpState) { return Cluster().SetOperationalState(aOpState); }
    DataModel::Nullable<uint8_t> GetCurrentPhase() const { return Cluster().GetCurrentPhase(); }
    uint8_t GetCurrentOperationalState() const { return Cluster().GetCurrentOperationalState(); }
    void GetCurrentOperationalError(GenericOperationalError & error) const { Cluster().GetCurrentOperationalError(error); }
    void UpdateCountdownTimeFromDelegate() { Cluster().UpdateCountdownTimeFromDelegate(); }
    void OnOperationalErrorDetected(const Structs::ErrorStateStruct::Type & aError)
    {
        Cluster().OnOperationalErrorDetected(aError);
    }
    void OnOperationCompletionDetected(uint8_t aCompletionErrorCode,
                                       const Optional<DataModel::Nullable<uint32_t>> & aTotalOperationalTime = NullOptional,
                                       const Optional<DataModel::Nullable<uint32_t>> & aPausedTime           = NullOptional)
    {
        Cluster().OnOperationCompletionDetected(aCompletionErrorCode, aTotalOperationalTime, aPausedTime);
    }
    void ReportOperationalStateListChange() { Cluster().ReportOperationalStateListChange(); }
    void ReportPhaseListChange() { Cluster().ReportPhaseListChange(); }
    bool IsSupportedPhase(uint8_t aPhase) { return Cluster().IsSupportedPhase(aPhase); }
    bool IsSupportedOperationalState(uint8_t aState) { return Cluster().IsSupportedOperationalState(aState); }

protected:
    InstanceBase(OperationalStateCluster & cluster, ServerClusterRegistration & registration, Delegate * aDelegate);

    OperationalStateCluster & Cluster() { return *mClusterPtr; }
    const OperationalStateCluster & Cluster() const { return *mClusterPtr; }

    bool mRegistered = false;

private:
    Delegate * mDelegate;
    OperationalStateCluster * mClusterPtr;
    ServerClusterRegistration * mRegPtr;
};

namespace detail {
struct OperationalInstanceBase
{
    RegisteredServerCluster<OperationalStateCluster> mCluster;
    OperationalInstanceBase(Delegate * aDelegate, EndpointId aEndpointId, const OperationalStateCluster::Config & config = {}) :
        mCluster(aEndpointId, aDelegate, config)
    {}
};
} // namespace detail

/**
 * Concrete Instance for standalone OperationalState cluster use. Owns the cluster storage inline
 * (no heap allocation), following the same embedded-storage pattern as RvcOperationalState::Instance
 * and OvenCavityOperationalState::Instance.
 */
class Instance : private detail::OperationalInstanceBase, public InstanceBase
{
public:
    Instance(Delegate * aDelegate, EndpointId aEndpointId, const OperationalStateCluster::Config & config = {}) :
        detail::OperationalInstanceBase(aDelegate, aEndpointId, config),
        InstanceBase(detail::OperationalInstanceBase::mCluster.Cluster(),
                     detail::OperationalInstanceBase::mCluster.Registration(), aDelegate)
    {}
    ~Instance() override = default;
};

} // namespace OperationalState
namespace RvcOperationalState {

class RvcOperationalStateCluster : public OperationalState::OperationalStateCluster
{
public:
    RvcOperationalStateCluster(EndpointId endpointId, Delegate * delegate,
                               const OperationalState::OperationalStateCluster::Config & config = {});

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

protected:
    bool IsDerivedClusterStatePauseCompatible(uint8_t aState) override;
    bool IsDerivedClusterStateResumeCompatible(uint8_t aState) override;

    std::optional<DataModel::ActionReturnStatus> HandleDerivedClusterCommand(const DataModel::InvokeRequest & request,
                                                                             chip::TLV::TLVReader & input,
                                                                             CommandHandler * handler) override;

private:
    Delegate * mRvcDelegate;

    std::optional<DataModel::ActionReturnStatus> HandleGoHomeCommand(const DataModel::InvokeRequest & request,
                                                                     chip::TLV::TLVReader & input, CommandHandler * handler);
};

namespace detail {
struct RvcInstanceBase
{
    RegisteredServerCluster<RvcOperationalStateCluster> mCluster;
    RvcInstanceBase(Delegate * aDelegate, EndpointId aEndpointId,
                    const OperationalState::OperationalStateCluster::Config & config = {}) :
        mCluster(aEndpointId, aDelegate, config)
    {}
};
} // namespace detail

class Instance : private detail::RvcInstanceBase, public OperationalState::InstanceBase
{
public:
    Instance(Delegate * aDelegate, EndpointId aEndpointId, const OperationalState::OperationalStateCluster::Config & config = {}) :
        detail::RvcInstanceBase(aDelegate, aEndpointId, config),
        OperationalState::InstanceBase(detail::RvcInstanceBase::mCluster.Cluster(),
                                       detail::RvcInstanceBase::mCluster.Registration(),
                                       static_cast<OperationalState::Delegate *>(aDelegate))
    {}
};

} // namespace RvcOperationalState

namespace OvenCavityOperationalState {

class OvenCavityOperationalStateCluster : public OperationalState::OperationalStateCluster
{
public:
    OvenCavityOperationalStateCluster(EndpointId endpointId, OperationalState::Delegate * delegate,
                                      const OperationalState::OperationalStateCluster::Config & config = {});

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
};

namespace detail {
struct OvenInstanceBase
{
    RegisteredServerCluster<OvenCavityOperationalStateCluster> mCluster;
    OvenInstanceBase(OperationalState::Delegate * aDelegate, EndpointId aEndpointId,
                     const OperationalState::OperationalStateCluster::Config & config = {}) :
        mCluster(aEndpointId, aDelegate, config)
    {}
};
} // namespace detail

class Instance : private detail::OvenInstanceBase, public OperationalState::InstanceBase
{
public:
    Instance(OperationalState::Delegate * aDelegate, EndpointId aEndpointId,
             const OperationalState::OperationalStateCluster::Config & config = {}) :
        detail::OvenInstanceBase(aDelegate, aEndpointId, config),
        OperationalState::InstanceBase(detail::OvenInstanceBase::mCluster.Cluster(),
                                       detail::OvenInstanceBase::mCluster.Registration(), aDelegate)
    {}
};

} // namespace OvenCavityOperationalState

} // namespace Clusters
} // namespace app
} // namespace chip
