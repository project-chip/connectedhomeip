/*
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

#include <app/data-model-provider/Context.h>
#include <app/data-model-provider/Provider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>

namespace chip {
namespace app {

/// Represents a runtime context for server cluster interfaces to interact
/// with the outside world such as:
///   - notify of state changes to trigger attribute reports
///   - emit events
///   - potentially interact/review global metadata
///
/// The context object is quite large in terms of exposed functionality and not all clusters use
/// all the information within, however a common context is used to minimize RAM overhead
/// for every cluster maintaining such a context.
struct ServerClusterContext
{
    DataModel::Provider * const provider                          = nullptr; /// underlying provider that the cluster operates in
    PersistentStorageDelegate * const storage                     = nullptr; /// read/write persistent storage
    DataModel::InteractionModelContext * const interactionContext = nullptr; /// outside-world communication

    bool operator!=(const ServerClusterContext & other) const
    {
        return (provider != other.provider)                     //
            || (interactionContext != other.interactionContext) //
            || (storage != other.storage);
    }
};

} // namespace app
} // namespace chip
