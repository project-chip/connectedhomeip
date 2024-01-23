/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/icd/client/CheckInDelegate.h>
#include <app/icd/client/ICDClientStorage.h>
#include <unordered_map>

namespace chip {
namespace app {

using namespace std;

/// Callbacks for check in protocol
class DefaultCheckInDelegate : public CheckInDelegate
{
public:
    virtual ~DefaultCheckInDelegate() {}
    CHIP_ERROR Init(ICDClientStorage * storage);
    void OnCheckInComplete(const ICDClientInfo & clientInfo) override;
    ICDRefreshKeyInfo * OnKeyRefreshNeeded(ICDClientInfo & clientInfo, ICDClientStorage * clientStorage) override;
    void OnKeyRefreshDone(const ICDClientInfo & clientInfo, CHIP_ERROR aError) override;

private:
    struct HashFunction
    {
        size_t operator()(const ScopedNodeId & peer) const
        {
            size_t nodeIdHash    = std::hash<uint64_t>()(peer.GetNodeId());
            size_t fabricIdxHash = std::hash<uint64_t>()(peer.GetFabricIndex()) << 1;
            return nodeIdHash ^ fabricIdxHash;
        }
    };
    ICDClientStorage * mpStorage = nullptr;
    // Data structure used to store the ICDRefreshKeyInfo created for every node. Since the check-in delegate has to manage key
    // refresh requests from multiple servers, this is used to keep track of the memory allocated for each node so that it can
    // be freed up at the end of the key refresh process.
    unordered_map<ScopedNodeId, ICDRefreshKeyInfo *, HashFunction> mICDRefreshKeyMap;
};

} // namespace app
} // namespace chip
