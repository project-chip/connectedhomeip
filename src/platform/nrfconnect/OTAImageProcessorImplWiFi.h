/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
