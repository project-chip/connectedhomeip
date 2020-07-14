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

#include <lib/datamodel/ClusterServer.h>
namespace chip {
namespace System {
class PacketBuffer;
} // namespace System
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
 * @param [in] server The ClusterServer object that holds the data model
 * @param [in] buffer The buffer holding the message.  This function guarantees
 *                    that it will free the buffer before returning.
 */
void HandleDataModelMessage(chip::DataModel::ClusterServer & server, chip::System::PacketBuffer * buffer);

#endif // DATA_MODEL_HANDLER_H
