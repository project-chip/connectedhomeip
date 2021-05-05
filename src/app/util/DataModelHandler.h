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

/**
 * @file
 *   This file defines the API for the handler for data model messages.
 */

#pragma once

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

/**
 * Initialize the data model internal code to be ready to send and receive
 * data model messages.
 *
 */
void InitDataModelHandler(chip::Messaging::ExchangeManager * exchangeMgr);

/**
 * Handle a message that should be processed via our data model processing
 * codepath.
 *
 * @param [in] exchange The exchange on which the message was received.
 * @param [in] buffer The buffer holding the message.  This function guarantees
 *                    that it will free the buffer before returning.
 *
 */
void HandleDataModelMessage(chip::Messaging::ExchangeContext * exchange, chip::System::PacketBufferHandle buffer);
