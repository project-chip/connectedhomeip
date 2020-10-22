/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef _RENDEZVOUS_MESSAGE_HANDLER_H
#define _RENDEZVOUS_MESSAGE_HANDLER_H

#include <core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;

class RendezvousMessageHandler
{
public:
    // Handle a rendezvous message. Returns:
    // - CHIP_NO_ERROR if the message was handled successfully.
    // - CHIP_ERROR_INVALID_MESSAGE_TYPE if the message was not recognized.
    // - Some other error encountered processing a specific message type.
    static CHIP_ERROR HandleMessageReceived(System::PacketBuffer * buffer);
};

#endif // _RENDEZVOUS_MESSAGE_HANDLER_H
