/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/keypad-input-server/keypad-input-server.h>
#include <jni.h>

using chip::app::CommandResponseHelper;
using KeypadInputDelegate = chip::app::Clusters::KeypadInput::Delegate;
using SendKeyResponseType = chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::Type;

class KeypadInputManager : public KeypadInputDelegate
{
public:
    static void NewManager(jint endpoint, jobject manager);
    void InitializeWithObjects(jobject managerObject);

    void HandleSendKey(CommandResponseHelper<SendKeyResponseType> & helper,
                       const chip::app::Clusters::KeypadInput::CecKeyCode & keyCode) override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

private:
    jobject mKeypadInputManagerObject = nullptr;
    jmethodID mSendKeyMethod          = nullptr;

    // TODO: set this based upon meta data from app
    uint32_t mDynamicEndpointFeatureMap = 7;
};
