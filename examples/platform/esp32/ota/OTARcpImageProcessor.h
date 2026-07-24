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

#include <sdkconfig.h>

#if defined(CONFIG_ENABLE_MULTI_IMAGE_OTA) && defined(CONFIG_AUTO_UPDATE_RCP) && defined(CONFIG_OPENTHREAD_BORDER_ROUTER)

#include <esp_rcp_ota.h>
#include <lib/support/Span.h>
#include <platform/ESP32/multi_ota/SubImageProcessor.h>

namespace chip {

/// Image ID for the OpenThread RCP co-processor firmware sub-image (platform-reserved range).
constexpr OTAProcessorTag kRcpImageProcessorTag = 2;

/**
 * @brief Sub-image processor that stages an OpenThread RCP firmware image via esp_rcp_ota.
 *
 * The sub-image payload is a standalone esp_rcp_ota image (RCP firmware only). Its bytes are
 * handed to esp_rcp_ota_receive() and committed to the RCP staging partition on Finish(). The
 * staged image is flashed to the RCP at the next boot by esp_rcp_update (CONFIG_AUTO_UPDATE_RCP),
 * so Apply() is a no-op.
 */
class OTARcpImageProcessor : public SubImageProcessor
{
public:
    CHIP_ERROR Init(const SubImageHeader & entry) override;
    bool IsInitialized() override;
    CHIP_ERROR IsReadyForOTA(DeviceState & state) override;
    CHIP_ERROR Write(ByteSpan & block) override;
    CHIP_ERROR Finish() override;
    void Abort(AbortContext & context) override;

private:
    esp_rcp_ota_handle_t mRcpOtaHandle = 0;
    bool mInitialized                  = false;
    bool mStarted                      = false; // esp_rcp_ota_begin() has been called
};

} // namespace chip

#endif // CONFIG_ENABLE_MULTI_IMAGE_OTA && CONFIG_AUTO_UPDATE_RCP && CONFIG_OPENTHREAD_BORDER_ROUTER
