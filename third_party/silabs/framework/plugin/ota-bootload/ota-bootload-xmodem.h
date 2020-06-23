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
 * @brief Routines for sending data via xmodem
 *******************************************************************************
   ******************************************************************************/

// Initialize xmodem state machine for a new transfer
// If startImmediately is set, will not wait for an initial 'C' character
//   to be received before sending the first block of data
void emAfInitXmodemState(bool startImmediately);

// Send a chunk of data via xmodem.  Arbitrary lengths of data may be passed,
//   it will be broken up into appropriate sized chunks for transmission. Xmodem
//   itself transfers data in 128 byte chunks
// Note: This function will block for the duration of time required to send
//   the data that is passed in.
// The "finished" flag should be set when called with the final chunk to be
//   transferred to terminate the transfer properly
bool emAfSendXmodemData(const uint8_t *data, uint16_t length, bool finished);
