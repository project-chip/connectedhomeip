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

#pragma once
#include <device/types/closure/Closure.h>
#include <device/types/closure-panel/impl/LoggingClosurePanel.h>
#include <device/types/on-off-light/impl/LoggingOnOffLight.h>
#include <lib/support/TimerDelegate.h>
namespace chip{
namespace app {


struct PanelList
{
    ClosurePanel::Config config;
    Span<const EndpointComposition::SemanticTag> tags;   
};

class LoggingClosure : public Closure,
                       public Clusters::ClosureControl::ClosureControlClusterDelegate,
                       public TimerContext
{
public:

    LoggingClosure(TimerDelegate & Tdelegate,
                    Clusters::IdentifyDelegate& Idelegate, Closure::Config CConfig,
    Credentials::GroupDataProvider & groupDataProvider,FabricTable & fabricTable, std::vector<PanelList> panels);
    ~LoggingClosure() override;
    Protocols::InteractionModel::Status HandleStopCommand() override;


    Protocols::InteractionModel::Status HandleMoveToCommand(const Optional<Clusters::ClosureControl::TargetPositionEnum> & position,
                                                                    const Optional<bool> & latch,
                                                                    const Optional<Clusters::Globals::ThreeLevelAutoEnum> & speed) override;

    Protocols::InteractionModel::Status HandleCalibrateCommand() override;

    bool IsReadyToMove() override;
    ElapsedS GetCalibrationCountdownTime() override;
    ElapsedS GetMovingCountdownTime() override;
    ElapsedS GetWaitingForMotionCountdownTime() override;

    // -- TimerContext Interface --
    void TimerFired() override;
private:
    // Simulated duration a Calibrate command takes to complete. Kept well under the
    // test suite's default --timeout (30s) so the resulting MainState report arrives in time.
    static constexpr uint32_t kTimeoutnDurationSec = 3;
    bool RegistersAccessDevicePanel() const override;
    void CancelTimer();
    LoggingOnOffLight::Context OnOffContext;
    CHIP_ERROR RegisterParts(EndpointIdAllocator &allocator, CodeDrivenDataModelProvider &provider,EndpointComposition composition) override;
    void UnregisterParts(CodeDrivenDataModelProvider &provide) override;
    // TODO add LoggingDoorLock after migration
    std::vector<std::unique_ptr<LoggingClosurePanel>> mLoggingClosurePanel;
    std::unique_ptr<LoggingOnOffLight> mLoggingOnOffLights;
    std::vector<PanelList> mPanelList;
    TimerDelegate & mTimerDelegate;
    std::optional<Clusters::ClosureControl::GenericOverallCurrentState> mPendingCurrentState;
};

}
}