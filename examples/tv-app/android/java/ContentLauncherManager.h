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

class ContentLauncherManager : public chip::app::Clusters::ContentLauncher::Delegate
{
public:
    static void NewManager(jint endpoint, jobject manager);
    void InitializeWithObjects(jobject managerObject);

    void
    HandleLaunchContent(const std::list<Parameter> & parameterList, bool autoplay, const chip::CharSpan & data,
                        chip::app::CommandResponseHelper<chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::Type> &
                            responser) override;
    void HandleLaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                         const std::list<BrandingInformation> & brandingInformation,
                         chip::app::CommandResponseHelper<chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::Type> &
                             responser) override;
    CHIP_ERROR HandleGetAcceptHeaderList(chip::app::AttributeValueEncoder & aEncoder) override;
    uint32_t HandleGetSupportedStreamingProtocols() override;

private:
    jobject mContentLauncherManagerObject           = nullptr;
    jmethodID mGetAcceptHeaderMethod                = nullptr;
    jmethodID mGetSupportedStreamingProtocolsMethod = nullptr;
    jmethodID mLaunchContentMethod                  = nullptr;
    jmethodID mLaunchUrlMethod                      = nullptr;
};
