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

// Codegen (backward-compat) delegate layer for OperationalState-derived clusters.
// New code should derive directly from OperationalStateCluster::Delegate.

#include "OperationalStateCluster.h"
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

class Instance;

/**
 * Backward-compat delegate for OperationalState clusters using the old Instance wrappers.
 * Extends OperationalStateCluster::Delegate with SetInstance/GetInstance for lifecycle binding.
 *
 * @note New applications should implement OperationalStateCluster::Delegate directly.
 */
class Delegate : public OperationalStateCluster::Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    /**
     * Binds this delegate to an Instance. Called internally by the Instance constructor.
     * @note For internal SDK use only.
     */
    void SetInstance(Instance * aInstance)
    {
        VerifyOrDie(mInstance == nullptr || aInstance == nullptr || mInstance == aInstance);
        mInstance = aInstance;
    }

protected:
    const Instance * GetInstance() const { return mInstance; }
    Instance * GetInstance() { return mInstance; }

private:
    Instance * mInstance = nullptr;
};

} // namespace OperationalState

namespace RvcOperationalState {

/**
 * Backward-compat delegate for RvcOperationalState cluster using the old Instance wrappers.
 * Provides default "unsupported" implementations for Start/Stop (not used by RVC) and
 * inherits the default "unsupported" GoHome from OperationalStateCluster::Delegate.
 */
class Delegate : public OperationalState::Delegate
{
public:
    void HandleStartStateCallback(OperationalState::GenericOperationalError & err) override
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnknownEnumValue));
    }

    void HandleStopStateCallback(OperationalState::GenericOperationalError & err) override
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnknownEnumValue));
    }
};

} // namespace RvcOperationalState

} // namespace Clusters
} // namespace app
} // namespace chip
