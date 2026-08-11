/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/**********************************************************
 * Includes
 *********************************************************/

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "CustomerAppTask.h"
#include "LEDWidget.h"
#include "RefrigeratorConfig.h"

#if SL_MATTER_DISPLAY_ENABLED
#include "RefrigeratorUI.h"
#include "lcd.h"
#if SL_MATTER_QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // SL_MATTER_QR_CODE_ENABLED
#endif // SL_MATTER_DISPLAY_ENABLED

#if defined(ENABLE_CHIP_SHELL)
#include "EventHandlerLibShell.h"
#endif // ENABLE_CHIP_SHELL

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/temperature-control-server/temperature-control-server.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <static-supported-temperature-levels.h>

#ifdef SL_MATTER_ENABLE_AWS
#include "MatterAwsControl.h"
#endif // SL_MATTER_ENABLE_AWS

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define APP_FUNCTION_BUTTON 0
#define APP_REFRIGERATOR 1

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
using namespace chip::TLV;
using namespace chip::DeviceLayer;
using namespace chip::literals;
using chip::Protocols::InteractionModel::Status;

template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

namespace RefAndTempAttr = chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Attributes;
namespace RefAlarmAttr   = chip::app::Clusters::RefrigeratorAlarm::Attributes;
namespace TempCtrlAttr   = chip::app::Clusters::TemperatureControl::Attributes;

/**********************************************************
 * Variable declarations
 *********************************************************/

namespace {

CustomerAppTask & AppInstance()
{
    return CustomerAppTask::GetAppTask();
}

// Endpoint composition. Defaults live in RefrigeratorConfig.h
constexpr EndpointId kRefEndpointId           = REFRIGERATOR_ENDPOINT;
constexpr EndpointId kColdCabinetEndpointId   = COLD_CABINET_ENDPOINT;
constexpr EndpointId kFreezeCabinetEndpointId = FREEZE_CABINET_ENDPOINT;

app::Clusters::TemperatureControl::AppSupportedTemperatureLevelsDelegate sAppSupportedTemperatureLevelsDelegate;

// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr const uint8_t kNamespaceRefrigerator = 0x41;
// Refrigerator Namespace: 0x41, tag 0x00 (Refrigerator)
constexpr const uint8_t kTagRefrigerator = 0x00;
// Refrigerator Namespace: 0x41, tag 0x01 (Freezer)
constexpr const uint8_t kTagFreezer                                                = 0x01;
const Clusters::Descriptor::Structs::SemanticTagStruct::Type refrigeratorTagList[] = { { .namespaceID = kNamespaceRefrigerator,
                                                                                         .tag         = kTagRefrigerator } };
const Clusters::Descriptor::Structs::SemanticTagStruct::Type freezerTagList[]      = { { .namespaceID = kNamespaceRefrigerator,
                                                                                         .tag         = kTagFreezer } };

ModeBase::Instance * gCabinetModeInstance = nullptr;

// Static SupportedModes definition for the cabinet mode cluster
const ModeTagStructType kModeTagsNormal[1]      = { { .value = to_underlying(ModeTag::kAuto) } };
const ModeTagStructType kModeTagsRapidCool[1]   = { { .value = to_underlying(ModeTag::kRapidCool) } };
const ModeTagStructType kModeTagsRapidFreeze[2] = { { .value = to_underlying(ModeBase::ModeTag::kMax) },
                                                    { .value = to_underlying(ModeTag::kRapidFreeze) } };

const Clusters::detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
    Clusters::detail::Structs::ModeOptionStruct::Type{
        .label = "Normal"_span, .mode = ModeNormal, .modeTags = DataModel::List<const ModeTagStructType>(kModeTagsNormal) },
    Clusters::detail::Structs::ModeOptionStruct::Type{ .label    = "Rapid Cool"_span,
                                                       .mode     = ModeRapidCool,
                                                       .modeTags = DataModel::List<const ModeTagStructType>(kModeTagsRapidCool) },
    Clusters::detail::Structs::ModeOptionStruct::Type{ .label    = "Rapid Freeze"_span,
                                                       .mode     = ModeRapidFreeze,
                                                       .modeTags = DataModel::List<const ModeTagStructType>(kModeTagsRapidFreeze) },
};

int8_t ConvertToPrintableTemp(int16_t temperature)
{
    constexpr uint8_t kRoundUpValue = 50;

    // Round up the temperature as we won't print decimals on LCD
    // Is it a negative temperature
    if (temperature < 0)
    {
        temperature -= kRoundUpValue;
    }
    else
    {
        temperature += kRoundUpValue;
    }

    return static_cast<int8_t>(temperature / 100);
}

} // namespace

/**********************************************************
 * AppTask Definitions
 *********************************************************/

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(&CustomerAppTask::ButtonEventHandler);

    err = AppInstance().InitRefrigerator();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AppTask::InitRefrigerator() failed");
        appError(err);
    }

#if defined(ENABLE_CHIP_SHELL)
    err = RegisterRefrigeratorEvents();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("RegisterRefrigeratorEvents() failed");
        appError(err);
    }
#endif // ENABLE_CHIP_SHELL

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

    CHIP_ERROR err = GetAppTask().BaseApplication::Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    GetAppTask().StartStatusLEDTimer();
#endif

    ChipLogDetail(AppServer, "App Task started");
    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            GetAppTask().DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, 0);
        }
    }
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

CHIP_ERROR AppTask::InitRefrigerator()
{
    ReturnErrorOnFailure(SetTreeCompositionForEndpoint(kRefEndpointId));
    ReturnErrorOnFailure(SetParentEndpointForEndpoint(kColdCabinetEndpointId, kRefEndpointId));
    ReturnErrorOnFailure(SetParentEndpointForEndpoint(kFreezeCabinetEndpointId, kRefEndpointId));

    // set TagList
    ReturnErrorOnFailure(SetTagList(kColdCabinetEndpointId,
                                    Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(refrigeratorTagList)));
    ReturnErrorOnFailure(
        SetTagList(kFreezeCabinetEndpointId, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(freezerTagList)));

    app::Clusters::TemperatureControl::SetDelegate(&sAppSupportedTemperatureLevelsDelegate);
    return CHIP_NO_ERROR;
}

void AppTask::DMPostAttributeChangeCallback(const ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                            uint8_t * value)
{
    EndpointId endpointId   = attributePath.mEndpointId;
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogDetail(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    switch (clusterId)
    {
    case Clusters::Identify::Id:
        ChipLogProgress(Zcl, "Identify cluster ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
        break;

    case Clusters::RefrigeratorAlarm::Id:
        switch (attributeId)
        {
        case RefAlarmAttr::Mask::Id:
            mMask = static_cast<chip::app::Clusters::RefrigeratorAlarm::AlarmBitmap>(static_cast<uint32_t>(*value));
            RefAlarmAttr::Mask::Set(endpointId, mMask);
            break;
        case RefAlarmAttr::State::Id:
            mState = static_cast<chip::app::Clusters::RefrigeratorAlarm::AlarmBitmap>(static_cast<uint32_t>(*value));
            RefAlarmAttr::State::Set(endpointId, mState);
            break;
        default:
            ChipLogError(AppServer, "Unhandled Refrigerator Alarm attribute " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
            return;
        }
#ifdef SL_MATTER_ENABLE_AWS
        matterAws::control::AttributeHandler(endpointId, attributeId);
#endif // SL_MATTER_ENABLE_AWS
        break;

    case Clusters::TemperatureControl::Id:
        switch (attributeId)
        {
        case TempCtrlAttr::TemperatureSetpoint::Id:
            mTemperatureSetpoint = ConvertToPrintableTemp(static_cast<int16_t>(*value));
            break;
        default:
            ChipLogError(AppServer, "Unhandled Temperature Control attribute " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
            return;
        }
#ifdef SL_MATTER_ENABLE_AWS
        matterAws::control::AttributeHandler(endpointId, attributeId);
#endif // SL_MATTER_ENABLE_AWS
        break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// ModeBase::Delegate implementation for RefrigeratorAndTemperatureControlledCabinetMode.
// -----------------------------------------------------------------------------

CHIP_ERROR AppTask::Init()
{
    return CHIP_NO_ERROR;
}

void AppTask::HandleChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    const uint8_t currentMode = GetInstance()->GetCurrentMode();

    // Disallow transitions between Normal (0) and Rapid Freeze (2)
    if ((currentMode == ModeNormal && newMode == ModeRapidFreeze) || (currentMode == ModeRapidFreeze && newMode == ModeNormal))
    {
        response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
        response.statusText.SetValue("Direct transition between Normal and Rapid Freeze not allowed"_span);
        return;
    }

    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR AppTask::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR AppTask::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (tags.size() < kModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
    tags.reduce_size(kModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}

void AppTask::CabinetModeClusterInit(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == kRefEndpointId); // this cluster is enabled on refrigerator endpoint (1 in default implementation)
    VerifyOrDie(gCabinetModeInstance == nullptr);
    gCabinetModeInstance =
        new ModeBase::Instance(&AppInstance(), kRefEndpointId, RefrigeratorAndTemperatureControlledCabinetMode::Id,
                               chip::to_underlying(ModeBase::Feature::kOnOff));
    VerifyOrDie(gCabinetModeInstance->Init() == CHIP_NO_ERROR);
}

void MatterRefrigeratorAndTemperatureControlledCabinetModeClusterInitCallback(chip::EndpointId endpointId)
{
    AppInstance().CabinetModeClusterInit(endpointId);
}
