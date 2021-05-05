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

#include "KeypadInputManager.h"

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

#include <app/util/af.h>
#include <app/util/basic-types.h>

#include <map>
#include <string>

using namespace std;

CHIP_ERROR KeypadInputManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO: Store feature map once it is supported
    map<string, bool> featureMap;
    featureMap["NV"] = true;
    featureMap["LK"] = true;
    featureMap["NK"] = true;

    SuccessOrExit(err);
exit:
    return err;
}

EmberAfKeypadInputStatus KeypadInputManager::proxyKeypadInputRequest(EmberAfKeypadInputCecKeyCode keyCode)
{
    // TODO: Insert code here
    return EMBER_ZCL_KEYPAD_INPUT_STATUS_SUCCESS;
}

static void sendResponse(const char * responseName, uint8_t keypadInputStatus)
{
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_KEYPAD_INPUT_CLUSTER_ID,
                              ZCL_SEND_KEY_RESPONSE_COMMAND_ID, "u", keypadInputStatus);

    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        emberAfKeypadInputClusterPrintln("Failed to send %s: 0x%X", responseName, status);
    }
}

bool emberAfKeypadInputClusterSendKeyCallback(EmberAfKeypadInputCecKeyCode keyCode)
{
    EmberAfKeypadInputStatus status = KeypadInputManager().proxyKeypadInputRequest(keyCode);
    sendResponse("KeypadInputResponse", status);
    return true;
}
