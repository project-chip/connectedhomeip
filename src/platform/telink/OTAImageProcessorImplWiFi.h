/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "OTAImageProcessorImpl.h"
#include "wifi/WiFiManager.h"

namespace chip {

namespace DeviceLayer {

class OTAImageProcessorImplWiFi : public OTAImageProcessorImpl
{
public:
    explicit OTAImageProcessorImplWiFi(ExternalFlashManager * flashHandler = nullptr) : OTAImageProcessorImpl(flashHandler) {}

    CHIP_ERROR PrepareDownload() override
    {
        CHIP_ERROR err = WiFiManager::Instance().SetLowPowerMode(false);
        if (err == CHIP_NO_ERROR)
        {
            return OTAImageProcessorImpl::PrepareDownload();
        }
        return err;
    };

    CHIP_ERROR Abort() override
    {
        CHIP_ERROR err = OTAImageProcessorImpl::Abort();
        if (err == CHIP_NO_ERROR)
        {
            return WiFiManager::Instance().SetLowPowerMode(true);
        }
        return err;
    };

    CHIP_ERROR Apply() override
    {
        CHIP_ERROR err = OTAImageProcessorImpl::Apply();
        if (err == CHIP_NO_ERROR)
        {
            return WiFiManager::Instance().SetLowPowerMode(true);
        }
        return err;
    };
};

} // namespace DeviceLayer
} // namespace chip
