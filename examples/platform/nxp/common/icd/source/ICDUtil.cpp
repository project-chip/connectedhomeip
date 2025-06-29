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
#include <platform/nxp/zephyr/ota/OTAImageProcessorImpl.h>
#else
#include <platform/nxp/common/ota/OTAImageProcessorImpl.h>
#endif /* CONFIG_APP_FREERTOS_OS */
#endif /* CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR */

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
