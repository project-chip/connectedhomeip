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

#include "OTARcpImageProcessor.h"

#if defined(CONFIG_ENABLE_MULTI_IMAGE_OTA) && defined(CONFIG_AUTO_UPDATE_RCP) && defined(CONFIG_OPENTHREAD_BORDER_ROUTER)

#include <esp_err.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {

CHIP_ERROR OTARcpImageProcessor::Init(const SubImageHeader &)
{
    mRcpOtaHandle = 0;
    mStarted      = false;
    mInitialized  = true;
    return CHIP_NO_ERROR;
}

bool OTARcpImageProcessor::IsInitialized()
{
    return mInitialized;
}

CHIP_ERROR OTARcpImageProcessor::IsReadyForOTA(DeviceState & state)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    state = DeviceState::kReady;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARcpImageProcessor::Write(ByteSpan & block)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    // Open the RCP OTA session on the first chunk.
    if (!mStarted)
    {
        esp_err_t err = esp_rcp_ota_begin(&mRcpOtaHandle);
        VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                            ChipLogError(SoftwareUpdate, "esp_rcp_ota_begin failed: %s", esp_err_to_name(err)));
        mStarted = true;
    }

    // esp_rcp_ota_receive() may consume the chunk in more than one step, so feed until drained.
    const uint8_t * data = block.data();
    size_t remaining     = block.size();
    while (remaining > 0)
    {
        size_t received = 0;
        esp_err_t err   = esp_rcp_ota_receive(mRcpOtaHandle, data, remaining, &received);
        VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_WRITE_FAILED,
                            ChipLogError(SoftwareUpdate, "esp_rcp_ota_receive failed: %s", esp_err_to_name(err)));
        // The RCP image is fully consumed; any trailing bytes are not part of it.
        VerifyOrReturnValue(received > 0, CHIP_NO_ERROR);
        data += received;
        remaining -= received;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARcpImageProcessor::Finish()
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    esp_err_t err = esp_rcp_ota_end(mRcpOtaHandle);
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(SoftwareUpdate, "esp_rcp_ota_end failed: %s", esp_err_to_name(err)));
    mStarted = false;
    ChipLogProgress(SoftwareUpdate, "RCP image staged; it will be flashed on the next boot");
    return CHIP_NO_ERROR;
}

void OTARcpImageProcessor::Abort(AbortContext & context)
{
    VerifyOrReturn(mInitialized);
    ChipLogProgress(SoftwareUpdate, "OTARcpImageProcessor abort (reason=%u)", static_cast<unsigned>(context.reason));
    if (mStarted)
    {
        esp_rcp_ota_abort(mRcpOtaHandle);
        mStarted = false;
    }
    mInitialized = false;
}

} // namespace chip

#endif // CONFIG_ENABLE_MULTI_IMAGE_OTA && CONFIG_AUTO_UPDATE_RCP && CONFIG_OPENTHREAD_BORDER_ROUTER

