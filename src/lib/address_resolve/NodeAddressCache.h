/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <map>
#include <queue>
#include <lib/core/NodeId.h>

namespace chip {
namespace AddressResolve {

class NodeAddressCache
{
public:
    static constexpr size_t kMaxCacheSize = 16;

    void CacheNode(const PeerId & peerId, const ResolveResult & result);
    bool Lookup(const PeerId & peerId, ResolveResult & result) const;
    CHIP_ERROR RemoveCachedNodeAddress(const PeerId & peerId);
    void Clear();
    size_t GetCacheSize() const { return mCache.size(); }

private:
    struct PeerIdComparator
    {
        bool operator()(const PeerId & lhs, const PeerId & rhs) const
        {
            if (lhs.GetCompressedFabricId() != rhs.GetCompressedFabricId())
                return lhs.GetCompressedFabricId() < rhs.GetCompressedFabricId();
            return lhs.GetNodeId() < rhs.GetNodeId();
        }
    };

    std::map<PeerId, ResolveResult, PeerIdComparator> mCache;
    std::queue<PeerId> mInsertionOrder;
};

} // namespace AddressResolve
} // namespace chip
