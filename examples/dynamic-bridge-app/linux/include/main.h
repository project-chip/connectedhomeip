/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/Optional.h>

#include "ActionCluster.h"
#include "Device.h"
#include "GeneratedClusters.h"

#include <memory>

class Room
{
public:
    Room() = default;
    void SetName(std::string name) { mName = name; }
    const std::string & GetName() { return mName; }
    chip::app::Clusters::Actions::EndpointListTypeEnum GetType() { return mType; };
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
    chip::app::Clusters::Actions::EndpointListTypeEnum mType;
    std::vector<chip::EndpointId> mEndpoints;
};

static constexpr uint32_t kMaxRooms = 16;
extern Room gRooms[kMaxRooms];

Room * FindRoom(const std::string & name);

chip::Span<Action *> GetActionListInfo(chip::EndpointId parentId);

chip::Optional<chip::ClusterId> LookupClusterByName(const char * name);
std::unique_ptr<GeneratedCluster> CreateCluster(const char * name);
std::unique_ptr<GeneratedCluster> CreateCluster(chip::ClusterId id);
