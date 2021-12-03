/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "AppEvent.h"
#include <OTADownloaderImpl.h>
#include <OTARequestorImpl.h>

class AppTask
{
public:
    int StartApp();

private:
    friend AppTask & GetAppTask(void);

    int Init();

    void PostEvent(AppEvent * aEvent);
    void DispatchEvent(const AppEvent * event);

    static void OnOtaEventHandler(AppEvent * aEvent);

    static void OnConnectProviderCallback(chip::NodeId nodeId, chip::FabricIndex fabricIndex,
                                          chip::Optional<chip::ByteSpan> ipAddress);

    static void OnProviderResponseCallback(OTARequestorImpl::OTAUpdateDetails * updateDetails);

    static void OnDownloadCompletedCallback(OTADownloaderImpl::ImageInfo * imageInfo);

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
