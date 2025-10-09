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

#include <app/util/attribute-storage.h>

#include <cstdint>
#include <stdbool.h>
#include <stdint.h>

#include <functional>
#include <string>
#include <sys/types.h>
#include <vector>

class Device
{
public:
    static const int kDeviceNameSize     = 32;
    static const int kDeviceUniqueIdSize = 32;

    enum Changed_t
    {
        kChanged_Reachable            = 1u << 0,
        kChanged_Location             = 1u << 1,
        kChanged_Name                 = 1u << 2,
        kChanged_ConfigurationVersion = 1u << 3,
        kChanged_Last                 = kChanged_ConfigurationVersion,
    } Changed;

    Device(const char * szDeviceName, std::string szLocation);
    virtual ~Device() {}

    bool IsReachable();
    void SetReachable(bool aReachable);
    void SetName(const char * szDeviceName);
    void SetUniqueId(const char * szDeviceUniqueId);
    void SetLocation(std::string szLocation);
    void GenerateUniqueId();
    uint32_t GetConfigurationVersion();
    void SetConfigurationVersion(uint32_t configurationVersion);
    inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
    inline chip::EndpointId GetEndpointId() { return mEndpointId; };
    inline void SetParentEndpointId(chip::EndpointId id) { mParentEndpointId = id; };
    inline chip::EndpointId GetParentEndpointId() { return mParentEndpointId; };
    inline char * GetName() { return mName; };
    inline char * GetUniqueId() { return mUniqueId; };
    inline std::string GetLocation() { return mLocation; };
    inline std::string GetZone() { return mZone; };
    inline void SetZone(std::string zone) { mZone = zone; };

private:
    virtual void HandleDeviceChange(Device * device, Device::Changed_t changeMask) = 0;

protected:
    bool mReachable                         = false;
    char mName[kDeviceNameSize + 1]         = { 0 };
    char mUniqueId[kDeviceUniqueIdSize + 1] = { 0 };
    uint32_t mConfigurationVersion;
    std::string mLocation;
    chip::EndpointId mEndpointId;
    chip::EndpointId mParentEndpointId;
    std::string mZone;
};

class DeviceOnOff : public Device
{
public:
    enum Changed_t
    {
        kChanged_OnOff = kChanged_Last << 1,
    } Changed;

    DeviceOnOff(const char * szDeviceName, std::string szLocation);

    bool IsOn();
    void SetOnOff(bool aOn);
    void Toggle();

    using DeviceCallback_fn = std::function<void(DeviceOnOff *, DeviceOnOff::Changed_t)>;
    void SetChangeCallback(DeviceCallback_fn aChanged_CB);

private:
    void HandleDeviceChange(Device * device, Device::Changed_t changeMask);

private:
    bool mOn;
    DeviceCallback_fn mChanged_CB;
};

class DeviceSwitch : public Device
{
public:
    enum Changed_t
    {
        kChanged_NumberOfPositions = kChanged_Last << 1,
        kChanged_CurrentPosition   = kChanged_Last << 2,
        kChanged_MultiPressMax     = kChanged_Last << 3,
    } Changed;

    DeviceSwitch(const char * szDeviceName, std::string szLocation, uint32_t aFeatureMap);

    void SetNumberOfPositions(uint8_t aNumberOfPositions);
    void SetCurrentPosition(uint8_t aCurrentPosition);
    void SetMultiPressMax(uint8_t aMultiPressMax);

    inline uint8_t GetNumberOfPositions() { return mNumberOfPositions; };
    inline uint8_t GetCurrentPosition() { return mCurrentPosition; };
    inline uint8_t GetMultiPressMax() { return mMultiPressMax; };
    inline uint32_t GetFeatureMap() { return mFeatureMap; };

    using DeviceCallback_fn = std::function<void(DeviceSwitch *, DeviceSwitch::Changed_t)>;
    void SetChangeCallback(DeviceCallback_fn aChanged_CB);

private:
    void HandleDeviceChange(Device * device, Device::Changed_t changeMask);

private:
    uint8_t mNumberOfPositions;
    uint8_t mCurrentPosition;
    uint8_t mMultiPressMax;
    uint32_t mFeatureMap;
    DeviceCallback_fn mChanged_CB;
};

class DeviceTempSensor : public Device
{
public:
    enum Changed_t
    {
        kChanged_MeasurementValue = kChanged_Last << 1,
    } Changed;

    DeviceTempSensor(const char * szDeviceName, std::string szLocation, int16_t min, int16_t max, int16_t measuredValue);

    inline int16_t GetMeasuredValue() { return mMeasurement; };
    void SetMeasuredValue(int16_t measurement);

    using DeviceCallback_fn = std::function<void(DeviceTempSensor *, DeviceTempSensor::Changed_t)>;
    void SetChangeCallback(DeviceCallback_fn aChanged_CB);

    const int16_t mMin;
    const int16_t mMax;

private:
    void HandleDeviceChange(Device * device, Device::Changed_t changeMask);

private:
    int16_t mMeasurement;
    DeviceCallback_fn mChanged_CB;
};

class ComposedDevice : public Device
{
public:
    ComposedDevice(const char * szDeviceName, std::string szLocation) : Device(szDeviceName, szLocation){};

    using DeviceCallback_fn = std::function<void(ComposedDevice *, ComposedDevice::Changed_t)>;

    void SetChangeCallback(DeviceCallback_fn aChanged_CB);

private:
    void HandleDeviceChange(Device * device, Device::Changed_t changeMask);

private:
    DeviceCallback_fn mChanged_CB;
};

class DevicePowerSource : public Device
{
public:
    enum Changed_t
    {
        kChanged_BatLevel     = kChanged_Last << 1,
        kChanged_Description  = kChanged_Last << 2,
        kChanged_EndpointList = kChanged_Last << 3,
    } Changed;

    DevicePowerSource(const char * szDeviceName, std::string szLocation,
                      chip::BitFlags<chip::app::Clusters::PowerSource::Feature> aFeatureMap) :
        Device(szDeviceName, szLocation),
        mFeatureMap(aFeatureMap){};

    using DeviceCallback_fn = std::function<void(DevicePowerSource *, DevicePowerSource::Changed_t)>;
    void SetChangeCallback(DeviceCallback_fn aChanged_CB) { mChanged_CB = aChanged_CB; }

    void SetBatChargeLevel(uint8_t aBatChargeLevel);
    void SetDescription(std::string aDescription);
    void SetEndpointList(std::vector<chip::EndpointId> mEndpointList);

    inline uint32_t GetFeatureMap() { return mFeatureMap.Raw(); };
    inline uint8_t GetBatChargeLevel() { return mBatChargeLevel; };
    inline uint8_t GetOrder() { return mOrder; };
    inline uint8_t GetStatus() { return mStatus; };
    inline std::string GetDescription() { return mDescription; };
    std::vector<chip::EndpointId> & GetEndpointList() { return mEndpointList; }

private:
    void HandleDeviceChange(Device * device, Device::Changed_t changeMask);

private:
    uint8_t mBatChargeLevel  = 0;
    uint8_t mOrder           = 0;
    uint8_t mStatus          = 0;
    std::string mDescription = "Primary Battery";
    chip::BitFlags<chip::app::Clusters::PowerSource::Feature> mFeatureMap;
    DeviceCallback_fn mChanged_CB;
    // This is linux, vector is not going to kill us here and it's easier. Plus, post c++11, storage is contiguous with .data()
    std::vector<chip::EndpointId> mEndpointList;
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
