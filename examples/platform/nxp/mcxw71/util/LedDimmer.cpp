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

#include "LedDimmer.h"
#include "fsl_common.h"
#include "fsl_port.h"
#include "fsl_tpm.h"

#include <lib/support/logging/CHIPLogging.h>

#define BOARD_TPM_BASEADDR TPM0
#define TPM_SOURCE_CLOCK CLOCK_GetIpFreq(kCLOCK_Tpm0)
#define BOARD_FIRST_TPM_CHANNEL kTPM_Chnl_0
#define BOARD_SECOND_TPM_CHANNEL kTPM_Chnl_1
#define BOARD_THIRD_TPM_CHANNEL kTPM_Chnl_2
#ifndef TPM_LED_ON_LEVEL
#define TPM_LED_ON_LEVEL kTPM_HighTrue
#endif
#ifndef DEMO_PWM_FREQUENCY
#define DEMO_PWM_FREQUENCY (24000U)
#endif

namespace chip::NXP::App {

volatile uint8_t updatedDutycycle = 0U;

static void initConfig()
{
    const port_pin_config_t porta20_pin17_config = { /* Internal pull-up/down resistor is disabled */
                                                     (uint16_t) kPORT_PullDisable,
                                                     /* Low internal pull resistor value is selected. */
                                                     (uint16_t) kPORT_LowPullResistor,
                                                     /* Fast slew rate is configured */
                                                     (uint16_t) kPORT_FastSlewRate,
                                                     /* Passive input filter is disabled */
                                                     (uint16_t) kPORT_PassiveFilterDisable,
                                                     /* Open drain output is disabled */
                                                     (uint16_t) kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     (uint16_t) kPORT_LowDriveStrength,
                                                     /* Normal drive strength is configured */
                                                     (uint16_t) kPORT_NormalDriveStrength,
                                                     /* Pin is configured as TPM0_CH0 */
                                                     (uint16_t) kPORT_MuxAlt5,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     (uint16_t) kPORT_UnlockRegister
    };
    /* PORTA20 (pin 17) is configured as TPM0_CH0 */
    PORT_SetPinConfig(PORTA, 20U, &porta20_pin17_config);

    const port_pin_config_t porta21_pin18_config = { /* Internal pull-up/down resistor is disabled */
                                                     (uint16_t) kPORT_PullDisable,
                                                     /* Low internal pull resistor value is selected. */
                                                     (uint16_t) kPORT_LowPullResistor,
                                                     /* Fast slew rate is configured */
                                                     (uint16_t) kPORT_FastSlewRate,
                                                     /* Passive input filter is disabled */
                                                     (uint16_t) kPORT_PassiveFilterDisable,
                                                     /* Open drain output is disabled */
                                                     (uint16_t) kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     (uint16_t) kPORT_LowDriveStrength,
                                                     /* Normal drive strength is configured */
                                                     (uint16_t) kPORT_NormalDriveStrength,
                                                     /* Pin is configured as TPM0_CH0 */
                                                     (uint16_t) kPORT_MuxAlt5,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     (uint16_t) kPORT_UnlockRegister
    };
    /* PORTA21 (pin 18) is configured as TPM0_CH0 */
    PORT_SetPinConfig(PORTA, 21U, &porta21_pin18_config);

    const port_pin_config_t porta19_pin14_config = { /* Internal pull-up/down resistor is disabled */
                                                     (uint16_t) kPORT_PullDisable,
                                                     /* Low internal pull resistor value is selected. */
                                                     (uint16_t) kPORT_LowPullResistor,
                                                     /* Fast slew rate is configured */
                                                     (uint16_t) kPORT_FastSlewRate,
                                                     /* Passive input filter is disabled */
                                                     (uint16_t) kPORT_PassiveFilterDisable,
                                                     /* Open drain output is disabled */
                                                     (uint16_t) kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     (uint16_t) kPORT_LowDriveStrength,
                                                     /* Normal drive strength is configured */
                                                     (uint16_t) kPORT_NormalDriveStrength,
                                                     /* Pin is configured as TPM0_CH0 */
                                                     (uint16_t) kPORT_MuxAlt5,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     (uint16_t) kPORT_UnlockRegister
    };
    /* PORTA19 (pin 14) is configured as TPM0_CH0 */
    PORT_SetPinConfig(PORTA, 19U, &porta19_pin14_config);
}

void LedDimmer::Init(uint8_t index, bool inverted)
{
    tpm_config_t tpmInfo;
    tpm_chnl_pwm_signal_param_t tpmParam[3];

    initConfig();

    /* TPM 0 Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_Tpm0);
    /* Set the source for the LPIT module */
    CLOCK_SetIpSrc(kCLOCK_Tpm0, kCLOCK_IpSrcFro6M);

    /* Fill in the TPM config struct with the default settings */
    TPM_GetDefaultConfig(&tpmInfo);
    /* Calculate the clock division based on the PWM frequency to be obtained */
    tpmInfo.prescale = TPM_CalculateCounterClkDiv(BOARD_TPM_BASEADDR, DEMO_PWM_FREQUENCY, TPM_SOURCE_CLOCK);
    /* Initialize TPM module */
    TPM_Init(BOARD_TPM_BASEADDR, &tpmInfo);

    /* Configure tpm params with frequency 24kHZ */
    tpmParam[0].chnlNumber = (tpm_chnl_t) BOARD_FIRST_TPM_CHANNEL;
#if (defined(FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT) && FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT)
    tpmParam[0].pauseLevel = kTPM_ClearOnPause;
#endif
    tpmParam[0].level            = TPM_LED_ON_LEVEL;
    tpmParam[0].dutyCyclePercent = updatedDutycycle;

    tpmParam[1].chnlNumber = (tpm_chnl_t) BOARD_SECOND_TPM_CHANNEL;
#if (defined(FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT) && FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT)
    tpmParam[1].pauseLevel = kTPM_ClearOnPause;
#endif
    tpmParam[1].level            = TPM_LED_ON_LEVEL;
    tpmParam[1].dutyCyclePercent = updatedDutycycle;

    tpmParam[2].chnlNumber = (tpm_chnl_t) BOARD_THIRD_TPM_CHANNEL;
#if (defined(FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT) && FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT)
    tpmParam[2].pauseLevel = kTPM_ClearOnPause;
#endif
    tpmParam[2].level            = TPM_LED_ON_LEVEL;
    tpmParam[2].dutyCyclePercent = updatedDutycycle;

    if (kStatus_Success !=
        TPM_SetupPwm(BOARD_TPM_BASEADDR, tpmParam, 2U, kTPM_EdgeAlignedPwm, DEMO_PWM_FREQUENCY, TPM_SOURCE_CLOCK))
    {
        return;
    }

    TPM_StartTimer(BOARD_TPM_BASEADDR, kTPM_SystemClock);
}

void LedDimmer::Set(uint8_t level)
{
    uint8_t control;

    mState = level != 0;

    updatedDutycycle = static_cast<int>(level * 90) / 255;

    control = TPM_GetChannelContorlBits(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_FIRST_TPM_CHANNEL);

    /* Disable output on each channel of the pair before updating the dutycycle */
    TPM_DisableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_FIRST_TPM_CHANNEL);
    TPM_DisableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_SECOND_TPM_CHANNEL);
    TPM_DisableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_THIRD_TPM_CHANNEL);

    /* Update PWM duty cycle */
    if ((kStatus_Success ==
         TPM_UpdatePwmDutycycle(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_FIRST_TPM_CHANNEL, kTPM_EdgeAlignedPwm, updatedDutycycle)) &&
        (kStatus_Success ==
         TPM_UpdatePwmDutycycle(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_SECOND_TPM_CHANNEL, kTPM_EdgeAlignedPwm,
                                updatedDutycycle)) &&
        (kStatus_Success ==
         TPM_UpdatePwmDutycycle(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_THIRD_TPM_CHANNEL, kTPM_EdgeAlignedPwm, updatedDutycycle)))
    {
        ChipLogError(NotSpecified, "TPM: Duty cycle updated successfully");
    }
    else
    {
        ChipLogError(NotSpecified, "ERR: Duty cycle failed to updated");
    }

    /* Start output on each channel of the pair with updated dutycycle */
    TPM_EnableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_FIRST_TPM_CHANNEL, control);
    TPM_EnableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_SECOND_TPM_CHANNEL, control);
    TPM_EnableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t) BOARD_THIRD_TPM_CHANNEL, control);
}

} // namespace chip::NXP::App
