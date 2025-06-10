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

#include <app/data-model-provider/MetadataTypes.h>
#include <data-model-providers/endpoint/EndpointProviderInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

/**
 * @brief Represents a registration entry for an EndpointProviderInterface within the EndpointProviderRegistry.
 *
 * This struct acts as a node in a singly-linked list used by the EndpointProviderRegistry.
 * It contains a pointer to the actual EndpointProviderInterface and a pointer to the next
 * registration in the list.
 *
 * Callers are responsible for ensuring that both this registration object and the
 * EndpointProviderInterface it points to outlive their registration with the EndpointProviderRegistry.
 */
struct EndpointProviderRegistration
{
    EndpointProviderInterface * const endpointProviderInterface;
    EndpointProviderRegistration * next;

    constexpr EndpointProviderRegistration(EndpointProviderInterface & interface,
                                           EndpointProviderRegistration * next_item = nullptr) :
        endpointProviderInterface(&interface),
        next(next_item)
    {}
    EndpointProviderRegistration(EndpointProviderRegistration && other) = default;

    EndpointProviderRegistration(const EndpointProviderRegistration & other)             = delete;
    EndpointProviderRegistration & operator=(const EndpointProviderRegistration & other) = delete;
};

/**
 * @brief Manages a collection of EndpointProviderInterface instances.
 *
 * The EndpointProviderRegistry can be used to discover and interact programmatically
 * with Matter endpoints. It maintains a linked list of EndpointProviderRegistration
 * objects.
 *
 * Responsibilities:
 * - Allows registration and unregistration of endpoint providers.
 * - Provides a way to retrieve a specific endpoint provider by its EndpointId.
 * - Offers an iterator to traverse all registered endpoint providers.
 *
 * Lifetime Management:
 * - The registry stores raw pointers to EndpointProviderInterface and EndpointProviderRegistration objects.
 * - It does NOT take ownership of these objects.
 * - Callers MUST ensure that any registered EndpointProviderInterface and its corresponding
 *   EndpointProviderRegistration object outlive the EndpointProviderRegistry or are unregistered
 *   before being destroyed.
 */
class EndpointProviderRegistry
{
public:
    class Iterator
    {
    public:
        explicit Iterator(EndpointProviderRegistration * registration) : mCurrent(registration) {}

        Iterator & operator++()
        {
            mCurrent = (mCurrent ? mCurrent->next : nullptr);
            return *this;
        }
        bool operator==(const Iterator & other) const { return mCurrent == other.mCurrent; }
        bool operator!=(const Iterator & other) const { return mCurrent != other.mCurrent; }
        EndpointProviderInterface * operator*() { return mCurrent->endpointProviderInterface; }
        EndpointProviderInterface * operator->() { return mCurrent->endpointProviderInterface; }

    private:
        EndpointProviderRegistration * mCurrent;
    };

    /**
     * @brief Registers an endpoint provider.
     *
     * The provided `entry` (EndpointProviderRegistration) must not already be part of another list
     * (i.e., `entry.next` must be nullptr). The EndpointProviderInterface within the entry must
     * be valid and have a valid EndpointId.
     *
     * @param entry The EndpointProviderRegistration containing the provider to register.
     *              The lifetime of this object must be managed by the caller.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_INVALID_ARGUMENT if entry.next is not nullptr,
     *                                     entry.endpointProviderInterface is nullptr,
     *                                     or the endpoint ID is kInvalidEndpointId.
     *         CHIP_ERROR_DUPLICATE_KEY_ID if an endpoint with the same ID is already registered.
     */
    CHIP_ERROR Register(EndpointProviderRegistration & entry);

    /**
     * @brief Unregisters an endpoint provider with the given EndpointId.
     * @param endpointId The ID of the endpoint provider to unregister.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_NOT_FOUND if no provider with the given ID is found.
     *         CHIP_ERROR_INVALID_ARGUMENT if endpointId is kInvalidEndpointId.
     */
    CHIP_ERROR Unregister(EndpointId endpointId);

    /** @return A pointer to the EndpointProviderInterface for the given endpointId, or nullptr if not found. */
    EndpointProviderInterface * Get(EndpointId endpointId);
    Iterator begin() { return Iterator(mRegistrations); }
    Iterator end() { return Iterator(nullptr); }

private:
    EndpointProviderRegistration * mRegistrations = nullptr;
    EndpointProviderInterface * mCachedInterface  = nullptr;
    EndpointId mCachedEndpointId                  = kInvalidEndpointId;
};

} // namespace app
} // namespace chip
