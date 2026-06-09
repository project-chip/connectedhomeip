/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "IdentifierEffect.h"
#include "stm32wba65i_discovery.h"
#include "FreeRTOS.h"
#include "timers.h"

#include <platform/CHIPDeviceLayer.h>
#include <app/clusters/identify-server/identify-server.h>

#define IDENTIFIER_BLINK_LED (500) // in mS

chip::app::Clusters::Identify::EffectIdentifierEnum sIdentifyEffect =
        chip::app::Clusters::Identify::EffectIdentifierEnum::kStopEffect;

TimerHandle_t IdentifierLightTimer;
void TimerIdentifierLightEventHandler(TimerHandle_t xTimer);

/**********************************************************
 * Identify Callbacks
 *********************************************************/
void OnIdentifyStart(Identify*) {
    ChipLogProgress(Zcl, "OnIdentifyStart");
    if (xTimerIsTimerActive(IdentifierLightTimer) == 0) {
        xTimerStart(IdentifierLightTimer, 0);
    }
}

void OnIdentifyStop(Identify*) {
    ChipLogProgress(Zcl, "OnIdentifyStop");
    xTimerStop(IdentifierLightTimer, 0);
    BSP_LED_Off(LED_RED);
}

void OnTriggerIdentifyEffectCompleted(chip::System::Layer *systemLayer, void *appState) {
    ChipLogProgress(Zcl, "IDENTIFY : effect completed");
    sIdentifyEffect = chip::app::Clusters::Identify::EffectIdentifierEnum::kStopEffect;
}

void OnTriggerIdentifyEffect(Identify *identify) {
    uint16_t duration = 0;

    sIdentifyEffect = identify->mCurrentEffectIdentifier;

    switch (sIdentifyEffect)
    {
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "IDENTIFY : effect kBlink (1s)");
        duration = 1; /* on/off once. */
        break;
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "IDENTIFY : effect kBreathe (15s)");
        duration = 15; /* on/off over 1s and repeated 15 times. */
        break;
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "IDENTIFY : effect kOkay (3s)");
        duration = 3; /* turns green for 1s; non-colored light flashes twice */
        break;
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "IDENTIFY : effect kChannelChange (16s)");
        /* turns orange for 8s; non-colored light switches for brightness max 0.5s and then min 7.5s */
        duration = 16;
        break;
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        ChipLogProgress(Zcl, "IDENTIFY : effect kFinishEffect");
        duration = 1;
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        break;
    case chip::app::Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        ChipLogProgress(Zcl, "IDENTIFY : effect kStopEffect");
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        sIdentifyEffect = chip::app::Clusters::Identify::EffectIdentifierEnum::kStopEffect;
        break;
    default:
        ChipLogProgress(Zcl, "IDENTIFY : unknown identifier effect");
        sIdentifyEffect = chip::app::Clusters::Identify::EffectIdentifierEnum::kStopEffect;
        break;
    }

    if (duration != 0)
    {
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(duration),
                                                           OnTriggerIdentifyEffectCompleted,
                                                           identify);
    }
}

void TimerIdentifierLightEventHandler(TimerHandle_t xTimer) {
    BSP_LED_Toggle(LED_RED);
}

static Identify gIdentify = {
    chip::EndpointId { 1 },
    OnIdentifyStart,
    OnIdentifyStop,
    chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnTriggerIdentifyEffect,
};

CHIP_ERROR IdentifierEffect_Init(void) {
    CHIP_ERROR err = CHIP_NO_ERROR;

    IdentifierLightTimer = xTimerCreate("IdentifierLightTimer", // Just a text name, not used by the RTOS kernel
                                        pdMS_TO_TICKS(IDENTIFIER_BLINK_LED), // default timer period (in mS)
                                        true, // timer autoreload
                                        NULL, // timer identifier
                                        TimerIdentifierLightEventHandler); // timer callback
    if (IdentifierLightTimer == NULL) {
        err = CHIP_ERROR_NO_MEMORY;
    }
    return err;
}
