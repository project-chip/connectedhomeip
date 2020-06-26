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
 * @brief Zigbee Over-the-air bootload cluster for upgrading firmware and
 * downloading device specific file.
 * This file handles the page request feature for the client.
 *******************************************************************************
   ******************************************************************************/

enum {
  EM_AF_NO_PAGE_REQUEST              = 0,
  EM_AF_WAITING_PAGE_REQUEST_REPLIES = 1,
  EM_AF_RETRY_MISSED_PACKETS         = 2,
  EM_AF_PAGE_REQUEST_COMPLETE        = 3,
  EM_AF_BLOCK_ALREADY_RECEIVED       = 4,
  EM_AF_PAGE_REQUEST_ERROR           = 0xFF
};
typedef uint8_t EmAfPageRequestClientStatus;

#define EM_AF_PAGE_REQUEST_BLOCK_SIZE 32

// This routine returns a timer indicating how long we should wait for
// the page request responses to come in.  0 if there was an error.
uint32_t emAfInitPageRequestClient(uint32_t offsetForPageRequest,
                                   uint32_t totalImageSize);
void emAfPageRequestTimerExpired(void);
bool emAfHandlingPageRequestClient(void);
EmAfPageRequestClientStatus emAfGetCurrentPageRequestStatus(void);
EmAfPageRequestClientStatus emAfNoteReceivedBlockForPageRequestClient(uint32_t offset);
EmAfPageRequestClientStatus emAfNextMissedBlockRequestOffset(uint32_t* nextOffset);

uint32_t emAfGetPageRequestMissedPacketDelayMs(void);
uint32_t emAfGetFinishedPageRequestOffset(void);
void emAfAbortPageRequest(void);
