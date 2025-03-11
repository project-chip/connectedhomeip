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
#include <data-model-providers/codegen/ServerClusterInterfaceRegistry.h>

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

    ConcreteClusterPath path = entry.serverClusterInterface->GetPath();

    VerifyOrReturnError(path.HasValidIds(), CHIP_ERROR_INVALID_ARGUMENT);

    // Double-checking for duplicates makes the checks O(n^2) on the total number of registered
    // items. We preserve this however we may want to make this optional at some point in time.
    VerifyOrReturnError(Get(path) == nullptr, CHIP_ERROR_DUPLICATE_KEY_ID);

    if (mContext.has_value())
    {
        ReturnErrorOnFailure(entry.serverClusterInterface->Startup(&*mContext));
    }

    entry.next     = mRegistrations;
    mRegistrations = &entry;

    return CHIP_NO_ERROR;
}

ServerClusterInterface * ServerClusterInterfaceRegistry::Unregister(const ConcreteClusterPath & path)
{
    ServerClusterRegistration * prev    = nullptr;
    ServerClusterRegistration * current = mRegistrations;

    while (current != nullptr)
    {
        if (current->serverClusterInterface->GetPath() == path)
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

            return current->serverClusterInterface;
        }

        prev    = current;
        current = current->next;
    }

    // Not found.
    return nullptr;
}

ServerClusterInterfaceRegistry::ClustersList ServerClusterInterfaceRegistry::ClustersOnEndpoint(EndpointId endpointId)
{
    return { mRegistrations, endpointId };
}

void ServerClusterInterfaceRegistry::UnregisterAllFromEndpoint(EndpointId endpointId)
{
    ServerClusterRegistration * prev    = nullptr;
    ServerClusterRegistration * current = mRegistrations;
    while (current != nullptr)
    {
        if (current->serverClusterInterface->GetPath().mEndpointId == endpointId)
        {
            if (mCachedInterface == current->serverClusterInterface)
            {
                mCachedInterface = nullptr;
            }
            if (prev == nullptr)
            {
                mRegistrations = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            ServerClusterRegistration * actual_next = current->next;

            current->next = nullptr; // Make sure current does not look like part of a list.
            if (mContext.has_value())
            {
                current->serverClusterInterface->Shutdown();
            }

            current = actual_next;
        }
        else
        {
            prev    = current;
            current = current->next;
        }
    }
}

ServerClusterInterface * ServerClusterInterfaceRegistry::Get(const ConcreteClusterPath & path)
{
    // Check the cache to speed things up
    if ((mCachedInterface != nullptr) && (mCachedInterface->GetPath() == path))
    {
        return mCachedInterface;
    }

    // The cluster searched for is not cached, do a linear search for it
    ServerClusterRegistration * current = mRegistrations;

    while (current != nullptr)
    {
        if (current->serverClusterInterface->GetPath() == path)
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
        CHIP_ERROR err = registration->serverClusterInterface->Startup(&*mContext);
        if (err != CHIP_NO_ERROR)
        {
#if CHIP_ERROR_LOGGING
            const ConcreteClusterPath path = registration->serverClusterInterface->GetPath();
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

} // namespace app
} // namespace chip
