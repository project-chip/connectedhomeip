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

#include "pw_status/status.h"
#include <pigweed/rpc_services/AccessInterceptor.h>
#include <set>

namespace chip {
namespace rpc {

/** @brief Custom write interception handler registry.
 *
 *    This class is specifically meant for registering custom Attribute Accessors that
 *    allow mini-AAI-handlers to process PigweedRPC read/writes separately from the cluster
 *    code. It is meant to be used by samples using this PigweedRPC Attributes service to
 *    allow the RPC interface to be used in more ways than simply simulating writes from a Matter
 *    client at the IM level. Handlers registered here by applications will be attempted before any
 *    standard processing of read/write would take place.
 */
class PigweedDebugAccessInterceptorRegistry
{
public:
    /**
     * Register an attribute access override.
     */
    void Register(PigweedDebugAccessInterceptor * attrOverride) { mAccessors.insert(attrOverride); }

    /**
     * Unregister an attribute access override (for example if the object
     * implementing PigweedDebugAccessInterceptor is being destroyed).
     */
    void Unregister(PigweedDebugAccessInterceptor * attrOverride)
    {
        if (mAccessors.find(attrOverride) == mAccessors.end())
        {
            ChipLogError(Support, "Attempt to unregister accessor that is not registered.");
            return;
        }
        mAccessors.erase(attrOverride);
    }

    /**
     *  Get all registered accessors.
     */
    std::set<PigweedDebugAccessInterceptor *> GetAllAccessors() { return mAccessors; }

    /**
     * Returns the singleton instance of the attribute accessor registory.
     */
    static PigweedDebugAccessInterceptorRegistry & Instance()
    {
        static PigweedDebugAccessInterceptorRegistry instance;
        return instance;
    }

private:
    std::set<PigweedDebugAccessInterceptor *> mAccessors;
};

} // namespace rpc
} // namespace chip
