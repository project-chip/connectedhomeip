/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MediaCommandBase.h"
#include "MediaSubscriptionBase.h"

#include <controller/CHIPCluster.h>
#include <functional>

#include <app-common/zap-generated/cluster-objects.h>

// COMMAND CLASSES
class LaunchAppCommand
    : public MediaCommandBase<chip::app::Clusters::ApplicationLauncher::Commands::LaunchApp::Type,
                              chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType>
{
public:
    LaunchAppCommand() : MediaCommandBase(chip::app::Clusters::ApplicationLauncher::Id) {}

    CHIP_ERROR Invoke(chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                      chip::Optional<chip::ByteSpan> data, std::function<void(CHIP_ERROR)> responseCallback);
};

class StopAppCommand : public MediaCommandBase<chip::app::Clusters::ApplicationLauncher::Commands::StopApp::Type,
                                               chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType>
{
public:
    StopAppCommand() : MediaCommandBase(chip::app::Clusters::ApplicationLauncher::Id) {}

    CHIP_ERROR Invoke(chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                      std::function<void(CHIP_ERROR)> responseCallback);
};

class HideAppCommand : public MediaCommandBase<chip::app::Clusters::ApplicationLauncher::Commands::HideApp::Type,
                                               chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType>
{
public:
    HideAppCommand() : MediaCommandBase(chip::app::Clusters::ApplicationLauncher::Id) {}

    CHIP_ERROR Invoke(chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application,
                      std::function<void(CHIP_ERROR)> responseCallback);
};

// SUBSCRIBER CLASSES
class CurrentAppSubscriber
    : public MediaSubscriptionBase<chip::app::Clusters::ApplicationLauncher::Attributes::CurrentApp::TypeInfo>
{
public:
    CurrentAppSubscriber() : MediaSubscriptionBase(chip::app::Clusters::ApplicationLauncher::Id) {}
};
