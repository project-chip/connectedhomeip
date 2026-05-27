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

// Codegen backward-compat Instance wrappers for OperationalState-derived clusters.
// New code should construct OperationalStateCluster (or a derived class) directly and
// register it with the ServerClusterInterfaceRegistry.

#include "OperationalStateDelegate.h"
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

/**
 * Backward-compat lifecycle wrapper around OperationalStateCluster.
 *
 * Provides Init()/Shutdown() for registering/unregistering the cluster with the
 * CodegenDataModelProvider, and forwards the full application-facing API to the
 * underlying cluster object.
 *
 * @note New applications should construct OperationalStateCluster directly.
 */
class InstanceBase
{
public:
    ~InstanceBase();

    CHIP_ERROR Init();
    void Shutdown();

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

    OperationalStateCluster & Cluster() { return mCluster; }
    const OperationalStateCluster & Cluster() const { return mCluster; }

    bool mRegistered = false;

private:
    Delegate * mDelegate;
    OperationalStateCluster & mCluster;
    ServerClusterRegistration * mRegPtr;
};

// detail:: base classes guarantee that the cluster + registration storage is initialized
// before InstanceBase receives references to them (base classes initialize before members).
namespace detail {
struct OperationalInstanceBase
{
    RegisteredServerCluster<OperationalStateCluster> mCluster;
    OperationalInstanceBase(Delegate * aDelegate, EndpointId aEndpointId, const OperationalStateCluster::Config & config = {}) :
        mCluster(aEndpointId, aDelegate, config)
    {}
};
} // namespace detail

class Instance : private detail::OperationalInstanceBase, public InstanceBase
{
public:
    Instance(Delegate * aDelegate, EndpointId aEndpointId, const OperationalStateCluster::Config & config = {}) :
        detail::OperationalInstanceBase(aDelegate, aEndpointId, config),
        InstanceBase(detail::OperationalInstanceBase::mCluster.Cluster(), detail::OperationalInstanceBase::mCluster.Registration(),
                     aDelegate)
    {}
};

} // namespace OperationalState

namespace RvcOperationalState {

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
                                       detail::RvcInstanceBase::mCluster.Registration(), aDelegate)
    {}
};

} // namespace RvcOperationalState

namespace OvenCavityOperationalState {

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
