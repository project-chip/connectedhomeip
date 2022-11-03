/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ApplicationLauncher.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR LaunchAppCommand::Invoke(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                    chip::Optional<chip::ByteSpan> data, std::function<void(CHIP_ERROR)> responseCallback)
{
    ApplicationLauncher::Commands::LaunchApp::Type request;
    request.application = application;
    request.data        = data;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR StopAppCommand::Invoke(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                  std::function<void(CHIP_ERROR)> responseCallback)
{
    ApplicationLauncher::Commands::StopApp::Type request;
    request.application = application;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR HideAppCommand::Invoke(chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application,
                                  std::function<void(CHIP_ERROR)> responseCallback)
{
    ApplicationLauncher::Commands::HideApp::Type request;
    request.application = application;
    return MediaCommandBase::Invoke(request, responseCallback);
}
