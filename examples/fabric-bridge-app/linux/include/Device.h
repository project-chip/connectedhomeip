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

    enum Changed_t
    {
        kChanged_Reachable = 1u << 0,
        kChanged_Location  = 1u << 1,
        kChanged_Name      = 1u << 2,
        kChanged_Last      = kChanged_Name,
    } Changed;

    Device(const char * szDeviceName, std::string szLocation);
    virtual ~Device() {}

    bool IsReachable();
    void SetReachable(bool aReachable);
    void SetName(const char * szDeviceName);
    void SetLocation(std::string szLocation);
    inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
    inline chip::EndpointId GetEndpointId() { return mEndpointId; };
    inline void SetParentEndpointId(chip::EndpointId id) { mParentEndpointId = id; };
    inline chip::EndpointId GetParentEndpointId() { return mParentEndpointId; };
    inline char * GetName() { return mName; };
    inline std::string GetLocation() { return mLocation; };
    inline std::string GetZone() { return mZone; };
    inline void SetZone(std::string zone) { mZone = zone; };

private:
    virtual void HandleDeviceChange(Device * device, Device::Changed_t changeMask) = 0;

protected:
    bool mReachable;
    char mName[kDeviceNameSize];
    std::string mLocation;
    chip::EndpointId mEndpointId;
    chip::EndpointId mParentEndpointId;
    std::string mZone;
};

class EndpointListInfo
{
public:
    EndpointListInfo(uint16_t endpointListId, std::string name, chip::app::Clusters::Actions::EndpointListTypeEnum type);
    EndpointListInfo(uint16_t endpointListId, std::string name, chip::app::Clusters::Actions::EndpointListTypeEnum type,
                     chip::EndpointId endpointId);
    void AddEndpointId(chip::EndpointId endpointId);
    inline uint16_t GetEndpointListId() { return mEndpointListId; };
    std::string GetName() { return mName; };
    inline chip::app::Clusters::Actions::EndpointListTypeEnum GetType() { return mType; };
    inline chip::EndpointId * GetEndpointListData() { return mEndpoints.data(); };
    inline size_t GetEndpointListSize() { return mEndpoints.size(); };

private:
    uint16_t mEndpointListId = static_cast<uint16_t>(0);
    std::string mName;
    chip::app::Clusters::Actions::EndpointListTypeEnum mType = static_cast<chip::app::Clusters::Actions::EndpointListTypeEnum>(0);
    std::vector<chip::EndpointId> mEndpoints;
};

class Room
{
public:
    Room(std::string name, uint16_t endpointListId, chip::app::Clusters::Actions::EndpointListTypeEnum type, bool isVisible);
    inline void setIsVisible(bool isVisible) { mIsVisible = isVisible; };
    inline bool getIsVisible() { return mIsVisible; };
    inline void setName(std::string name) { mName = name; };
    inline std::string getName() { return mName; };
    inline chip::app::Clusters::Actions::EndpointListTypeEnum getType() { return mType; };
    inline uint16_t getEndpointListId() { return mEndpointListId; };

private:
    bool mIsVisible;
    std::string mName;
    uint16_t mEndpointListId;
    chip::app::Clusters::Actions::EndpointListTypeEnum mType;
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
