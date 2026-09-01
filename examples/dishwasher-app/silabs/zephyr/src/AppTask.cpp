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

#include "AppTask.h"
#include "CHIPDeviceManager.h"

#include "DataModelHelper.h"
#include "DeviceEnergyManager.h"
#include "DishwasherManager.h"
#include "operational-state-delegate-impl.h"

#include <DEMConfig.h>
#include <ElectricalSensorManager.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <memory>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::DataModel;

namespace {

/* -------------------------------------------------------------------------- */
/*                          LED / button DT bindings                          */
/* -------------------------------------------------------------------------- */

#if DT_NODE_EXISTS(DT_ALIAS(led0))
#define HAS_LED0 1
static const struct gpio_dt_spec sLed0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static struct k_timer sLed0BlinkTimer;
static bool sLed0State;
#endif

#if DT_NODE_EXISTS(DT_ALIAS(led1))
#define HAS_LED1 1
static const struct gpio_dt_spec sLed1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static struct k_timer sLed1BlinkTimer;
static bool sLed1State;
static bool sLed1BlinkActive;
#endif

#if DT_NODE_EXISTS(DT_ALIAS(sw0))
#define HAS_BUTTON0 1
static const struct gpio_dt_spec sButton0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback sButton0CbData;
static struct k_work_delayable sFactoryResetWarningWork;
static struct k_work_delayable sFactoryResetTriggerWork;
#endif

#if DT_NODE_EXISTS(DT_ALIAS(sw1))
#define HAS_BUTTON1 1
static const struct gpio_dt_spec sButton1 = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static struct gpio_callback sButton1CbData;
static struct k_work sCycleOpStateWork;
#endif

/* -------------------------------------------------------------------------- */
/*                     Electrical sensor (endpoint 2)                         */
/* -------------------------------------------------------------------------- */

const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyRangeStruct::Type kMeasurementAccuracyRanges[] = {
    { .rangeMin   = 0,
      .rangeMax   = 1'000'000'000'000'000,
      .percentMax = MakeOptional(static_cast<chip::Percent100ths>(500)),
      .percentMin = MakeOptional(static_cast<chip::Percent100ths>(50)) }
};

const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyStruct::Type kMeasurementAccuracy = {
    .measurementType  = MeasurementTypeEnum::kElectricalEnergy,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 1'000'000'000'000'000,
    .accuracyRanges   = DataModel::List<const ElectricalEnergyMeasurement::Structs::MeasurementAccuracyRangeStruct::Type>(
        kMeasurementAccuracyRanges)
};

std::unique_ptr<ElectricalSensorManager> gESManager;

// EPM attribute values indexed by OperationalStateEnum (Stopped/Running/Paused/Error)
static const struct
{
    int64_t Voltage;       // mV
    int64_t ActiveCurrent; // mA
    int64_t ActivePower;   // mW
} kAttributes[4] = {
    { 120'000, 0, 0 },             // kStopped
    { 120'000, 15'000, 1800'000 }, // kRunning
    { 120'000, 125, 17'000 },      // kPaused
    { 0, 0, 0 },                   // kError
};

#ifdef HAS_LED0
static void Led0BlinkTimerHandler(struct k_timer * timer)
{
    sLed0State = !sLed0State;
    gpio_pin_set_dt(&sLed0, static_cast<int>(sLed0State));
}
#endif

#ifdef HAS_LED1
static void Led1BlinkTimerHandler(struct k_timer * timer)
{
    sLed1State = !sLed1State;
    gpio_pin_set_dt(&sLed1, static_cast<int>(sLed1State));
}

static void StopLed1Blink()
{
    if (sLed1BlinkActive)
    {
        k_timer_stop(&sLed1BlinkTimer);
        sLed1BlinkActive = false;
    }
}
#endif

#ifdef HAS_BUTTON0
static void FactoryResetWarningWorkHandler(struct k_work * work)
{
    LOG_INF("Keep holding to factory reset in 3 seconds. Release to cancel.");
}

static void FactoryResetTriggerWorkHandler(struct k_work * work)
{
    LOG_INF("Factory reset triggered");
    chip::Server::GetInstance().ScheduleFactoryReset();
}

static void Button0PressedHandler(const struct device * dev, struct gpio_callback * cb, uint32_t pins)
{
    if (gpio_pin_get_dt(&sButton0) > 0)
    {
        k_work_schedule(&sFactoryResetWarningWork, K_SECONDS(2));
        k_work_schedule(&sFactoryResetTriggerWork, K_SECONDS(5));
    }
    else
    {
        bool wasPending = k_work_cancel_delayable(&sFactoryResetTriggerWork) != 0;
        k_work_cancel_delayable(&sFactoryResetWarningWork);
        if (wasPending)
        {
            LOG_INF("Factory reset canceled");
        }
    }
}
#endif

#ifdef HAS_BUTTON1
static void CycleOperationalStateWork(intptr_t arg)
{
    static_cast<void>(arg);
    DishwasherManager * mgr = GetDishwasherManager();
    if (mgr != nullptr)
    {
        mgr->CycleOperationalState();
    }
}

static void CycleOpStateWorkHandler(struct k_work * work)
{
    static_cast<void>(work);
    RETURN_SAFELY_IGNORED PlatformMgr().ScheduleWork(CycleOperationalStateWork, 0);
}

static void Button1PressedHandler(const struct device * dev, struct gpio_callback * cb, uint32_t pins)
{
    k_work_submit(&sCycleOpStateWork);
}
#endif

CHIP_ERROR InitElectricalSensorManager()
{
    EndpointId endpointId = DataModelHelper::GetEndpointIdFromCluster(ElectricalPowerMeasurement::Id);
    VerifyOrReturnError(endpointId != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "EPM cluster endpoint not found"));

    VerifyOrReturnError(!gESManager, CHIP_ERROR_INCORRECT_STATE, ChipLogError(AppServer, "ES manager already initialized"));

    gESManager = std::make_unique<ElectricalSensorManager>();
    VerifyOrReturnError(gESManager, CHIP_ERROR_NO_MEMORY);

    ElectricalSensorManager::EpmConfig epmConfig{
        .features = BitMask<ElectricalPowerMeasurement::Feature, uint32_t>(
            ElectricalPowerMeasurement::Feature::kDirectCurrent, ElectricalPowerMeasurement::Feature::kAlternatingCurrent),
        .optionalAttributes = BitMask<ElectricalPowerMeasurement::OptionalAttributes, uint32_t>(
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeVoltage,
            ElectricalPowerMeasurement::OptionalAttributes::kOptionalAttributeActiveCurrent),
    };

    ElectricalSensorManager::EemConfig eemConfig{
        .features = BitMask<ElectricalEnergyMeasurement::Feature, uint32_t>(
            ElectricalEnergyMeasurement::Feature::kImportedEnergy, ElectricalEnergyMeasurement::Feature::kCumulativeEnergy),
        .optionalAttributes = ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster::OptionalAttributesSet()
                                  .Set<ElectricalEnergyMeasurement::Attributes::CumulativeEnergyReset::Id>(),
        .accuracyStruct = kMeasurementAccuracy,
    };

    ElectricalSensorManager::PtConfig ptConfig{
        .features = BitMask<PowerTopology::Feature, uint32_t>(PowerTopology::Feature::kNodeTopology),
    };

    ReturnErrorOnFailure(gESManager->Init(endpointId, epmConfig, eemConfig, ptConfig));
    return CHIP_NO_ERROR;
}

} // namespace

ElectricalSensorManager * GetESManager()
{
    return gESManager.get();
}

void UpdateEpmAttributesForOperationalState(OperationalStateEnum state)
{
    VerifyOrReturn(gESManager != nullptr);

    size_t index = 0;
    switch (state)
    {
    case OperationalStateEnum::kStopped:
        index = 0;
        break;
    case OperationalStateEnum::kRunning:
        index = 1;
        break;
    case OperationalStateEnum::kPaused:
        index = 2;
        break;
    case OperationalStateEnum::kError:
    default:
        index = 3;
        break;
    }

    chip::ChipError err = gESManager->SendPowerReading(kAttributes[index].ActivePower, kAttributes[index].Voltage,
                                                       kAttributes[index].ActiveCurrent);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(AppServer, "ERR: updating EPM attributes for operational state %" CHIP_ERROR_FORMAT, err.Format()));
}

void UpdateOperationalStateLed(OperationalStateEnum state)
{
#ifdef HAS_LED1
    VerifyOrReturn(gpio_is_ready_dt(&sLed1));

    StopLed1Blink();
    sLed1State = false;
    gpio_pin_set_dt(&sLed1, 0);

    switch (state)
    {
    case OperationalStateEnum::kRunning:
        gpio_pin_set_dt(&sLed1, 1);
        sLed1State = true;
        break;
    case OperationalStateEnum::kPaused:
        sLed1BlinkActive = true;
        // ~300 ms on / 700 ms off
        k_timer_start(&sLed1BlinkTimer, K_MSEC(300), K_MSEC(1000));
        break;
    case OperationalStateEnum::kError:
        sLed1BlinkActive = true;
        k_timer_start(&sLed1BlinkTimer, K_MSEC(100), K_MSEC(100));
        break;
    case OperationalStateEnum::kStopped:
    default:
        break;
    }
#else
    (void) state;
#endif
}

void AppTask::ActionInitiated(OperationalStateEnum action)
{
    if (action == OperationalStateEnum::kRunning)
    {
        ChipLogProgress(DeviceLayer, "Starting dishwasher");
    }
    else if (action == OperationalStateEnum::kStopped)
    {
        ChipLogProgress(DeviceLayer, "Stopping dishwasher");
    }
    else if (action == OperationalStateEnum::kPaused)
    {
        ChipLogProgress(DeviceLayer, "Pausing dishwasher");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Dishwasher action error");
        action = OperationalStateEnum::kError;
    }

    PlatformMgr().LockChipStack();
    CHIP_ERROR err = OperationalState::GetInstance()->SetOperationalState(to_underlying(action));
    PlatformMgr().UnlockChipStack();

    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(DeviceLayer, "ERR: updating Operational state %" CHIP_ERROR_FORMAT, err.Format()));

    GetDishwasherManager()->UpdateOperationState(action);
    UpdateEpmAttributesForOperationalState(action);
}

void AppTask::ActionCompleted()
{
    // LED already updated in UpdateOperationState -> UpdateDishwasherLed.
}

void AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to Zephyr Matter Dishwasher Demo App");

#ifdef HAS_LED0
    if (gpio_is_ready_dt(&sLed0))
    {
        VerifyOrDie(gpio_pin_configure_dt(&sLed0, GPIO_OUTPUT_INACTIVE) == 0);
        k_timer_init(&sLed0BlinkTimer, Led0BlinkTimerHandler, nullptr);
    }
#endif

#ifdef HAS_LED1
    if (gpio_is_ready_dt(&sLed1))
    {
        VerifyOrDie(gpio_pin_configure_dt(&sLed1, GPIO_OUTPUT_INACTIVE) == 0);
        k_timer_init(&sLed1BlinkTimer, Led1BlinkTimerHandler, nullptr);
    }
#endif

#ifdef HAS_BUTTON0
    k_work_init_delayable(&sFactoryResetWarningWork, FactoryResetWarningWorkHandler);
    k_work_init_delayable(&sFactoryResetTriggerWork, FactoryResetTriggerWorkHandler);

    if (gpio_is_ready_dt(&sButton0))
    {
        VerifyOrDie(gpio_pin_configure_dt(&sButton0, GPIO_INPUT) == 0);
        VerifyOrDie(gpio_pin_interrupt_configure_dt(&sButton0, GPIO_INT_EDGE_BOTH) == 0);
        gpio_init_callback(&sButton0CbData, Button0PressedHandler, BIT(sButton0.pin));
        gpio_add_callback(sButton0.port, &sButton0CbData);
    }
#endif

#ifdef HAS_BUTTON1
    k_work_init(&sCycleOpStateWork, CycleOpStateWorkHandler);

    if (gpio_is_ready_dt(&sButton1))
    {
        VerifyOrDie(gpio_pin_configure_dt(&sButton1, GPIO_INPUT) == 0);
        VerifyOrDie(gpio_pin_interrupt_configure_dt(&sButton1, GPIO_INT_EDGE_TO_ACTIVE) == 0);
        gpio_init_callback(&sButton1CbData, Button1PressedHandler, BIT(sButton1.pin));
        gpio_add_callback(sButton1.port, &sButton1CbData);
    }
#endif
}

void AppTask::PostInitMatterStack()
{
#ifdef HAS_LED0
    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        sLed0State = false;
        k_timer_start(&sLed0BlinkTimer, K_MSEC(500), K_MSEC(500));
    }
#endif
}

void AppTask::PostInitMatterServerInstance()
{
    CHIP_ERROR err = DeviceEnergyManager::Instance().Init();
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(DeviceLayer, "DeviceEnergyManager.Init failed: %" CHIP_ERROR_FORMAT, err.Format()));

    err = InitElectricalSensorManager();
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(DeviceLayer, "ElectricalSensorManager.Init failed: %" CHIP_ERROR_FORMAT, err.Format()));

    GetDishwasherManager()->SetCallbacks(ActionInitiated, ActionCompleted);

    OperationalStateEnum state = static_cast<OperationalStateEnum>(OperationalState::GetInstance()->GetCurrentOperationalState());
    GetDishwasherManager()->UpdateOperationState(state);
    UpdateEpmAttributesForOperationalState(state);
}

AppTask & AppTask::GetDefaultInstance()
{
    static AppTask sAppTask;
    return sAppTask;
}

chip::Zephyr::App::AppTaskBase & chip::Zephyr::App::GetAppTask()
{
    return AppTask::GetDefaultInstance();
}
