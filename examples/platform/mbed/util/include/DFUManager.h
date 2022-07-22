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

/**
 *    @file
 *      Provides an implementation of Device Firmware Upgrade using Matter OTA.
 */

#pragma once

#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#ifdef CHIP_OTA_REQUESTOR
#include <BDXDownloader.h>
#include <DefaultOTARequestor.h>
#include <DefaultOTARequestorStorage.h>
#include <OTAImageProcessorImpl.h>
#include <OTARequestorDriverImpl.h>
#endif // CHIP_OTA_REQUESTOR

typedef bool (*OnUpdateAvailable)(void * context, uint32_t softwareVersion, chip::CharSpan softwareVersionString);
typedef bool (*OnUpdateApply)(void * context);

class DFUManager
{
public:
    CHIP_ERROR Init(chip::Callback::Callback<OnUpdateAvailable> * onUpdateAvailable = nullptr,
                    chip::Callback::Callback<OnUpdateApply> * onUpdateApply         = nullptr);

private:
#ifdef CHIP_OTA_REQUESTOR
    DFUManager() :
        mOnOtaUpdateAvailableCallback(OnOtaUpdateAvailableHandler, this), mOnOtaUpdateApplyCallback(OnOtaUpdateApplyHandler, this)
    {}
#endif
    friend DFUManager & GetDFUManager(void);

    static DFUManager sDFUMgr;

#ifdef CHIP_OTA_REQUESTOR
    chip::DefaultOTARequestor mRequestorCore;
    chip::DefaultOTARequestorStorage mRequestorStorage;
    chip::DeviceLayer::OTARequestorDriverImpl mRequestorDriver;
    chip::BDXDownloader mDownloader;
    chip::OTAImageProcessorImpl mImageProcessor;

    static bool OnOtaUpdateAvailableHandler(void * context, const chip::UpdateDescription & desc);
    static bool OnOtaUpdateApplyHandler(void * context);

    chip::Callback::Callback<chip::DeviceLayer::OnOtaUpdateAvailable> mOnOtaUpdateAvailableCallback;
    chip::Callback::Callback<chip::DeviceLayer::OnOtaUpdateApply> mOnOtaUpdateApplyCallback;
#endif // CHIP_OTA_REQUESTOR

    chip::Callback::Callback<OnUpdateAvailable> * mOnUpdateAvailableCallback;
    chip::Callback::Callback<OnUpdateApply> * mOnUpdateApplyCallback;
};

inline DFUManager & GetDFUManager(void)
{
    return DFUManager::sDFUMgr;
}
