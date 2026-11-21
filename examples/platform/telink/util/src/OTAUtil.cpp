/*
 *    Copyright (c) 2022-2026 Project CHIP Authors
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

#ifdef CONFIG_CHIP_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/CodegenIntegration.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <platform/telink/OTAImageProcessorImpl.h>

using namespace chip;
using namespace chip::DeviceLayer;
#endif

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_TELINK_OTA_MULTI_IMAGE_TEST
#include <lib/support/CodeUtils.h>
#include <zephyr/sys/crc.h>
#endif

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

#ifdef CONFIG_CHIP_OTA_REQUESTOR

namespace {

DefaultOTARequestorStorage sOTARequestorStorage;
DefaultOTARequestorDriver sOTARequestorDriver;
chip::BDXDownloader sBDXDownloader;
chip::DefaultOTARequestor sOTARequestor;
chip::DeviceLayer::OTAImageProcessorImpl sOTAImageProcessor;

#ifdef CONFIG_TELINK_OTA_MULTI_IMAGE_TEST
class TestExternalOTAImageHandler : public chip::DeviceLayer::ExternalOTAImageHandler
{
public:
    CHIP_ERROR Begin(uint32_t imageIndex, uint32_t imageSize) override
    {
        mSize  = imageSize;
        mBytes = 0;
        mCrc   = 0;
        LOG_INF("External OTA test: image %u, size %u", static_cast<unsigned>(imageIndex), static_cast<unsigned>(imageSize));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Write(chip::ByteSpan data) override
    {
        VerifyOrReturnError(data.size() <= mSize - mBytes, CHIP_ERROR_INVALID_ARGUMENT);
        mCrc = crc32_ieee_update(mCrc, data.data(), data.size());
        mBytes += static_cast<uint32_t>(data.size());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR End() override
    {
        VerifyOrReturnError(mBytes == mSize, CHIP_ERROR_INVALID_ARGUMENT);
        LOG_INF("External OTA test: received %u bytes, CRC32 0x%08x", static_cast<unsigned>(mBytes), static_cast<unsigned>(mCrc));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Apply() override
    {
        LOG_INF("External OTA test: apply");
        return CHIP_NO_ERROR;
    }

    void Abort() override { LOG_INF("External OTA test: abort"); }

private:
    uint32_t mSize  = 0;
    uint32_t mBytes = 0;
    uint32_t mCrc   = 0;
};

TestExternalOTAImageHandler sExternalOTAImageHandler;
#endif
} // namespace

void InitBasicOTARequestor()
{
    VerifyOrReturn(GetRequestorInstance() == nullptr);

#ifdef CONFIG_TELINK_OTA_MULTI_IMAGE_TEST
    OTAImageProcessorImpl::SetExternalImageHandler(&sExternalOTAImageHandler);
#endif
    sOTAImageProcessor.SetOTADownloader(&sBDXDownloader);
    sBDXDownloader.SetImageProcessorDelegate(&sOTAImageProcessor);
    sOTARequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
    LogErrorOnFailure(sOTARequestor.Init(Server::GetInstance(), sOTARequestorStorage, sOTARequestorDriver, sBDXDownloader,
                                         GetOTARequestorAttributes(), GetDefaultOTARequestorEventGenerator()));
    chip::SetRequestorInstance(&sOTARequestor);
    sOTARequestorDriver.Init(&sOTARequestor, &sOTAImageProcessor);
}

#endif

void OtaConfirmNewImage()
{
    if (mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT)
    {
        int img_confirmation = boot_write_img_confirmed();
        if (img_confirmation)
        {
            LOG_ERR("Image not confirmed %d. Will be reverted!", img_confirmation);
        }
        else
        {
            LOG_INF("Image confirmed");
        }
    }
}
