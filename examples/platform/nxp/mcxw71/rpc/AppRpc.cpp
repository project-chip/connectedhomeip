/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "AppRpc.h"
#include "ButtonApp.h"
#include "ButtonBle.h"
#include "ButtonManager.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "Rpc.h"

// These instances do not need to be initialized, since the RPC
// code only simulates the press of a button.
static chip::NXP::App::ButtonApp sAppButton;
static chip::NXP::App::ButtonBle sBleButton;

CHIP_ERROR chip::NXP::App::Rpc::Init()
{
    /* set clock */
    CLOCK_SetIpSrc(kCLOCK_Lpuart1, kCLOCK_IpSrcFro192M);
    /* enable clock */
    CLOCK_EnableClock(kCLOCK_Lpuart1);

    BOARD_InitPinLPUART1_TX();
    BOARD_InitPinLPUART1_RX();
    chip::rpc::Init();

    return CHIP_NO_ERROR;
}

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
void chip::NXP::App::Rpc::Reboot()
{
    NVIC_SystemReset();
}
#endif

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
void chip::NXP::App::Rpc::ButtonHandler(const chip_rpc_ButtonEvent & request)
{
    button_callback_message_t * message = new button_callback_message_t;

    switch (request.idx)
    {
    case 0:
        message->event = kBUTTON_EventShortPress;
        ButtonManager::HandleCallbacks(nullptr, message, &sBleButton);
        break;
    case 1:
        message->event = kBUTTON_EventLongPress;
        ButtonManager::HandleCallbacks(nullptr, message, &sBleButton);
        break;
    case 2:
        message->event = kBUTTON_EventShortPress;
        ButtonManager::HandleCallbacks(nullptr, message, &sAppButton);
        break;
    case 3:
        message->event = kBUTTON_EventLongPress;
        ButtonManager::HandleCallbacks(nullptr, message, &sAppButton);
        break;
    default:
        break;
    }

    delete message;
}
#endif
