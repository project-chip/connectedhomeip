/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "TargetNavigator.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR NavigateTargetCommand::Invoke(const uint8_t target, const Optional<chip::CharSpan> data,
                                         std::function<void(CHIP_ERROR)> responseCallback)
{
    TargetNavigator::Commands::NavigateTarget::Type request;
    request.target = target;
    request.data   = data;
    return MediaCommandBase::Invoke(request, responseCallback);
}
