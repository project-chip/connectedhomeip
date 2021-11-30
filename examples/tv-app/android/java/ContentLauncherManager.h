/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/af-structs.h>

#include <app/AttributeAccessInterface.h>
#include <app/clusters/content-launch-server/content-launch-server.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <list>

class ContentLauncherManager
{
public:
    void InitializeWithObjects(jobject managerObject);
    CHIP_ERROR GetAcceptsHeader(chip::app::AttributeValueEncoder & aEncoder);
    CHIP_ERROR GetSupportedStreamingTypes(chip::app::AttributeValueEncoder & aEncoder);
    ContentLaunchResponse LaunchContent(std::list<ContentLaunchParamater> parameterList, bool autoplay,
                                        const chip::CharSpan & data);
    ContentLaunchResponse LaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                                    ContentLaunchBrandingInformation & brandingInformation);

private:
    friend ContentLauncherManager & ContentLauncherMgr();

    static ContentLauncherManager sInstance;
    jobject mContentLauncherManagerObject       = nullptr;
    jmethodID mGetAcceptsHeaderMethod           = nullptr;
    jmethodID mGetSupportedStreamingTypesMethod = nullptr;
    jmethodID mLaunchContentMethod              = nullptr;
    jmethodID mLaunchUrlMethod                  = nullptr;
};

inline ContentLauncherManager & ContentLauncherMgr()
{
    return ContentLauncherManager::sInstance;
}
