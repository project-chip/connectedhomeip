/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "Device.h"

#include <app/clusters/basic-information/CodegenIntegration.h>
#include <crypto/RandUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DefaultTimerDelegate.h>

#include <string>
#include <sys/types.h>

using namespace chip;
using namespace chip::app::Clusters::Actions;
using chip::Protocols::InteractionModel::Status;

// We cannot use the proxy since we need ember support.
// As a result we update the version at runtime by fetching the Basic Information cluster
// during processing.
class EmberBridgeVersionUpdate : public chip::app::Clusters::ConfigurationVersionDelegate
{
public:
    CHIP_ERROR IncreaseConfigurationVersion() override
    {
        auto cluster = app::Clusters::BasicInformation::GetClusterInstance();
        VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
        return cluster->IncreaseConfigurationVersion();
    }
};

static EmberBridgeVersionUpdate gEmberVersionUpdate;

Device::Device(const char * szDeviceName, std::string szLocation)
{
    chip::Platform::CopyString(mName, szDeviceName);
    chip::Platform::CopyString(mUniqueId, "");
    mLocation = szLocation;
}

Status Device::OnNodeLabelChanged(const std::string & newNodeLabel)
{
    VerifyOrReturnValue(mName != newNodeLabel, Status::Success);
    chip::Platform::CopyString(mName, newNodeLabel.c_str());
    // NOTE: WE do NOT call device change as that handles attribute chance callbacks and those
    //       are handled by the cluster code.
    // HandleDeviceChange(this, kChanged_Name);
    return chip::Protocols::InteractionModel::Status::Success;
}

app::ServerClusterRegistration &
Device::CreateBridgedDeviceInfo(chip::EndpointId endpointId,
                                chip::app::Clusters::BridgedDeviceBasicInformationCluster::RequiredData && required,
                                chip::app::Clusters::BridgedDeviceBasicInformationCluster::FixedData && fixed)
{
    VerifyOrDie(!mBridgedDevice.IsConstructed());

    static chip::app::DefaultTimerDelegate timerDelegate;

    mBridgedDevice.Create(endpointId, std::move(required), std::move(fixed),
                          app::Clusters::BridgedDeviceBasicInformationCluster::Context{
                              .parentVersionConfiguration = gEmberVersionUpdate,
                              .delegate                   = *this,
                              .timerDelegate              = timerDelegate,
                          });
    return mBridgedDevice.Registration();
}

bool Device::IsReachable()
{
    return mBridgedDevice.IsConstructed() && mBridgedDevice.Cluster().GetReachable();
}

void Device::SetReachable(bool aReachable)
{
    VerifyOrReturn(mBridgedDevice.IsConstructed());
    VerifyOrReturn(mBridgedDevice.Cluster().GetReachable() != aReachable);

    mBridgedDevice.Cluster().SetReachable(aReachable);
    ChipLogProgress(DeviceLayer, "Device[%s]: %s", mName, aReachable ? "ONLINE" : "OFFLINE");
}

void Device::SetName(const char * szName)
{
    ChipLogProgress(DeviceLayer, "Device[%s]: New Name=\"%s\"", mName, szName);
    chip::Platform::CopyString(mName, szName);

    if (mBridgedDevice.IsConstructed())
    {
        mBridgedDevice.Cluster().SetNodeLabel(CharSpan::fromCharString(szName));
    }
}

void Device::SetUniqueId(const char * szDeviceUniqueId)
{
    if (mBridgedDevice.IsConstructed())
    {
        // TODO: We could implement a version bump here if this functionality is required.
        ChipLogError(DeviceLayer, "Unique id is FIXED and cannot be changed after bridged device startup.") return;
        return;
    }

    chip::Platform::CopyString(mUniqueId, szDeviceUniqueId);
    ChipLogProgress(DeviceLayer, "Device[%s]: New UniqueId=\"%s\"", mName, mUniqueId);
}

void Device::SetLocation(std::string szLocation)
{
    bool changed = (mLocation.compare(szLocation) != 0);

    mLocation = szLocation;

    ChipLogProgress(DeviceLayer, "Device[%s]: Location=\"%s\"", mName, mLocation.c_str());

    if (changed)
    {
        HandleDeviceChange(this, kChanged_Location);
    }
}

void Device::GenerateUniqueId()
{
    // Ensure the buffer is zeroed out
    memset(mUniqueId, 0, kDeviceUniqueIdSize + 1);

    static const char kRandCharChoices[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // Prefix the generated value with "GEN-"
    memcpy(mUniqueId, "GEN-", 4);
    for (unsigned idx = 4; idx < kDeviceUniqueIdSize; idx++)
    {
        mUniqueId[idx] = kRandCharChoices[Crypto::GetRandU8() % (sizeof(kRandCharChoices) - 1)];
    }

    mUniqueId[kDeviceUniqueIdSize] = '\0'; // Ensure null-termination
}

uint32_t Device::GetConfigurationVersion()
{
    VerifyOrReturnValue(mBridgedDevice.IsConstructed(), 0);
    return mBridgedDevice.Cluster().GetConfigurationVersion();
}

void Device::IncreaseConfigurationVersion()
{
    VerifyOrReturn(mBridgedDevice.IsConstructed());
    LogErrorOnFailure(mBridgedDevice.Cluster().IncreaseConfigurationVersion());
    ChipLogProgress(DeviceLayer, "Device[%s]: New Configuration Version=\"%d\"", mName,
                    mBridgedDevice.Cluster().GetConfigurationVersion());
}

DeviceOnOff::DeviceOnOff(const char * szDeviceName, std::string szLocation) : Device(szDeviceName, szLocation)
{
    mOn = false;
}

bool DeviceOnOff::IsOn()
{
    return mOn;
}

void DeviceOnOff::SetOnOff(bool aOn)
{
    bool changed;

    changed = aOn ^ mOn;
    mOn     = aOn;
    ChipLogProgress(DeviceLayer, "Device[%s]: %s", mName, aOn ? "ON" : "OFF");

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_OnOff);
    }
}

void DeviceOnOff::Toggle()
{
    bool aOn = !IsOn();
    SetOnOff(aOn);
}

void DeviceOnOff::SetChangeCallback(DeviceCallback_fn aChanged_CB)
{
    mChanged_CB = aChanged_CB;
}

void DeviceOnOff::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (DeviceOnOff::Changed_t) changeMask);
    }
}

DeviceSwitch::DeviceSwitch(const char * szDeviceName, std::string szLocation, uint32_t aFeatureMap) :
    Device(szDeviceName, szLocation)
{
    mNumberOfPositions = 2;
    mCurrentPosition   = 0;
    mMultiPressMax     = 2;
    mFeatureMap        = aFeatureMap;
}

void DeviceSwitch::SetNumberOfPositions(uint8_t aNumberOfPositions)
{
    bool changed;

    changed            = aNumberOfPositions != mNumberOfPositions;
    mNumberOfPositions = aNumberOfPositions;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_NumberOfPositions);
    }
}

void DeviceSwitch::SetCurrentPosition(uint8_t aCurrentPosition)
{
    bool changed;

    changed          = aCurrentPosition != mCurrentPosition;
    mCurrentPosition = aCurrentPosition;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_CurrentPosition);
    }
}

void DeviceSwitch::SetMultiPressMax(uint8_t aMultiPressMax)
{
    bool changed;

    changed        = aMultiPressMax != mMultiPressMax;
    mMultiPressMax = aMultiPressMax;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_MultiPressMax);
    }
}

void DeviceSwitch::SetChangeCallback(DeviceCallback_fn aChanged_CB)
{
    mChanged_CB = aChanged_CB;
}

void DeviceSwitch::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (DeviceSwitch::Changed_t) changeMask);
    }
}

DeviceTempSensor::DeviceTempSensor(const char * szDeviceName, std::string szLocation, int16_t min, int16_t max,
                                   int16_t measuredValue) :
    Device(szDeviceName, szLocation),
    mMin(min), mMax(max), mMeasurement(measuredValue)
{}

void DeviceTempSensor::SetMeasuredValue(int16_t measurement)
{
    // Limit measurement based on the min and max.
    if (measurement < mMin)
    {
        measurement = mMin;
    }
    else if (measurement > mMax)
    {
        measurement = mMax;
    }

    bool changed = mMeasurement != measurement;

    ChipLogProgress(DeviceLayer, "TempSensorDevice[%s]: New measurement=\"%d\"", mName, measurement);

    mMeasurement = measurement;

    if (changed && mChanged_CB)
    {
        mChanged_CB(this, kChanged_MeasurementValue);
    }
}

void DeviceTempSensor::SetChangeCallback(DeviceCallback_fn aChanged_CB)
{
    mChanged_CB = aChanged_CB;
}

void DeviceTempSensor::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (DeviceTempSensor::Changed_t) changeMask);
    }
}

void ComposedDevice::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (ComposedDevice::Changed_t) changeMask);
    }
}

void DevicePowerSource::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (DevicePowerSource::Changed_t) changeMask);
    }
}

void DevicePowerSource::SetBatChargeLevel(uint8_t aBatChargeLevel)
{
    bool changed;

    changed         = aBatChargeLevel != mBatChargeLevel;
    mBatChargeLevel = aBatChargeLevel;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_BatLevel);
    }
}

void DevicePowerSource::SetDescription(std::string aDescription)
{
    bool changed;

    changed      = aDescription != mDescription;
    mDescription = aDescription;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_Description);
    }
}

void DevicePowerSource::SetEndpointList(std::vector<chip::EndpointId> aEndpointList)
{
    bool changed  = aEndpointList != mEndpointList;
    mEndpointList = aEndpointList;

    if (changed && mChanged_CB)
    {
        mChanged_CB(this, kChanged_EndpointList);
    }
}

EndpointListInfo::EndpointListInfo(uint16_t endpointListId, std::string name, EndpointListTypeEnum type)
{
    mEndpointListId = endpointListId;
    mName           = name;
    mType           = type;
}

EndpointListInfo::EndpointListInfo(uint16_t endpointListId, std::string name, EndpointListTypeEnum type,
                                   chip::EndpointId endpointId)
{
    mEndpointListId = endpointListId;
    mName           = name;
    mType           = type;
    mEndpoints.push_back(endpointId);
}

void EndpointListInfo::AddEndpointId(chip::EndpointId endpointId)
{
    mEndpoints.push_back(endpointId);
}

Room::Room(std::string name, uint16_t endpointListId, EndpointListTypeEnum type, bool isVisible)
{
    mName           = name;
    mEndpointListId = endpointListId;
    mType           = type;
    mIsVisible      = isVisible;
}

Action::Action(uint16_t actionId, std::string name, ActionTypeEnum type, uint16_t endpointListId, uint16_t supportedCommands,
               ActionStateEnum status, bool isVisible)
{
    mActionId          = actionId;
    mName              = name;
    mType              = type;
    mEndpointListId    = endpointListId;
    mSupportedCommands = supportedCommands;
    mStatus            = status;
    mIsVisible         = isVisible;
}
