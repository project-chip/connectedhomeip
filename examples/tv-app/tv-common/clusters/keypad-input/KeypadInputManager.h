/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/keypad-input-server/keypad-input-server.h>

using chip::app::CommandResponseHelper;
using KeypadInputDelegate = chip::app::Clusters::KeypadInput::Delegate;
using SendKeyResponseType = chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::Type;
using CecKeyCodeType      = chip::app::Clusters::KeypadInput::CecKeyCode;

class KeypadInputManager : public KeypadInputDelegate
{
public:
    void HandleSendKey(CommandResponseHelper<SendKeyResponseType> & helper, const CecKeyCodeType & keyCode) override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

private:
    // TODO: set this based upon meta data from app
    uint32_t mDynamicEndpointFeatureMap = 7;
};
