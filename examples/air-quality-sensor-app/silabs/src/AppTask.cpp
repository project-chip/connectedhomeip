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

/**********************************************************
 * Includes
 *********************************************************/

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "CustomerAppTask.h"

#include "LEDWidget.h"

#if SL_MATTER_DISPLAY_ENABLED
#include "AirQualitySensorUI.h"
#include "lcd.h"
#if SL_MATTER_QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // SL_MATTER_QR_CODE_ENABLED
#endif // SL_MATTER_DISPLAY_ENABLED

#include <AirQualityConfig.h>
#include <air-quality-sensor-manager.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <cmsis_os2.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformError.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#ifdef USE_AIR_QUALITY_SENSOR
#include "AirQualitySensor.h"
#endif

#ifdef SL_MATTER_ENABLE_AWS
#include "MatterAwsControl.h"
#endif // SL_MATTER_ENABLE_AWS

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define APP_FUNCTION_BUTTON 0

using namespace chip;
using namespace chip::app;
using namespace chip::TLV;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::AirQuality;
using namespace chip::app::Clusters;

namespace {

CustomerAppTask & AppInstance()
{
    return CustomerAppTask::GetAppTask();
}

constexpr uint16_t kSensorTimerPeriodMs = SENSOR_TIMER_PERIOD_MS; // 30s timer period

osTimerId_t sSensorTimer = nullptr;

#ifndef USE_AIR_QUALITY_SENSOR
constexpr uint16_t kSimulatedReadingFrequency =
    (60000 / kSensorTimerPeriodMs); // for every two timer cycles, a simulated sensor update is triggered.
int32_t mSimulatedAirQuality[] = { 5, 55, 105, 155, 205, 255, 305, 355, 400 };
#endif

} // namespace

/**
 * @brief Classifies the air quality based on a given sensor value.
 *
 * This function compares the input value against predefined thresholds
 * defined in the AirQualityConfig.h file. The thresholds are used
 * to classify the air quality into categories defined by the AirQualityEnum.
 * The thresholds are defined in the SensorThresholds enum.
 *
 * @param value The sensor value used to classify air quality.
 * @return AirQualityEnum The classified air quality category.
 */
AirQualityEnum classifyAirQuality(int32_t value)
{
    if (value < MIN_THRESHOLD)
    {
        return AirQualityEnum::kUnknown;
    }
    else if (value < GOOD_THRESHOLD)
    {
        return AirQualityEnum::kGood;
    }
    else if (value < FAIR_THRESHOLD)
    {
        return AirQualityEnum::kFair;
    }
    else if (value < MODERATE_THRESHOLD)
    {
        return AirQualityEnum::kModerate;
    }
    else if (value < POOR_THRESHOLD)
    {
        return AirQualityEnum::kPoor;
    }
    else if (value < VERY_POOR_THRESHOLD)
    {
        return AirQualityEnum::kVeryPoor;
    }
    else
    {
        return AirQualityEnum::kExtremelyPoor;
    }
}

void InitAirQualitySensorManager(intptr_t arg)
{
    AirQualitySensorManager::InitInstance();
}

void AppTask::WriteAirQualityToAttribute(intptr_t context)
{
    int32_t * air_quality_ptr = reinterpret_cast<int32_t *>(context);
    AirQualitySensorManager::GetInstance()->OnAirQualityChangeHandler(classifyAirQuality(*air_quality_ptr));
    ChipLogDetail(AppServer, "RAW AirQuality value: %ld and corresponding Enum value : %d", *air_quality_ptr,
                  chip::to_underlying(AirQualitySensorManager::GetInstance()->GetAirQuality()));
    AppInstance().UpdateAirQualitySensorUI();
    delete air_quality_ptr;
}

CHIP_ERROR AppTask::InitAirQualitySensor()
{
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(InitAirQualitySensorManager);
    // Create cmsisos sw timer for air quality sensor timer.
    sSensorTimer = osTimerNew(&CustomerAppTask::SensorTimerEventHandler, osTimerPeriodic, nullptr, nullptr);
    if (sSensorTimer == nullptr)
    {
        ChipLogError(AppServer, "sSensorTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

#ifdef USE_AIR_QUALITY_SENSOR
    sl_status_t status = AirQualitySensor::Init();
    if (status != SL_STATUS_OK)
    {
        ChipLogError(AppServer, "Failed to Init Sensor with error code: %lx", status);
        return MATTER_PLATFORM_ERROR(status);
    }
#endif
    // Update Air Quality immediatly at bootup
    CustomerAppTask::SensorTimerEventHandler(nullptr);
    // Trigger periodic update
    uint32_t delayTicks = ((uint64_t) osKernelGetTickFreq() * kSensorTimerPeriodMs) / 1000;

    // Starts or restarts the function timer
    if (osTimerStart(sSensorTimer, delayTicks))
    {
        ChipLogError(AppServer, "sSensorTimer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::GetAirQualityValue(int32_t & air_quality)
{
#ifdef USE_AIR_QUALITY_SENSOR
    sl_status_t status = AirQualitySensor::GetAirQuality(air_quality);
    if (status != SL_STATUS_OK)
    {
        ChipLogError(AppServer, "Failed to read Air Quality: %lx", status);
        return MATTER_PLATFORM_ERROR(status);
    }
#else
    // Initialize static variables to keep track of the current index and repetition count
    static uint8_t nbOfRepetition = 0;
    static uint8_t simulatedIndex = 0;

    // Ensure the simulatedIndex wraps around the array size to avoid out-of-bounds access
    simulatedIndex = simulatedIndex % MATTER_ARRAY_SIZE(mSimulatedAirQuality);
    // Retrieve the current air quality value from the simulated data array using the simulatedIndex
    air_quality = mSimulatedAirQuality[simulatedIndex];

    // Increment the repetition count
    nbOfRepetition++;
    // Check if the number of repetitions has reached the threshold to simulate a new reading
    if (nbOfRepetition >= kSimulatedReadingFrequency)
    {
        // Move to the next index for the next simulated reading
        simulatedIndex++;
        // Reset the repetition count
        nbOfRepetition = 0;
    }
#endif // USE_AIR_QUALITY_SENSOR

    return CHIP_NO_ERROR;
}

void AppTask::SensorTimerEventHandler(void * arg)
{
    int32_t air_quality = 0;
    CHIP_ERROR err      = AppInstance().GetAirQualityValue(air_quality);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(AppServer, "GetAirQualityValue() failed: %" CHIP_ERROR_FORMAT ", skipping cluster update", err.Format()));

    int32_t * air_quality_ptr = new int32_t(air_quality);
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(AppTask::WriteAirQualityToAttribute,
                                                                     reinterpret_cast<intptr_t>(air_quality_ptr));
}

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(&CustomerAppTask::ButtonEventHandler);
#if SL_MATTER_DISPLAY_ENABLED
    GetLCD().SetCustomUI(AirQualitySensorUI::DrawUI);
#endif

    err = AppInstance().InitAirQualitySensor();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "InitAirQualitySensor() failed");
        appError(err);
    }

    return err;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

    CHIP_ERROR err = AppInstance().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    AppInstance().StartStatusLEDTimer();
#endif

    ChipLogDetail(AppServer, "App Task started");
    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            AppInstance().DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, 0);
        }
    }
}

void AppTask::UpdateAirQualitySensorUI()
{
// Update the LCD with the Stored value. Show QR Code if not provisioned
#if SL_MATTER_DISPLAY_ENABLED
    GetLCD().WriteDemoUI(false);
#if SL_MATTER_QR_CODE_ENABLED
    if (BaseApplication::GetProvisionStatus())
    {
        GetLCD().ShowQRCode(true);
    }
#endif // SL_MATTER_QR_CODE_ENABLED
#endif
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent aEvent           = {};
    aEvent.Type               = AppEvent::kEventType_Button;
    aEvent.ButtonEvent.Action = btnAction;

    if (button == APP_FUNCTION_BUTTON)
    {
        aEvent.Handler = BaseApplication::ButtonHandler;
        AppInstance().PostEvent(&aEvent);
    }
}

void AppTask::DMPostAttributeChangeCallback(const ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                            uint8_t * value)
{
    ClusterId clusterId                      = attributePath.mClusterId;
    [[maybe_unused]] AttributeId attributeId = attributePath.mAttributeId;
    ChipLogDetail(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == Clusters::Identify::Id)
    {
        ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
    }
}
