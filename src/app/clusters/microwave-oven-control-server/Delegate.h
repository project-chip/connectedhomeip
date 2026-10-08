/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

#include <app/clusters/microwave-oven-control-server/AppDelegate.h>
#include <lib/core/Optional.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cstdint>

namespace chip::app::Clusters::MicrowaveOvenControl {

class Instance;

/** @brief
 *  Defines methods for implementing application-specific logic for the MicrowaveOvenControl Cluster.
 */
class Delegate : public AppDelegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of a Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     * @note This method is for internal SDK use and should only be called by the `Instance` constructor and destructor.
     */
    void SetInstance(Instance * aInstance)
    {
        VerifyOrDie(mInstance == nullptr || aInstance == nullptr || mInstance == aInstance);
        mInstance = aInstance;
    }

private:
    Instance * mInstance = nullptr;

protected:
    /**
     * @brief Provides access to the const Instance pointer.
     * This method is placed in the protected section because it must be called by classes derived from the delegate.
     *
     * @return A const pointer to the Instance object associated with this delegate.
     */
    const Instance * GetInstance() const { return mInstance; }

    /**
     * @brief Provides access to the Instance pointer.
     * This method is placed in the protected section because it must be called by classes derived from the delegate.
     *
     * @return A pointer to the Instance object associated with this delegate.
     */
    Instance * GetInstance() { return mInstance; }
};

} // namespace chip::app::Clusters::MicrowaveOvenControl
