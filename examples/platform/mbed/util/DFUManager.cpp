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

#include "DFUManager.h"

#ifdef CHIP_OTA_REQUESTOR
#include <app/server/Server.h>
#endif // CHIP_OTA_REQUESTOR

#ifdef BOOT_ENABLED
#include <bootutil/bootutil.h>
#endif // BOOT_ENABLED

DFUManager DFUManager::sDFUMgr;

CHIP_ERROR DFUManager::Init(chip::Callback::Callback<OnUpdateAvailable> * onUpdateAvailable,
                            chip::Callback::Callback<OnUpdateApply> * onUpdateApply)
{
    int ret;
    CHIP_ERROR err = CHIP_NO_ERROR;

    mOnUpdateAvailableCallback = onUpdateAvailable;
    mOnUpdateApplyCallback     = onUpdateApply;

#ifdef BOOT_ENABLED
    ret = boot_set_confirmed();
    if (ret == 0)
    {
        ChipLogProgress(SoftwareUpdate, "Boot confirmed");
    }
    else
    {
        ChipLogError(SoftwareUpdate, "Failed to confirm boot: %d", ret);
        err = CHIP_ERROR_INTERNAL;
    }

#if defined(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION) && defined(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING)
    ChipLogProgress(SoftwareUpdate, "Current software version: [%ld] %s", uint32_t(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION),
                    CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
#endif // CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION && CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#endif // BOOT_ENABLED

#ifdef CHIP_OTA_REQUESTOR
    SetRequestorInstance(&mRequestorCore);
    mRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    mRequestorCore.Init(chip::Server::GetInstance(), mRequestorStorage, mRequestorDriver, mDownloader);
    mImageProcessor.SetOTADownloader(&mDownloader);
    mDownloader.SetImageProcessorDelegate(&mImageProcessor);
    mRequestorDriver.Init(&mRequestorCore, &mImageProcessor, &mOnOtaUpdateAvailableCallback, &mOnOtaUpdateApplyCallback);
#endif // CHIP_OTA_REQUESTOR

    return err;
}

#ifdef CHIP_OTA_REQUESTOR
bool DFUManager::OnOtaUpdateAvailableHandler(void * context, const chip::UpdateDescription & desc)
{
    DFUManager * dfuMgr = reinterpret_cast<DFUManager *>(context);
    if (dfuMgr->mOnUpdateAvailableCallback)
    {
        return dfuMgr->mOnUpdateAvailableCallback->mCall(dfuMgr->mOnUpdateAvailableCallback->mContext, desc.softwareVersion,
                                                         desc.softwareVersionStr);
    }

    return true;
}

bool DFUManager::OnOtaUpdateApplyHandler(void * context)
{
    DFUManager * dfuMgr = reinterpret_cast<DFUManager *>(context);
    if (dfuMgr->mOnUpdateApplyCallback)
    {
        return dfuMgr->mOnUpdateApplyCallback->mCall(dfuMgr->mOnUpdateAvailableCallback->mContext);
    }

    return true;
}
#endif
