/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MediaCommandBase.h"

#include <functional>
#include <zap-generated/CHIPClusters.h>

class SendKeyCommand : public MediaCommandBase<chip::app::Clusters::KeypadInput::Commands::SendKey::Type,
                                               chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::DecodableType>
{
public:
    SendKeyCommand() : MediaCommandBase(chip::app::Clusters::KeypadInput::Id) {}

    CHIP_ERROR Invoke(const chip::app::Clusters::KeypadInput::CecKeyCode keyCode, std::function<void(CHIP_ERROR)> responseCallback);
};
