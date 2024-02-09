/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "OTAProvider.h"
#include "ota-fw-placeholder.bin.h"

#if CONFIG_CHIP_OTA_PROVIDER
void chip::NXP::App::InitOTAServer()
{
    int res;
    chip::ota::DefaultOTAProviderUserConsent gUserConsentProvider;

    // Check if file exists. If not, open the file and write the OTA FW inside
    if (FS_CheckFileSize(OTA_FILE_NAME) <= 0)
    {
        res = FS_DeleteFile(OTA_FILE_NAME);

        if (res == ERR_OK)
        {
            res = FS_WriteBufferToFile(OTA_FILE_NAME, (uint8_t *) ota_fw_placeholder_bin, ota_fw_placeholder_bin_len);
        }

        if (res != ota_fw_placeholder_bin_len)
        {
            ChipLogError(SoftwareUpdate, "LittleFS file creation or writing failed: %d", res);
            return;
        }
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    RTBdxOtaSender * bdxOtaSender = gOtaProvider.GetBdxOtaSender();
    VerifyOrReturn(bdxOtaSender != nullptr);

    if (gOtaFilepath != nullptr)
    {
        gOtaProvider.SetOTAFilePath(gOtaFilepath);
        bdxOtaSender->SetFilepath(gOtaFilepath);
    }

    err = chip::Server::GetInstance().GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::BDX::Id,
                                                                                                        bdxOtaSender);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(SoftwareUpdate, "RegisterUnsolicitedMessageHandler failed: %s", chip::ErrorStr(err));
        return;
    }

    ChipLogDetail(SoftwareUpdate, "Using OTA file: %s", gOtaFilepath ? gOtaFilepath : "(none)");

    gOtaProvider.SetIgnoreQueryImageCount(gIgnoreQueryImageCount);
    gOtaProvider.SetIgnoreApplyUpdateCount(gIgnoreApplyUpdateCount);
    gOtaProvider.SetQueryImageStatus(gQueryImageStatus);
    gOtaProvider.SetApplyUpdateAction(gOptionUpdateAction);
    gOtaProvider.SetDelayedQueryActionTimeSec(gDelayedQueryActionTimeSec);
    gOtaProvider.SetDelayedApplyActionTimeSec(gDelayedApplyActionTimeSec);

    if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
    {
        gUserConsentProvider.SetGlobalUserConsentState(gUserConsentState);
        gOtaProvider.SetUserConsentDelegate(&gUserConsentProvider);
    }

    if (gUserConsentNeeded)
    {
        gOtaProvider.SetUserConsentNeeded(true);
    }

    if (gPollInterval != 0)
    {
        gOtaProvider.SetPollInterval(gPollInterval);
    }

    chip::app::Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, &gOtaProvider);
}
#endif
