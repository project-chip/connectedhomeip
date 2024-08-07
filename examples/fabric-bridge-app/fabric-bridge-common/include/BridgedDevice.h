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

class BridgedDevice
{
public:
    /// Defines all attributes that we keep track of for a bridged device
    struct BridgedAttributes
    {
        std::string uniqueId;
        std::string vendorName;
        uint16_t vendorId = 0;
        std::string productName;
        uint16_t productId = 0;
        std::string nodeLabel;
        uint16_t hardwareVersion = 0;
        std::string hardwareVersionString;
        uint32_t softwareVersion = 0;
        std::string softwareVersionString;
    };

    BridgedDevice(chip::NodeId nodeId);
    virtual ~BridgedDevice() {}

    bool IsReachable();
    void SetReachable(bool reachable);

    inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
    inline chip::EndpointId GetEndpointId() { return mEndpointId; };
    inline chip::NodeId GetNodeId() { return mNodeId; };
    inline void SetParentEndpointId(chip::EndpointId id) { mParentEndpointId = id; };
    inline chip::EndpointId GetParentEndpointId() { return mParentEndpointId; };

    const BridgedAttributes & GetBridgedAttributes() const { return mAttributes; }
    void SetBridgedAttributes(const BridgedAttributes & value) { mAttributes = value; }

protected:
    bool mReachable;
    std::string mLocation;
    chip::NodeId mNodeId;
    chip::EndpointId mEndpointId;
    chip::EndpointId mParentEndpointId;
    std::string mZone;

    BridgedAttributes mAttributes;
};
