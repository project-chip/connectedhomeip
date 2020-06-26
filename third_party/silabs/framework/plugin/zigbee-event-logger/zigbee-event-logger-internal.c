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
 * @brief Internal routines for the Zigbee Event Logger plugin.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "zigbee-event-logger-internal.h"

//-----------------------------------------------------------------------------
// Internal API

uint8_t* addUint32_t(uint8_t* data, uint32_t itemToAdd)
{
  data = addUint16_t(data, (uint16_t)itemToAdd);
  data = addUint16_t(data, (uint16_t)(itemToAdd >> 16));
  return data;
}

uint8_t* addUint16_t(uint8_t* data, uint16_t itemToAdd)
{
  data = addUint8_t(data, (uint8_t)itemToAdd);
  data = addUint8_t(data, (uint8_t)(itemToAdd >> 8));
  return data;
}

uint8_t* addUint8_t(uint8_t* data, uint8_t itemToAdd)
{
  *data = itemToAdd;
  data++;
  return data;
}

uint8_t* addArray(uint8_t* data, uint8_t *itemToAdd, uint8_t itemToAddLength)
{
  while ( itemToAddLength > 0 ) {
    *data = *itemToAdd++;
    data++;
    itemToAddLength--;
  }
  return data;
}

uint32_t parseUint32_t(uint8_t** data)
{
  uint32_t value = parseUint16_t(data);
  value += (parseUint16_t(data) << 16);
  return value;
}

uint16_t parseUint16_t(uint8_t** data)
{
  uint16_t value = parseUint8_t(data);
  value += (parseUint8_t(data) << 8);
  return value;
}

uint8_t parseUint8_t(uint8_t** data)
{
  uint8_t value = **data;
  // NOTE:  "*data = *data+1" is not the same as "*data++".
  *data = *data + 1;
  return value;
}

void parseArray(uint8_t *value, uint8_t** data, uint8_t dataLength)
{
  while ( dataLength > 0 ) {
    *value++ = **data;
    *data = *data + 1;
    dataLength--;
  }
}
