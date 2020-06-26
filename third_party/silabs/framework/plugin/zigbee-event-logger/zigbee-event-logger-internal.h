/**
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
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Internal definitions for the Zigbee Event Logger plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef _ZIGBEE_EVENT_LOGGER_INTERNAL_H_
#define _ZIGBEE_EVENT_LOGGER_INTERNAL_H_

uint8_t* addUint8_t(uint8_t* data, uint8_t itemToAdd);
uint8_t* addUint16_t(uint8_t* data, uint16_t itemToAdd);
uint8_t* addUint32_t(uint8_t* data, uint32_t itemToAdd);
uint8_t* addArray(uint8_t* data, uint8_t *itemToAdd, uint8_t itemToAddLength);

uint8_t  parseUint8_t(uint8_t** data);
uint16_t parseUint16_t(uint8_t** data);
uint32_t parseUint32_t(uint8_t** data);
void parseArray(uint8_t *value, uint8_t** data, uint8_t dataLength);

#endif  // #ifndef _ZIGBEE_EVENT_LOGGER_INTERNAL_H_
