/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/AttributeAccessInterface.h>
#include <app/clusters/content-launch-server/content-launch-server.h>
#include <jni.h>
#include <lib/core/CHIPError.h>

using chip::CharSpan;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ContentLauncherDelegate = chip::app::Clusters::ContentLauncher::Delegate;
using LaunchResponseType      = chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::Type;
using ParameterType           = chip::app::Clusters::ContentLauncher::Structs::ParameterStruct::DecodableType;
using BrandingInformationType = chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type;

class ContentLauncherManager : public ContentLauncherDelegate
{
public:
    static void NewManager(jint endpoint, jobject manager);
    void InitializeWithObjects(jobject managerObject);

    void HandleLaunchContent(CommandResponseHelper<LaunchResponseType> & helper,
                             const chip::app::DataModel::DecodableList<ParameterType> & parameterList, bool autoplay,
                             const CharSpan & data) override;
    void HandleLaunchUrl(CommandResponseHelper<LaunchResponseType> & helper, const CharSpan & contentUrl,
                         const CharSpan & displayString, const BrandingInformationType & brandingInformation) override;
    CHIP_ERROR HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder) override;
    uint32_t HandleGetSupportedStreamingProtocols() override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

private:
    jobject mContentLauncherManagerObject           = nullptr;
    jmethodID mGetAcceptHeaderMethod                = nullptr;
    jmethodID mGetSupportedStreamingProtocolsMethod = nullptr;
    jmethodID mLaunchContentMethod                  = nullptr;
    jmethodID mLaunchUrlMethod                      = nullptr;
};
