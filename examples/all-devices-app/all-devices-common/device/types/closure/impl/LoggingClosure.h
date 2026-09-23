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
#include <app/TestEventTriggerDelegate.h>
#include <device/types/closure-panel/impl/LoggingClosurePanel.h>
#include <device/types/closure/Closure.h>
#include <device/types/on-off-light/impl/LoggingOnOffLight.h>
#include <lib/support/TimerDelegate.h>
namespace chip {
namespace app {

struct PanelList
{
    ClosurePanel::Config config;
    Span<const EndpointComposition::SemanticTag> tags;
};

class LoggingClosure : public Clusters::ClosureControl::ClosureControlClusterDelegate,
                       public Closure,
                       public TimerContext,
                       public TestEventTriggerHandler
{
public:
    LoggingClosure(TimerDelegate & Tdelegate, Clusters::IdentifyDelegate & Idelegate, Closure::Config CConfig,
                   Credentials::GroupDataProvider & groupDataProvider, FabricTable & fabricTable, std::vector<PanelList> panels,
                   TestEventTriggerDelegate & testEventTriggerDelegate);
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

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

private:
    static constexpr uint32_t kTimeoutDurationSec = 1;
    bool RegistersAccessDevicePanel() const override;
    void CancelTimer();
    LoggingOnOffLight::Context OnOffContext;
    CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                             EndpointComposition composition) override;
    void UnregisterParts(CodeDrivenDataModelProvider & provide) override;
    // TODO add LoggingDoorLock after migration
    std::vector<std::unique_ptr<LoggingClosurePanel>> mLoggingClosurePanel;
    // Held by base-class handle: OnOffLoad hides the SingleEndpoint Register/Unregister names,
    // while DeviceInterface declares both publicly.
    std::unique_ptr<DeviceInterface> mLoggingOnOffLights;
    // Non-owning view of the same object as mLoggingOnOffLights, kept as the leaf type because only
    // SingleEndpoint exposes the endpoint id that UnregisterParts needs. Cleared alongside the owner.
    LoggingOnOffLight * mOnOffLightPart = nullptr;
    std::vector<PanelList> mPanelList;
    TimerDelegate & mTimerDelegate;
    TestEventTriggerDelegate & mTestEventTriggerDelegate;
    std::optional<Clusters::ClosureControl::GenericOverallCurrentState> mPendingCurrentState;

    static constexpr uint64_t kTriggerError         = 0x0104000000000000;
    static constexpr uint64_t kTriggerSetupRequired = 0x0104000000000003;
    static constexpr uint64_t kTriggerDisengaged    = 0x0104000000000002;
    static constexpr uint64_t kTriggerClear         = 0x0104000000000004;
    static constexpr uint64_t kTriggerProtected     = 0x0104000000000001;
};

} // namespace app
} // namespace chip
