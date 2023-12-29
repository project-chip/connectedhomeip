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

#include "ICDRefreshKeyInfo.h"
#include <app/icd/client/CheckInDelegate.h>
#include <app/icd/client/ICDClientStorage.h>
#include <unordered_map> //TODO: update check_includes_config.py

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
    void OnRefreshKeyGenerate(const ICDClientInfo & clientInfo, uint8_t * keyData, uint8_t keyLength) override;
    CHIP_ERROR OnRefreshKeyRetrieve(const ScopedNodeId & nodeId, ICDRefreshKeyInfo & refreshKeyInfo) override;
    void OnRegistrationComplete(const ICDClientInfo & clientInfo) override;
    // Hash function for the map : {NodeId, ICDRefreshKeyInfo}
    struct HashFunction
    {
        size_t operator()(const ScopedNodeId & peer) const
        {
            size_t nodeIdHash    = std::hash<uint64_t>()(peer.GetNodeId());
            size_t fabricIdxHash = std::hash<uint64_t>()(peer.GetFabricIndex()) << 1;
            return nodeIdHash ^ fabricIdxHash;
        }
    };

private:
    ICDClientStorage * mpStorage = nullptr;
    unordered_map<ScopedNodeId, ICDRefreshKeyInfo, HashFunction> icdRefreshKeyMap;
};

} // namespace app
} // namespace chip
