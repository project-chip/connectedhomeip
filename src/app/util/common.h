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
/***************************************************************************/
/**
 * @file
 * @brief This file contains the includes that are
 *common to all clusters in the util.
 *******************************************************************************
 ******************************************************************************/

#ifndef SILABS_COMMON_H
#define SILABS_COMMON_H

// App framework
#include "af.h"
#include "attribute-storage.h"
#include "attribute-table.h"
#include "time-util.h"
#include "util.h"

// the variables used to setup and send responses to cluster messages
extern EmberApsFrame emberAfResponseApsFrame;
extern uint8_t appResponseData[EMBER_AF_RESPONSE_BUFFER_LEN];
extern uint16_t appResponseLength;
extern ChipResponseDestination * emberAfResponseDestination;

#endif // SILABS_COMMON_H
