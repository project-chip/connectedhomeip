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
#include "OvenCavityOperationalStateCluster.h"
#include "RvcOperationalStateCluster.h"
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/support/CHIPMem.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

// detail:: base classes guarantee that the cluster + registration storage is initialized
// before Instance receives references to them (base classes initialize before members).
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
 * Backward-compat lifecycle wrapper around OperationalStateCluster.
 *
 * Provides Init()/Shutdown() for registering/unregistering the cluster with the
 * CodegenDataModelProvider, and forwards the full application-facing API.
 *
 * RvcOperationalState::Instance and OvenCavityOperationalState::Instance derive from this,
 * so OperationalState::Instance * can hold any of the three cluster types.
 *
 * @note New applications should construct OperationalStateCluster directly.
 */
class Instance
{
public:
    /**
     * Standalone constructor: creates and owns an OperationalStateCluster for the given endpoint.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, const OperationalStateCluster::Config & config = {});
    ~Instance();

    CHIP_ERROR Init();
    void Shutdown();

    // Forwarders to the underlying cluster. Defined out-of-line in CodegenIntegration.cpp so the
    // cluster methods are not inlined (and duplicated) into each shim — keeps the back-compat layer small.
    CHIP_ERROR SetCurrentPhase(const DataModel::Nullable<uint8_t> & aPhase);
    CHIP_ERROR SetOperationalState(uint8_t aOpState);
    DataModel::Nullable<uint8_t> GetCurrentPhase() const;
    uint8_t GetCurrentOperationalState() const;
    void GetCurrentOperationalError(GenericOperationalError & error) const;
    void UpdateCountdownTimeFromDelegate();
    void OnOperationalErrorDetected(const Structs::ErrorStateStruct::Type & aError);
    void OnOperationCompletionDetected(uint8_t aCompletionErrorCode,
                                       const Optional<DataModel::Nullable<uint32_t>> & aTotalOperationalTime = NullOptional,
                                       const Optional<DataModel::Nullable<uint32_t>> & aPausedTime           = NullOptional);
    void ReportOperationalStateListChange();
    void ReportPhaseListChange();
    bool IsSupportedPhase(uint8_t aPhase);
    bool IsSupportedOperationalState(uint8_t aState);

protected:
    /**
     * Constructor for derived instances (Rvc, OvenCavity) that supply their own cluster storage.
     * The derived class must ensure the cluster and registration outlive this object.
     */
    Instance(OperationalStateCluster & cluster, ServerClusterRegistration & registration, Delegate * aDelegate);

    OperationalStateCluster & Cluster() { return mCluster; }
    const OperationalStateCluster & Cluster() const { return mCluster; }

    bool mRegistered = false;

private:
    Delegate * mDelegate;
    // mOwnedStorage declared before mCluster so it is initialized first in the standalone constructor,
    // allowing mCluster to be bound to the storage's cluster object. Held by Platform::UniquePtr so the
    // storage lifetime is managed automatically (no manual delete) and the type is non-copyable.
    Platform::UniquePtr<detail::OperationalInstanceBase> mOwnedStorage;
    OperationalStateCluster & mCluster;
    ServerClusterRegistration * mRegPtr;
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

class Instance : private detail::RvcInstanceBase, public OperationalState::Instance
{
public:
    Instance(Delegate * aDelegate, EndpointId aEndpointId, const OperationalState::OperationalStateCluster::Config & config = {}) :
        detail::RvcInstanceBase(aDelegate, aEndpointId, config),
        OperationalState::Instance(detail::RvcInstanceBase::mCluster.Cluster(), detail::RvcInstanceBase::mCluster.Registration(),
                                   aDelegate)
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

class Instance : private detail::OvenInstanceBase, public OperationalState::Instance
{
public:
    Instance(OperationalState::Delegate * aDelegate, EndpointId aEndpointId,
             const OperationalState::OperationalStateCluster::Config & config = {}) :
        detail::OvenInstanceBase(aDelegate, aEndpointId, config),
        OperationalState::Instance(detail::OvenInstanceBase::mCluster.Cluster(), detail::OvenInstanceBase::mCluster.Registration(),
                                   aDelegate)
    {}
};

} // namespace OvenCavityOperationalState

} // namespace Clusters
} // namespace app
} // namespace chip
