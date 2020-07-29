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

#ifndef DATA_MODEL_HANDLER_H
#define DATA_MODEL_HANDLER_H

namespace chip {
namespace System {
class PacketBuffer;
} // namespace System

class SecureSessionMgrBase;
class MessageHeader;
} // namespace chip

/**
 * Initialize the data model handler.  This must be called once, and before any
 * HandleDataModelMessage calls happen.
 */
void InitDataModelHandler();

/**
 * Handle a message that should be processed via our data model processing
 * codepath.
 *
 * @param[in] header The message header for the incoming message.
 * @param[in] buffer The buffer holding the message.  This function guarantees
 *                   that it will free the buffer before returning.
 * @param[in] mgr The session manager to use for sending a response to the
 *                message.
 */
void HandleDataModelMessage(const chip::MessageHeader & header, chip::System::PacketBuffer * buffer,
                            chip::SecureSessionMgrBase * mgr);

#endif // DATA_MODEL_HANDLER_H
