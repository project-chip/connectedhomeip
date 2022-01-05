/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#pragma once

#include <cstdint>

#include "AmebaOTADownloader.h"
#include "AmebaOTARequestor.h"

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_ota_provider_announce = 0,
        kEventType_ota_provider_connect,
        kEventType_ota_provider_response,
        kEventType_ota_download_completed,
        kEventType_ota_download_failed,
    };

    uint16_t Type;

    union
    {
        struct
        {
            AmebaOTARequestor::OTAUpdateDetails * imageDatails;
        } OTAProviderResponseEvent;

        struct
        {
            chip::AmebaOTADownloader::ImageInfo * imageInfo;
        } OTADownloadCompletedEvent;
    };

    EventHandler Handler;
};
