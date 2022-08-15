/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/support/Span.h>
#include <app-common/app-common/zap-generated/cluster-enums.h>

#include <stdbool.h>
#include <stdint.h>

#include "Clusters.h"

// Device types for dynamic endpoints: TODO Need a generated file from ZAP to define these!
// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGED_NODE 0x0013
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT_SWITCH 0x0103
// (taken from chip-devices.xml)
#define DEVICE_TYPE_ROOT_NODE 0x0016
// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGE 0x000e

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

// This represents a single logical device occupying one endpoint. A composed device consists of multiple
// Device objects that reference a tree.
class Device
{
public:
    Device(chip::Span<chip::DataVersion> dataVersions, chip::Span<EmberAfCluster> clusters, chip::Span<ClusterImpl *> clusterImpl,
           const chip::Span<const EmberAfDeviceType> & deviceTypeList, chip::EndpointId parentId);
    ~Device() = default;

    const chip::Span<chip::DataVersion> & versions() { return mDataVersions; }
    const chip::Span<const EmberAfDeviceType> & deviceTypes() { return mDeviceTypeList; }
    const EmberAfEndpointType * endpointType() { return &mEndpointType; }
    const chip::Span<ClusterImpl *> & clusters() { return mClusterImpl; }

    void SetEndpointId(chip::EndpointId id);
    chip::EndpointId GetEndpointId() { return mEndpointId; }
    chip::EndpointId GetParentEndpointId() { return mParentEndpointId; }

    const char * GetName();
    void SetName(const char * name);

private:
    chip::EndpointId mParentEndpointId;
    chip::EndpointId mEndpointId;
    chip::Span<chip::DataVersion> mDataVersions;
    chip::Span<EmberAfCluster> mClusters;
    chip::Span<ClusterImpl *> mClusterImpl;
    chip::Span<const EmberAfDeviceType> mDeviceTypeList;
    EmberAfEndpointType mEndpointType;
    const char * mDeviceName = "";
};

class Action
{
public:
    Action(uint16_t actionId, std::string name, chip::app::Clusters::Actions::ActionTypeEnum type, uint16_t endpointListId,
           uint16_t supportedCommands, chip::app::Clusters::Actions::ActionStateEnum status, bool isVisible);
    inline void setName(std::string name) { mName = name; };
    inline std::string getName() { return mName; };
    inline chip::app::Clusters::Actions::ActionTypeEnum getType() { return mType; };
    inline chip::app::Clusters::Actions::ActionStateEnum getStatus() { return mStatus; };
    inline uint16_t getActionId() { return mActionId; };
    inline uint16_t getEndpointListId() { return mEndpointListId; };
    inline uint16_t getSupportedCommands() { return mSupportedCommands; };
    inline void setIsVisible(bool isVisible) { mIsVisible = isVisible; };
    inline bool getIsVisible() { return mIsVisible; };

private:
    std::string mName;
    chip::app::Clusters::Actions::ActionTypeEnum mType;
    chip::app::Clusters::Actions::ActionStateEnum mStatus;
    uint16_t mActionId;
    uint16_t mEndpointListId;
    uint16_t mSupportedCommands;
    bool mIsVisible;
};

int AddDeviceEndpoint(Device * dev);
int RemoveDeviceEndpoint(Device * dev);
Device * FindDeviceEndpoint(chip::EndpointId id);
