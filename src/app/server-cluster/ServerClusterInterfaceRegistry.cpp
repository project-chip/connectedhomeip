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
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <optional>

namespace chip {
namespace app {

ServerClusterInterfaceRegistry::~ServerClusterInterfaceRegistry()
{
    while (mRegistrations != nullptr)
    {
        ServerClusterRegistration * next = mRegistrations->next;
        if (mContext.has_value())
        {
            mRegistrations->serverClusterInterface->Shutdown();
        }
        mRegistrations->next = nullptr;
        mRegistrations       = next;
    }
}

CHIP_ERROR ServerClusterInterfaceRegistry::Register(ServerClusterRegistration & entry)
{
    // we have no strong way to check if entry is already registered somewhere else, so we use "next" as some
    // form of double-check
    VerifyOrReturnError(entry.next == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(entry.serverClusterInterface != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    Span<const ConcreteClusterPath> paths = entry.serverClusterInterface->GetPaths();
    VerifyOrReturnError(!paths.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    for (const ConcreteClusterPath & path : paths)
    {
        VerifyOrReturnError(path.HasValidIds(), CHIP_ERROR_INVALID_ARGUMENT);

        // Double-checking for duplicates makes the checks O(n^2) on the total number of registered
        // items. We preserve this however we may want to make this optional at some point in time.
        VerifyOrReturnError(Get(path) == nullptr, CHIP_ERROR_DUPLICATE_KEY_ID);
    }

    if (mContext.has_value())
    {
        // To preserve similarity with SetContext, do not fail the register even Startup fails.
        // This will cause Shutdown to be called for both success and failed startups.
        LogErrorOnFailure(entry.serverClusterInterface->Startup(*mContext));
    }

    entry.next     = mRegistrations;
    mRegistrations = &entry;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ServerClusterInterfaceRegistry::Unregister(ServerClusterInterface * what)
{
    ServerClusterRegistration * prev    = nullptr;
    ServerClusterRegistration * current = mRegistrations;

    while (current != nullptr)
    {
        if (current->serverClusterInterface == what)
        {
            // take the item out of the current list and return it.
            ServerClusterRegistration * next = current->next;

            if (prev == nullptr)
            {
                mRegistrations = next;
            }
            else
            {
                prev->next = next;
            }

            if (mCachedInterface == current->serverClusterInterface)
            {
                mCachedInterface = nullptr;
            }

            current->next = nullptr; // Make sure current does not look like part of a list.
            if (mContext.has_value())
            {
                current->serverClusterInterface->Shutdown();
            }

            return CHIP_NO_ERROR;
        }

        prev    = current;
        current = current->next;
    }

    return CHIP_ERROR_NOT_FOUND;
}

ServerClusterInterface * ServerClusterInterfaceRegistry::Get(const ConcreteClusterPath & clusterPath)
{
    // Check the cache to speed things up
    if ((mCachedInterface != nullptr) && mCachedInterface->PathsContains(clusterPath))
    {
        return mCachedInterface;
    }

    // The cluster searched for is not cached, do a linear search for it
    ServerClusterRegistration * current = mRegistrations;

    while (current != nullptr)
    {
        if (current->serverClusterInterface->PathsContains(clusterPath))
        {
            mCachedInterface = current->serverClusterInterface;
            return mCachedInterface;
        }

        current = current->next;
    }

    // not found
    return nullptr;
}

CHIP_ERROR ServerClusterInterfaceRegistry::SetContext(ServerClusterContext && context)
{
    if (mContext.has_value())
    {
        // if there is no difference, do not re-initialize.
        VerifyOrReturnError(*mContext != context, CHIP_NO_ERROR);
        ClearContext();
    }

    mContext.emplace(std::move(context));
    bool had_failure = false;

    for (ServerClusterRegistration * registration = mRegistrations; registration != nullptr; registration = registration->next)
    {
        CHIP_ERROR err = registration->serverClusterInterface->Startup(*mContext);
        if (err != CHIP_NO_ERROR)
        {
#if CHIP_ERROR_LOGGING
            // Paths MUST contain at least one element. Log the first one for identification (even though there may be more)
            const ConcreteClusterPath path = registration->serverClusterInterface->GetPaths().front();
            ChipLogError(DataManagement, "Cluster %u/" ChipLogFormatMEI " startup failed: %" CHIP_ERROR_FORMAT, path.mEndpointId,
                         ChipLogValueMEI(path.mClusterId), err.Format());
#endif
            had_failure = true;
            // NOTE: this makes the object be in an awkward state:
            //       - cluster is not initialized
            //       - mContext is valid
            //       As a result, ::Shutdown on this cluster WILL be called even if startup failed.
        }
    }

    if (had_failure)
    {
        return CHIP_ERROR_HAD_FAILURES;
    }

    return CHIP_NO_ERROR;
}

void ServerClusterInterfaceRegistry::ClearContext()
{
    if (!mContext.has_value())
    {
        return;
    }
    for (ServerClusterRegistration * registration = mRegistrations; registration != nullptr; registration = registration->next)
    {
        registration->serverClusterInterface->Shutdown();
    }

    mContext.reset();
}

ServerClusterInterfaceRegistry::ServerClusterInstances ServerClusterInterfaceRegistry::AllServerClusterInstances()
{
    return { mRegistrations };
}

} // namespace app
} // namespace chip
