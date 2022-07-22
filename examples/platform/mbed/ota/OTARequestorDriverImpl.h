/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/* This file contains the declarations for OTARequestorDriver, a platform-agnostic
 * interface for processing firmware update.
 */

#pragma once

#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/OTARequestorDriver.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

#include "blockdevice/BlockDevice.h"

namespace chip {

namespace DeviceLayer {

typedef bool (*OnOtaUpdateAvailable)(void * context, const UpdateDescription &);
typedef bool (*OnOtaUpdateApply)(void * context);

class OTARequestorDriverImpl : public DefaultOTARequestorDriver
{

    friend class DefaultOTARequestorDriver;

public:
    void Init(OTARequestorInterface * requestor, OTAImageProcessorInterface * processor,
              Callback::Callback<OnOtaUpdateAvailable> * onUpdateAvailable = nullptr,
              Callback::Callback<OnOtaUpdateApply> * onUpdateApply         = nullptr)
    {
        mRequestor                   = requestor;
        mImageProcessor              = processor;
        onOtaUpdateAvailableCallback = onUpdateAvailable;
        onOtaUpdateApplyCallback     = onUpdateApply;
    }

    void UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay);
    void UpdateDownloaded(void);

private:
    Callback::Callback<OnOtaUpdateAvailable> * onOtaUpdateAvailableCallback;
    Callback::Callback<OnOtaUpdateApply> * onOtaUpdateApplyCallback;
};

} // namespace DeviceLayer
} // namespace chip
