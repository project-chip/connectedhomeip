/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "KeypadInput.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR SendKeyCommand::Invoke(const chip::app::Clusters::KeypadInput::CecKeyCode keyCode,
                                  std::function<void(CHIP_ERROR)> responseCallback)
{
    KeypadInput::Commands::SendKey::Type request;
    request.keyCode = keyCode;
    return MediaCommandBase::Invoke(request, responseCallback);
}
