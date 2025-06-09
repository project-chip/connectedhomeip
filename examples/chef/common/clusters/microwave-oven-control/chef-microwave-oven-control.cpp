/*
*
*    Copyright (c) 2025 Project CHIP Authors
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


#include "chef-microwave-oven-control.h"


using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::ModeBase;
using namespace chip::app::Clusters::MicrowaveOvenControl;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;
using Status            = Protocols::InteractionModel::Status;


#ifdef MATTER_DM_PLUGIN_MICROWAVE_OVEN_CONTROL_SERVER


ChefMicrowaveOvenDevice::ChefMicrowaveOvenDevice(EndpointId aClustersEndpoint) :
   mOperationalStateInstancePtr(OperationalState::GetOperationalStateInstance()),
   mMicrowaveOvenModeInstancePtr(ChefMicrowaveOvenMode::GetInstance(aClustersEndpoint)),
   mMicrowaveOvenControlInstance(this, aClustersEndpoint, MicrowaveOvenControl::Id,
                                 BitMask<MicrowaveOvenControl::Feature>(MicrowaveOvenControl::Feature::kPowerAsNumber,
                                                                        MicrowaveOvenControl::Feature::kPowerNumberLimits),
                                 *mOperationalStateInstancePtr, *mMicrowaveOvenModeInstancePtr)
{
   VerifyOrDie(mOperationalStateInstancePtr != nullptr);
   VerifyOrDie(mMicrowaveOvenModeInstancePtr != nullptr);
}


void ChefMicrowaveOvenDevice::MicrowaveOvenInit()
{
   mOperationalStateInstancePtr->SetOperationalState(to_underlying(OperationalStateEnum::kStopped));
   mMicrowaveOvenControlInstance.Init();
}


/**
* MicrowaveOvenControl cluster
*/
Protocols::InteractionModel::Status ChefMicrowaveOvenDevice::HandleSetCookingParametersCallback(
   uint8_t cookMode, uint32_t cookTimeSec, bool startAfterSetting, Optional<uint8_t> powerSettingNum,
   Optional<uint8_t> wattSettingIndex)
{
   Status status;
   // Update cook mode.
   if ((status = mMicrowaveOvenModeInstancePtr->UpdateCurrentMode(cookMode)) != Status::Success)
   {
       return status;
   }


   mMicrowaveOvenControlInstance.SetCookTimeSec(cookTimeSec);


   // If using power as number, check if powerSettingNum has value before setting the power number.
   // If powerSetting field is missing in the command, the powerSettingNum passed here is handled to the max value
   // and user can use this value directly.
   if (powerSettingNum.HasValue())
   {
       mPowerSettingNum = powerSettingNum.Value();
   }


   return Status::Success;
}


Protocols::InteractionModel::Status ChefMicrowaveOvenDevice::HandleModifyCookTimeSecondsCallback(uint32_t finalCookTimeSec)
{
   mMicrowaveOvenControlInstance.SetCookTimeSec(finalCookTimeSec);
   return Status::Success;
}


CHIP_ERROR ChefMicrowaveOvenDevice::GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting)
{
   VerifyOrReturnError(index < MATTER_ARRAY_SIZE(mWattSettingList), CHIP_ERROR_NOT_FOUND);


   wattSetting = mWattSettingList[index];
   return CHIP_NO_ERROR;
}


static constexpr EndpointId kDemoEndpointId = 1;


Platform::UniquePtr<ChefMicrowaveOvenDevice> gMicrowaveOvenDevice;


void MatterMicrowaveOvenServerInit()
{
   gMicrowaveOvenDevice = Platform::MakeUnique<ChefMicrowaveOvenDevice>(kDemoEndpointId);
   gMicrowaveOvenDevice.get()->MicrowaveOvenInit();
}


void MatterMicrowaveOvenServerShutdown()
{
   gMicrowaveOvenDevice = nullptr;
}


#endif // MATTER_DM_PLUGIN_MICROWAVE_OVEN_CONTROL_SERVER
