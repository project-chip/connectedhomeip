/*
 *    Copyright (c) 2026 Project CHIP Authors
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

// helper to return tags for the closurepanel
#include "LoggingClosure.h"


namespace chip{
namespace app {

    LoggingClosure::LoggingClosure(TimerDelegate & Tdelegate,
                    Clusters::IdentifyDelegate& Idelegate, Closure::Config CConfig,
    Credentials::GroupDataProvider & groupDataProvider,FabricTable & fabricTable, std::vector<PanelList> panels) : 
                                    Closure(CConfig,Tdelegate,Idelegate,*this), OnOffContext({groupDataProvider,fabricTable,Tdelegate,Idelegate}), 
                                    mPanelList(std::move(panels))
    {}
    
    Protocols::InteractionModel::Status LoggingClosure::HandleStopCommand()
    {
        ChipLogProgress(DeviceLayer, "LoggingClosure::HandleStopCommand()");
        return Protocols::InteractionModel::Status::Success;
    }
    
    Protocols::InteractionModel::Status LoggingClosure::HandleMoveToCommand(const Optional<Clusters::ClosureControl::TargetPositionEnum> & position,
                                                                        const Optional<bool> & latch,
                                                                        const Optional<Clusters::Globals::ThreeLevelAutoEnum> & speed)
    {
        ChipLogProgress(DeviceLayer, "LoggingClosure::HandleMoveToCommand() -> position=%hhu latch=%d speed=%u",
                    position.ValueOr(Clusters::ClosureControl::TargetPositionEnum::kUnknownEnumValue), latch.ValueOr(false),
                    to_underlying(speed.ValueOr(Clusters::Globals::ThreeLevelAutoEnum::kAuto)));
        return Protocols::InteractionModel::Status::Success;
    }
    
    Protocols::InteractionModel::Status LoggingClosure::HandleCalibrateCommand()
    {
        ChipLogProgress(DeviceLayer, "LoggingClosure::HandleCalibrateCommand()");
        return Protocols::InteractionModel::Status::Success;
    }

    bool LoggingClosure::IsReadyToMove()
    {
        ChipLogProgress(DeviceLayer, "LoggingClosure::IsReadyToMove()");
        return true;
    }
    ElapsedS LoggingClosure::GetCalibrationCountdownTime()
    {
        ChipLogProgress(DeviceLayer, "LoggingClosure::GetCalibrationCountdownTime()");
        return 0u;
    }
    ElapsedS LoggingClosure::GetMovingCountdownTime()
    {
        ChipLogProgress(DeviceLayer, "LoggingClosure::GetMovingCountdownTime()");
        return 0u;
    }
    ElapsedS LoggingClosure::GetWaitingForMotionCountdownTime()
    {
        ChipLogProgress(DeviceLayer, "LoggingClosure::GetMovingCountdownTime()");
        return 0u;
    }
    bool LoggingClosure::RegistersAccessDevicePanel() const 
    {
        for(const auto & panel : mPanelList)
        {
            if(panel.config.withAccess)
            {
                return true;
            }
        }
        return false;
    }
    CHIP_ERROR LoggingClosure::RegisterParts(EndpointIdAllocator &allocator, CodeDrivenDataModelProvider &provider,EndpointComposition composition)
    {        

        for (const auto& panel : mPanelList)
        {
            auto ClosurePanel = std::make_unique<LoggingClosurePanel>(panel.config);
            // the tag list will be provided in the devicefactory.h
            composition.tagList = panel.tags;
            ReturnErrorOnFailure(ClosurePanel->Register(allocator, provider, composition));
            mLoggingClosurePanel.push_back(std::move(ClosurePanel));
        }

        mLoggingOnOffLights = std::make_unique<LoggingOnOffLight>(OnOffContext);
        ReturnErrorOnFailure(mLoggingOnOffLights->Register(allocator.Allocate(), provider, EndpointComposition::WithParent(GetEndpointId())));
        return CHIP_NO_ERROR;
    }

    void LoggingClosure::UnregisterParts(CodeDrivenDataModelProvider &provider)
    {
        for (size_t i = 0; i < mLoggingClosurePanel.size(); i++)
        {
            mLoggingClosurePanel[i]->Unregister(provider);
        }
    }
    
}
}