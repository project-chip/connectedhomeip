/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/util/attribute-storage.h>

#include <stdbool.h>
#include <stdint.h>

#include <functional>
#include <string>
#include <vector>

class Device
{
public:
    static const int kDeviceNameSize = 32;

    Device(chip::NodeId nodeId);
    virtual ~Device() {}

    bool IsReachable();
    void SetReachable(bool reachable);
    void SetName(const char * name);
    void SetLocation(std::string location) { mLocation = location; };
    inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
    inline chip::EndpointId GetEndpointId() { return mEndpointId; };
    inline chip::NodeId GetNodeId() { return mNodeId; };
    inline void SetParentEndpointId(chip::EndpointId id) { mParentEndpointId = id; };
    inline chip::EndpointId GetParentEndpointId() { return mParentEndpointId; };
    inline char * GetName() { return mName; };
    inline std::string GetLocation() { return mLocation; };
    inline std::string GetZone() { return mZone; };
    inline void SetZone(std::string zone) { mZone = zone; };

protected:
    bool mReachable;
    char mName[kDeviceNameSize];
    std::string mLocation;
    chip::NodeId mNodeId;
    chip::EndpointId mEndpointId;
    chip::EndpointId mParentEndpointId;
    std::string mZone;
};
