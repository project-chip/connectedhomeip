/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "OnOff.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR OnCommand::Invoke(std::function<void(CHIP_ERROR)> responseCallback)
{
    OnOff::Commands::On::Type request;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR OffCommand::Invoke(std::function<void(CHIP_ERROR)> responseCallback)
{
    OnOff::Commands::Off::Type request;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR ToggleCommand::Invoke(std::function<void(CHIP_ERROR)> responseCallback)
{
    OnOff::Commands::Toggle::Type request;
    return MediaCommandBase::Invoke(request, responseCallback);
}
