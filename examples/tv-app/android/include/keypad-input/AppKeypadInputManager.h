/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include "../../java/ContentAppCommandDelegate.h"
#include <app/clusters/keypad-input-server/keypad-input-server.h>
#include <lib/core/DataModelTypes.h>

/// Keypad Input delegate for content app endpoints.
///
/// Forwards SendKey to the content app through ContentAppCommandDelegate (the Java
/// ContentAppEndpointManager.sendCommand path) and returns the status reported by the app.
class AppKeypadInputManager : public chip::app::Clusters::KeypadInput::Delegate
{
public:
    using SendKeyResponseType = chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::Type;

    explicit AppKeypadInputManager(chip::AppPlatform::ContentAppCommandDelegate * commandDelegate) :
        mCommandDelegate(commandDelegate)
    {}

    void SetEndpointId(chip::EndpointId endpointId) { mEndpointId = endpointId; }

    void HandleSendKey(chip::app::CommandResponseHelper<SendKeyResponseType> & helper,
                       const chip::app::Clusters::KeypadInput::CECKeyCodeEnum & keyCode) override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override { return kEndpointFeatureMap; }

private:
    chip::AppPlatform::ContentAppCommandDelegate * mCommandDelegate;
    chip::EndpointId mEndpointId = chip::kInvalidEndpointId;

    // TODO: set this based upon meta data from app
    static constexpr uint32_t kEndpointFeatureMap = 7;
};
