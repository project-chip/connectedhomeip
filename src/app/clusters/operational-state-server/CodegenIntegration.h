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

using LegacyDelegate = chip::app::Clusters::OperationalState::Delegate;

class InstanceDelegateWrapper : public OperationalStateCluster::Delegate
{
public:
    InstanceDelegateWrapper(LegacyDelegate * aDelegate = nullptr) { SetDelegate(aDelegate); }
    ~InstanceDelegateWrapper() override { SetDelegate(nullptr); }

    void SetInstance(Instance * aInstance)
    {
        // Identical check with OperationalState::Delegate, forcing a 1:1
        // between instance and delegate, so that every delegate knows exactly what instance it
        // belongs to and every instance is managed by one delegate only. Doing this here
        // since mDelegate may not yet be set (and that was the one doing the check before).
        VerifyOrDie(mInstance == nullptr || aInstance == nullptr || mInstance == aInstance);

        mInstance = aInstance;
        if (mDelegate != nullptr)
        {
            mDelegate->SetInstance(aInstance);
        }
    }

    void SetDelegate(LegacyDelegate * aDelegate)
    {
        // We expect a general invariant of mDelegate::instance == mInstance.
        //
        // At the same time though, delegates throughout try to ensure that delegates are not reset and always
        // travel through a "SetInstance" from null to a value.
        //
        // Enforce that the delegates actually match: when we receive the delegate,
        // either it has no instance yet (and we can set it to our underlying instance whatever it is)
        // or it is already set to the required instance anyway.
        //
        // What we try to avoid is the case of:
        //   - mDelegate instance is FOO
        //   - mInstance is BAR
        // then:
        //   - we would do SetInstance(nullptr) + SetInstance(mInstance), effectively switching FOO to BAR
        //     without erroring out.
        VerifyOrDie((aDelegate == nullptr) || (aDelegate->GetInstance() == nullptr) || (aDelegate->GetInstance() == mInstance));

        if (mDelegate != nullptr)
        {
            mDelegate->SetInstance(nullptr);
        }
        mDelegate = aDelegate;
        if (mDelegate != nullptr)
        {
            mDelegate->SetInstance(mInstance);
        }
    }

    LegacyDelegate * GetDelegate() const { return mDelegate; }

    DataModel::Nullable<uint32_t> GetCountdownTime() override
    {
        if (mDelegate != nullptr)
        {
            return mDelegate->GetCountdownTime();
        }
        return DataModel::NullNullable;
    }

    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) override
    {
        if (mDelegate != nullptr)
        {
            return mDelegate->GetOperationalStateAtIndex(index, operationalState);
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override
    {
        if (mDelegate != nullptr)
        {
            return mDelegate->GetOperationalPhaseAtIndex(index, operationalPhase);
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    void HandlePauseStateCallback(GenericOperationalError & err) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->HandlePauseStateCallback(err);
        }
    }

    void HandleResumeStateCallback(GenericOperationalError & err) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->HandleResumeStateCallback(err);
        }
    }

    void HandleStartStateCallback(GenericOperationalError & err) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->HandleStartStateCallback(err);
        }
    }

    void HandleStopStateCallback(GenericOperationalError & err) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->HandleStopStateCallback(err);
        }
    }

    void HandleGoHomeCommandCallback(GenericOperationalError & err) override
    {
        if (mDelegate != nullptr)
        {
            mDelegate->HandleGoHomeCommandCallback(err);
        }
        else
        {
            err.Set(to_underlying(ErrorStateEnum::kUnknownEnumValue));
        }
    }

private:
    Instance * mInstance       = nullptr;
    LegacyDelegate * mDelegate = nullptr;
};

struct OperationalInstanceBase
{
    InstanceDelegateWrapper mDelegateWrapper;
    RegisteredServerCluster<OperationalStateCluster> mCluster;
    OperationalInstanceBase(LegacyDelegate * aDelegate, EndpointId aEndpointId,
                            const OperationalStateCluster::Config & config = {}) :
        mDelegateWrapper(aDelegate),
        mCluster(aEndpointId, mDelegateWrapper, config)
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
    virtual ~Instance();

    CHIP_ERROR Init();
    void Shutdown();

    void SetDelegate(Delegate * aDelegate);
    Delegate * GetDelegate() const;

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
    Instance(OperationalStateCluster & cluster, ServerClusterRegistration & registration,
             detail::InstanceDelegateWrapper & delegateWrapper, Delegate * aDelegate);

    OperationalStateCluster & Cluster() { return mCluster; }
    const OperationalStateCluster & Cluster() const { return mCluster; }

    bool mRegistered = false;

private:
    // mOwnedStorage declared first so it is initialized first in the standalone constructor,
    // allowing mCluster, mRegPtr, and mDelegateWrapperPtr to be bound to the storage's objects.
    // Held by Platform::UniquePtr so the storage lifetime is managed automatically (no manual delete)
    // and the type is non-copyable.
    Platform::UniquePtr<detail::OperationalInstanceBase> mOwnedStorage;
    OperationalStateCluster & mCluster;
    ServerClusterRegistration * mRegPtr;
    detail::InstanceDelegateWrapper * mDelegateWrapperPtr;
};

} // namespace OperationalState

namespace RvcOperationalState {

namespace detail {
struct RvcInstanceBase
{
    OperationalState::detail::InstanceDelegateWrapper mDelegateWrapper;
    RegisteredServerCluster<RvcOperationalStateCluster> mCluster;
    RvcInstanceBase(Delegate * aDelegate, EndpointId aEndpointId,
                    const OperationalState::OperationalStateCluster::Config & config = {}) :
        mDelegateWrapper(aDelegate),
        mCluster(aEndpointId, mDelegateWrapper, config)
    {}
};
} // namespace detail

class Instance : private detail::RvcInstanceBase, public OperationalState::Instance
{
public:
    Instance(Delegate * aDelegate, EndpointId aEndpointId, const OperationalState::OperationalStateCluster::Config & config = {}) :
        detail::RvcInstanceBase(aDelegate, aEndpointId, config),
        OperationalState::Instance(detail::RvcInstanceBase::mCluster.Cluster(), detail::RvcInstanceBase::mCluster.Registration(),
                                   detail::RvcInstanceBase::mDelegateWrapper, aDelegate)
    {}
};

} // namespace RvcOperationalState

namespace OvenCavityOperationalState {

namespace detail {
struct OvenInstanceBase
{
    OperationalState::detail::InstanceDelegateWrapper mDelegateWrapper;
    RegisteredServerCluster<OvenCavityOperationalStateCluster> mCluster;
    OvenInstanceBase(OperationalState::Delegate * aDelegate, EndpointId aEndpointId,
                     const OperationalState::OperationalStateCluster::Config & config = {}) :
        mDelegateWrapper(aDelegate),
        mCluster(aEndpointId, mDelegateWrapper, config)
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
                                   detail::OvenInstanceBase::mDelegateWrapper, aDelegate)
    {}
};

} // namespace OvenCavityOperationalState

} // namespace Clusters
} // namespace app
} // namespace chip
