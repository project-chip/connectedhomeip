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

/** @brief Custom debug request interceptors.
 *
 *    This class is specifically meant for registering custom Accessors that
 *    allow mini-handlers to process PigweedRPC read/writes separately from the cluster
 *    code. It is meant to be used by samples using this PigweedRPC services to allow the RPC
 *    interface to be used in more ways than simply for example, simulating writes from a Matter
 *    client at the IM level. Handlers registered here by applications should be attempted before any
 *    standard processing.
 */
class PigweedDebugAccessInterceptorRegistry
{
public:
    /**
     * Registers an attribute access inteceptor within this registry. Use `Unregister` to
     * unregister an interceptor that was previously registered.
     * @param attrOverride - the interceptor to be registered.
     */
    void Register(PigweedDebugAccessInterceptor * attrOverride) { mAccessors.insert(attrOverride); }

    void Unregister(PigweedDebugAccessInterceptor * attrOverride)
    {
        if (mAccessors.find(attrOverride) == mAccessors.end())
        {
            ChipLogError(Support, "Attempt to unregister accessor that is not registered.");
            return;
        }
        mAccessors.erase(attrOverride);
    }

    const std::set<PigweedDebugAccessInterceptor *> & GetAllAccessors() const { return mAccessors; }

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
