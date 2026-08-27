/*
 *
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

#include <clusters/RvcOperationalState/Enums.h>
#include <device/types/robotic-vacuum-cleaner/RoboticVacuumCleaner.h>
#include <lib/support/TimerDelegate.h>
#include <platform/DiagnosticDataProvider.h>
#include <string>

namespace chip::app {

// PhysicalDockState tracks whether the simulated robot is on the charging dock, independently of
// RvcOperationalState. This is needed because HandleResumeStateCallback() can move the reported
// operational state straight to Running while the robot is still physically on the dock, so a
// later HandleCharging()/HandleCharged() call still needs to know the dock contact never changed.
//
/*
 *                                   +-------------------+
 * +-------------------------------->|     kOffDock      |<---------------------------------------+
 *                                   +-------------------+
 * |                 |                                     |                                       |
 * |                 | Placed on dock manually             | Homing found the charger              |
 * |                 | (HandleDocked())                    | (HandleChargerFound())                |
 * |                 v                                     v                                       |
 * |             +-----------+                           +-----------------+                       |
 * |             |  kOnDock  |                           | kOnDockCharging |                       |
 * |             +-----------+                           +-----------------+                       |
 * |                          -- dock contact made, starts charging -->                            |
 * |                          (HandleCharging())                                                   |
 * |                          <- charging finished, robot stays docked --                          |
 * |                          (HandleCharged(), RunMode == Idle)                                   |
 * |                 |                                     |                                       |
 * |                 | Robot leaves the dock to            | Robot leaves the dock to              |
 * |                 | resume cleaning/mapping             | resume cleaning/mapping               |
 * |                 | (Resume(), or                       | (Resume(), or                         |
 * |                 |  HandleCharged() mid-task)          |  HandleCharged() mid-task)            |
 * +                 +                                     +                                       +
 * +------------------------------------------------------------------------------------------------
 */
enum class PhysicalDockState : uint8_t
{
    kOffDock,
    kOnDock,
    kOnDockCharging,
};

// Single "fake robot vacuum" that implements every mandatory RVC delegate interface itself, so
// the simulation logic shares state directly as methods on one class instead of coordinating
// several delegate objects through setters and callbacks.
//
// It also simulates accelerated real-world behavior: once cleaning/mapping starts, it completes
// on its own after kActivitySimulationDurationSec; once charging starts, it completes on its own
// after kChargingSimulationDurationSec. The named-pipe entry points below remain available so
// tests can still drive (or pre-empt) these transitions manually.
class SimulatedRoboticVacuumCleaner : public RoboticVacuumCleaner,
                                      public Clusters::OperationalState::OperationalStateCluster::Delegate,
                                      public Clusters::ServiceArea::Delegate,
                                      public TimerContext
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit SimulatedRoboticVacuumCleaner(const Context & context);
    ~SimulatedRoboticVacuumCleaner() override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // -- TimerContext --
    void TimerFired() override;

    // -- OperationalState::Delegate --
    DataModel::Nullable<uint32_t> GetCountdownTime() override { return DataModel::NullNullable; }
    CHIP_ERROR GetOperationalStateAtIndex(size_t index,
                                          Clusters::OperationalState::GenericOperationalState & operationalState) override;
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t /* index */, MutableCharSpan & /* operationalPhase */) override
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    void HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;
    void HandleGoHomeCommandCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    // -- ServiceArea::Delegate --
    CHIP_ERROR Init() override;
    bool IsSetSelectedAreasAllowed(MutableCharSpan & statusText) override;
    bool IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas,
                               Clusters::ServiceArea::SelectAreasStatus & locationStatus, MutableCharSpan & statusText) override;
    bool HandleSkipArea(uint32_t skippedArea, MutableCharSpan & skipStatusText) override;
    bool IsSupportedAreasChangeAllowed() override;
    bool IsSupportedMapChangeAllowed() override;

    // Named-pipe simulation entry points ported from examples/rvc-app/rvc-common/src/rvc-device.cpp.
    // These remain available so tests can drive transitions manually; the timer above drives the
    // same transitions automatically after the simulated duration elapses.
    void HandleCharged();
    void HandleCharging();
    void HandleDocked();
    void HandleChargerFound();
    void HandleLowCharge();
    void HandleActivityComplete();
    void HandleAreaComplete();
    void HandleClearError();
    void HandleErrorEvent(const std::string & error);

private:
    // ModeBase::AppDelegate can only be implemented once per class, but RunMode and CleanMode each
    // need their own instance. These adapters hold no state of their own; they just forward into
    // the mode-specific methods below, which share the rest of the simulation's state directly.
    class RunModeAppDelegate : public Clusters::ModeBase::AppDelegate
    {
    public:
        explicit RunModeAppDelegate(SimulatedRoboticVacuumCleaner & owner) : mOwner(owner) {}
        CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
        CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
        CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
        CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                      DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override;
        void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override
        {
            mOwner.HandleRunModeChangeToMode(newMode, response);
        }

    private:
        SimulatedRoboticVacuumCleaner & mOwner;
    };

    class CleanModeAppDelegate : public Clusters::ModeBase::AppDelegate
    {
    public:
        explicit CleanModeAppDelegate(SimulatedRoboticVacuumCleaner & owner) : mOwner(owner) {}
        CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
        CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
        CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
        CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                      DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) override;
        void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override
        {
            mOwner.HandleCleanModeChangeToMode(newMode, response);
        }

    private:
        SimulatedRoboticVacuumCleaner & mOwner;
    };

    void HandleRunModeChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response);
    void HandleCleanModeChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response);

    void SetDeviceToIdleState();
    void UpdateServiceAreaProgressOnExit();
    void SetMapTopology();
    void SetAttributesAtCleanStart();
    void GoToNextArea(Clusters::ServiceArea::OperationalStatusEnum currentAreaOpState, bool & finished);

    // Clears physical dock state when RunMode leaves dock/charge for Running.
    void ClearDockChargingTracking();

    void CancelTimer();
    void StartActivityTimer();
    void StartChargingTimer();

    static constexpr uint32_t kActivitySimulationDurationSec = 30;
    static constexpr uint32_t kChargingSimulationDurationSec = 30;

    TimerDelegate & mTimerDelegate;
    RunModeAppDelegate mRunModeAppDelegate{ *this };
    CleanModeAppDelegate mCleanModeAppDelegate{ *this };

    uint8_t mStateBeforePause            = 0;
    PhysicalDockState mPhysicalDockState = PhysicalDockState::kOffDock;
};

} // namespace chip::app
