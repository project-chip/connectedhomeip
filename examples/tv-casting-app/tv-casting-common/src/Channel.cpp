/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Channel.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR ChangeChannelCommand::Invoke(const CharSpan & match, std::function<void(CHIP_ERROR)> responseCallback)
{
    Channel::Commands::ChangeChannel::Type request;
    request.match = match;
    return MediaCommandBase::Invoke(request, responseCallback);
}
