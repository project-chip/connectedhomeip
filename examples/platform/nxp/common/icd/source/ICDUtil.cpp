/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#include "ICDUtil.h"

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#ifndef CONFIG_APP_FREERTOS_OS
#include <platform/Zephyr/OTAImageProcessorImpl.h>
#else
#include <platform/nxp/common/ota/OTAImageProcessorImpl.h>
#endif /* CONFIG_APP_FREERTOS_OS */
#endif /* CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR */

#if CONFIG_NXP_USE_POWER_DOWN
#include "PWR_Interface.h"
#include <app/icd/server/ICDConfigurationData.h>

chip::NXP::App::NxpICDObserver chip::NXP::App::NxpICDObserver::sICDObserver;
#endif // CONFIG_NXP_USE_POWER_DOWN

chip::NXP::App::ICDUtil chip::NXP::App::ICDUtil::sICDUtil;

CHIP_ERROR chip::NXP::App::ICDUtil::OnSubscriptionRequested(chip::app::ReadHandler & aReadHandler,
                                                            chip::Transport::SecureSession & aSecureSession)
{
    uint16_t reqMinInterval           = 0;
    uint16_t reqMaxInterval           = 0;
    static uint16_t maxOfMinIntervals = 0;

    aReadHandler.GetReportingIntervals(reqMinInterval, reqMaxInterval);

    if (reqMinInterval > maxOfMinIntervals)
    {
        maxOfMinIntervals = reqMinInterval;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    chip::OTAImageProcessorImpl::GetDefaultInstance().SetRebootDelaySec(maxOfMinIntervals);
#endif
    return CHIP_NO_ERROR;
}

#if CONFIG_NXP_USE_POWER_DOWN
chip::NXP::App::NxpICDObserver::~NxpICDObserver()
{
    // Ensure low-power constraints are released
    OnEnterActiveMode();
}

void chip::NXP::App::NxpICDObserver::OnEnterIdleMode()
{
    // In Idle Mode remove the DeepSleep constraint and let the device go into
    // PowerDown if the polling period exceeds CONFIG_NXP_POWER_DOWN_POLL_THRESHOLD
    System::Clock::Milliseconds32 slowPollInterval = chip::ICDConfigurationData::GetInstance().GetSlowPollingInterval();
    if ((slowPollInterval >= chip::System::Clock::Seconds32(CONFIG_NXP_POWER_DOWN_POLL_THRESHOLD)) && (mLPState != PowerDown))
    {
        ::PWR_SetLowPowerModeConstraint(PWR_PowerDown);
        ::PWR_ReleaseLowPowerModeConstraint(PWR_DeepSleep);
        mLPState = PowerDown;
    }
}

void chip::NXP::App::NxpICDObserver::OnEnterActiveMode()
{
    // In Active Mode add a DeepSleep constraint to prevent the device from
    // going all the way to PowerDown state
    if (mLPState != DeepSleep)
    {
        ::PWR_SetLowPowerModeConstraint(PWR_DeepSleep);
        ::PWR_ReleaseLowPowerModeConstraint(PWR_PowerDown);
        mLPState = DeepSleep;
    }
}

void chip::NXP::App::NxpICDObserver::OnTransitionToIdle()
{
    // Do nothing.
}

void chip::NXP::App::NxpICDObserver::OnICDModeChange()
{
    // Do nothing.
}
#endif // CONFIG_NXP_USE_POWER_DOWN
