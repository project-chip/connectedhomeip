/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "OTAUtil.h"

#if CONFIG_CHIP_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/nrfconnect/OTAImageProcessorImpl.h>
#include <zephyr/dfu/mcuboot.h>
#endif

using namespace chip;
using namespace chip::DeviceLayer;

#if CONFIG_CHIP_OTA_REQUESTOR

namespace {

DefaultOTARequestorStorage sOTARequestorStorage;
DefaultOTARequestorDriver sOTARequestorDriver;
chip::BDXDownloader sBDXDownloader;
chip::DefaultOTARequestor sOTARequestor;
} // namespace

// compile-time factory method
OTAImageProcessorImpl & GetOTAImageProcessor()
{
#if CONFIG_PM_DEVICE && CONFIG_NORDIC_QSPI_NOR
    static OTAImageProcessorImpl sOTAImageProcessor(&GetFlashHandler());
#else
    static OTAImageProcessorImpl sOTAImageProcessor;
#endif
    return sOTAImageProcessor;
}

void InitBasicOTARequestor()
{
    VerifyOrReturn(GetRequestorInstance() == nullptr);

    OTAImageProcessorImpl & imageProcessor = GetOTAImageProcessor();
    imageProcessor.SetOTADownloader(&sBDXDownloader);
    sBDXDownloader.SetImageProcessorDelegate(&imageProcessor);
    sOTARequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
    sOTARequestor.Init(Server::GetInstance(), sOTARequestorStorage, sOTARequestorDriver, sBDXDownloader);
    chip::SetRequestorInstance(&sOTARequestor);
    sOTARequestorDriver.Init(&sOTARequestor, &imageProcessor);
    imageProcessor.TriggerFlashAction(ExternalFlashManager::Action::SLEEP);
}

void OtaConfirmNewImage()
{
#ifndef CONFIG_SOC_SERIES_NRF53X
    /* Check if the image is run in the REVERT mode and eventually
    confirm it to prevent reverting on the next boot.
    On nRF53 target there is not way to verify current swap type
    because we use permanent swap so we can skip it. */
    VerifyOrReturn(mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT);
#endif

    OTAImageProcessorImpl & imageProcessor = GetOTAImageProcessor();
    if (!boot_is_img_confirmed())
    {
        CHIP_ERROR err = System::MapErrorZephyr(boot_write_img_confirmed());
        if (CHIP_NO_ERROR == err)
        {
            imageProcessor.SetImageConfirmed();
            ChipLogProgress(SoftwareUpdate, "New firmware image confirmed");
        }
        else
        {
            ChipLogError(SoftwareUpdate, "Failed to confirm firmware image, it will be reverted on the next boot");
        }
    }
}

#endif

ExternalFlashManager & GetFlashHandler()
{
    static ExternalFlashManager sFlashHandler;
    return sFlashHandler;
}
