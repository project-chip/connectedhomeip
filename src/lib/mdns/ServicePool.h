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

#include <array>
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
        MdnsService mService;

        uint64_t GetNodeId() { return mNodeId; }
        uint64_t GetFabricId() { return mFabricId; }

        ~Entry();

    private:
        Entry(Entry & rhs) = delete;
        Entry & operator=(Entry & rhs) = delete;
        Entry & operator=(Entry && rhs) = delete;

        void Clear();

        Entry & MoveFrom(Entry && rhs);
        CHIP_ERROR Emplace(const MdnsService & service, uint64_t nodeId, uint64_t fabricId);
        CHIP_ERROR Emplace(MdnsService && service, uint64_t nodeId, uint64_t fabricId);

        bool mHasValue;
        bool mIsTombstone;
        uint64_t mNodeId;
        uint64_t mFabricId;
    };

    ServicePool() = default;

    /**
     * This method adds a service to the service pool.
     *
     * We'll create a deep copy of the service and make no assumption about its
     * lifecycle.
     *
     * @retval CHIP_ERROR_INVALID_ARGUMENT  The text in the service is too long.
     * @retval CHIP_ERROR_NO_MEMORY         Cannot allocate memory to copy the service.
     * 
     */
    CHIP_ERROR AddService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service);

    /**
     * This method adds a service to the service pool taking an r-value reference.
     *
     * We directly copy all the pointers inside the service and assume them to be
     * allocated from the CHIP heap. Take caution when using this API.
     * After this call service will have null mTextEntries.
     *
     */
    CHIP_ERROR AddService(uint64_t nodeId, uint64_t fabricId, MdnsService && service);

    /**
     * This method removes a service from the pool.
     *
     * Any dynamic memory in the service will be freed.
     *
     * @retval CHIP_ERROR_INVALID_ARGUMENT  No service with matching node and fabricID is found.
     */
    CHIP_ERROR RemoveService(uint64_t nodeId, uint64_t fabricId);

    /**
     * This method updates the service with given node and fabric ID.
     *
     * We'll create a deep copy of the service and the dynamic memory in the old
     * service will be freed.
     *
     * @retval CHIP_ERROR_INVALID_ARGUMENT  No service with matching node and fabricID is found.
     *
     */
    CHIP_ERROR UpdateService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service);

    /**
     * This method finds a service from the pool.
     *
     * @retval nullptr  No service with matching node and fabricID is found.
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

    Entry & FindAvailableSlot(uint64_t nodeId);

    using EntryList = std::array<Entry, kServicePoolCapacity>;

    EntryList mEntries;
    size_t mLazyDeleteCount;
};

} // namespace Mdns
} // namespace chip
