/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/application-launcher-server/application-launcher-server.h>
#include <list>

using chip::ByteSpan;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ApplicationLauncherDelegate = chip::app::Clusters::ApplicationLauncher::Delegate;
using ApplicationType             = chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type;
using LauncherResponseType        = chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::Type;

class ApplicationLauncherManager : public ApplicationLauncherDelegate
{
public:
    ApplicationLauncherManager() : ApplicationLauncherDelegate(){};
    ApplicationLauncherManager(bool featureMapContentPlatform) : ApplicationLauncherDelegate(featureMapContentPlatform){};

    CHIP_ERROR HandleGetCatalogList(AttributeValueEncoder & aEncoder) override;

    void HandleLaunchApp(CommandResponseHelper<LauncherResponseType> & helper, const ByteSpan & data,
                         const ApplicationType & application) override;
    void HandleStopApp(CommandResponseHelper<LauncherResponseType> & helper, const ApplicationType & application) override;
    void HandleHideApp(CommandResponseHelper<LauncherResponseType> & helper, const ApplicationType & application) override;
};
