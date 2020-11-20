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
    ServicePool();

    CHIP_ERROR AddService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service, bool deepCopy = true);

    CHIP_ERROR RemoveService(uint64_t nodeId, uint64_t fabricId);

    CHIP_ERROR UpdateService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service);

    bool FindService(uint64_t nodeId, uint64_t fabricId, MdnsService ** service);

    void ReHash();

    bool ShouldReHash() { return mLazyDeleteCount > kServicePoolCapacity / 4; }

    void Clear();

    static constexpr size_t kServicePoolCapacity = 32;

private:
    struct ServicePoolEntry
    {
        MdnsService mService;
        uint64_t mNodeId;
        uint64_t mFabricId;
        bool mLazyDelete;
    };

    void ClearNoFree();

    ServicePool(const ServicePool &) = delete;
    ServicePool & operator=(const ServicePool &) = delete;

    ServicePoolEntry mEntries[kServicePoolCapacity];
    size_t mLazyDeleteCount;
};

} // namespace Mdns
} // namespace chip
