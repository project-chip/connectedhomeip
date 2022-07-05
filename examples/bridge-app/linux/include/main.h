/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

class Room
{
public:
    Room() = default;
    void SetName(std::string name) { mName = name; }
    const std::string & GetName() { return mName; }
    chip::app::Clusters::BridgedActions::EndpointListTypeEnum GetType() { return mType; };
    uint16_t GetEndpointListId() { return mEndpointListId; };
    chip::EndpointId * GetEndpointListData() { return mEndpoints.data(); };
    size_t GetEndpointListSize() { return mEndpoints.size(); };

    void AddEndpoint(chip::EndpointId ep) { mEndpoints.push_back(ep); }
    void RemoveEndpoint(chip::EndpointId ep)
    {
        auto it = std::find(mEndpoints.begin(), mEndpoints.end(), ep);
        if (it != mEndpoints.end())
            mEndpoints.erase(it);
    }

private:
    std::string mName;
    uint16_t mEndpointListId = 0;
    chip::app::Clusters::BridgedActions::EndpointListTypeEnum mType;
    std::vector<chip::EndpointId> mEndpoints;
};

static constexpr uint32_t kMaxRooms = 16;
extern Room gRooms[kMaxRooms];

Room * FindRoom(const std::string & name);
