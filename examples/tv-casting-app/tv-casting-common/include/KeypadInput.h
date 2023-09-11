/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MediaCommandBase.h"

#include <controller/CHIPCluster.h>
#include <functional>

#include <app-common/zap-generated/cluster-objects.h>

class SendKeyCommand : public MediaCommandBase<chip::app::Clusters::KeypadInput::Commands::SendKey::Type,
                                               chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::DecodableType>
{
public:
    SendKeyCommand() : MediaCommandBase(chip::app::Clusters::KeypadInput::Id) {}

    CHIP_ERROR Invoke(const chip::app::Clusters::KeypadInput::CecKeyCode keyCode, std::function<void(CHIP_ERROR)> responseCallback);
};
