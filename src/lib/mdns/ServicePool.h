/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/mdns/platform/Mdns.h>

namespace chip {
namespace Mdns {

constexpr uint64_t kUndefinedNodeId = 0;

class ServicePool
{
public:
    class Entry
    {
    public:
        friend ServicePool;

        Entry();
        Entry(Entry && rhs);
        Entry & operator=(Entry && rhs);
        MdnsService mService;
        const uint64_t mNodeId;
        const uint64_t mFabricId;

        ~Entry();

    private:
        void Clear();

        Entry & MoveFrom(Entry && rhs);
        CHIP_ERROR Emplace(const MdnsService & service, uint64_t nodeId, uint64_t fabricId);
        CHIP_ERROR Emplace(MdnsService && service, uint64_t nodeId, uint64_t fabricId);

        bool mHasValue;
        bool mHasPending;
    };

    ServicePool() = default;

    /**
     * This method adds a service to the service pool.
     *
     * We'll create a deep copy of the service and make no assumption about its
     * lifecycle.
     *
     */
    CHIP_ERROR AddService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service);

    /**
     * This method adds a service to the service pool taking an r-value reference.
     *
     * We directly copy all the pointers inside the service and assumes them to be
     * allocated from the CHIP heap. Take caution when using this API.
     *
     */
    CHIP_ERROR AddService(uint64_t nodeId, uint64_t fabricId, MdnsService && service);

    /**
     * This method removes a service from the pool.
     *
     * Any dynamic memory in the service will be freed.
     *
     */
    CHIP_ERROR RemoveService(uint64_t nodeId, uint64_t fabricId);

    /**
     * This method updates the service with given node and fabric ID.
     *
     * We'll create a deep copy of the service and the dynamic memory in the old
     * service will be freed.
     *
     */
    CHIP_ERROR UpdateService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service);

    /**
     * This method removes a service from the pool.
     *
     * Any dynamic memory in the service will be freed.
     *
     */
    Entry * FindService(uint64_t nodeId, uint64_t fabricId);

    /**
     * This method performs a hash-map rehash to reduce collision
     *
     */
    void ReHash();

    /**
     * This method gives suggestions on when a re-hash is required.
     *
     * Calling ReHash when this function returns false is also valid.
     *
     */
    bool ShouldReHash() { return mLazyDeleteCount > kServicePoolCapacity / 2; }

    /**
     * This method clears the hash map.
     *
     * All the dynamic memory will be freed.
     *
     */
    void Clear();

    static constexpr size_t kServicePoolCapacity = 32;

    ~ServicePool() = default;

private:
    ServicePool(const ServicePool &) = delete;
    ServicePool & operator=(const ServicePool &) = delete;

    Entry & FindAvailableSlot(size_t hashValue);

    Entry mEntries[kServicePoolCapacity];
    size_t mLazyDeleteCount;
};

} // namespace Mdns
} // namespace chip
