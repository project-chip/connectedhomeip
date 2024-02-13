/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/clusters/keypad-input-server/keypad-input-server.h>

class KeypadInputManager : public chip::app::Clusters::KeypadInput::Delegate
{
    using SendKeyResponseType = chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::Type;
    using CecKeyCodeType      = chip::app::Clusters::KeypadInput::CECKeyCodeEnum;
    using Feature             = chip::app::Clusters::KeypadInput::Feature;

public:
    void HandleSendKey(chip::app::CommandResponseHelper<SendKeyResponseType> & helper, const CecKeyCodeType & keyCode) override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

private:
    static constexpr uint32_t mDynamicEndpointFeatureMap =
        chip::BitMask<Feature, uint32_t>(Feature::kNavigationKeyCodes, Feature::kLocationKeys, Feature::kNumberKeys).Raw();
};
