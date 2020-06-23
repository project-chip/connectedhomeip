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
 * @brief State machine for sending data via xmodem.  This code is agnostic to the
 * data being sent or the actual transport mechanism.  It has callbacks to
 * send the Xmodem formatted data over-the-air or via serial.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "xmodem-sender.h"

//------------------------------------------------------------------------------
// Globals

typedef enum {
  XMODEM_SEND_STATE_NONE,
  XMODEM_SEND_STATE_WAITING_READY,
  XMODEM_SEND_STATE_WAITING_DATA_ACK,
  XMODEM_SEND_STATE_WAITING_EOT_ACK,
} XmodemSendState;

static XmodemSendState xmodemSendState = XMODEM_SEND_STATE_NONE;
static EmberAfXmodemSenderTransmitFunction* sendFunction;
static EmberAfXmodemSenderGetNextBlockFunction* getNextBlockFunction;
static EmberAfXmodemSenderFinishedFunction* finishedFunction;
static uint32_t currentOffset;
static uint8_t blockNum;
static uint8_t retries;

// Normally Xmodem always operates on 128 byte packets.  However
// to send Xmodem over-the-air we are limited by a 128-byte MTU
// in 802.15.4.  Therefore we allow the sender to use a smaller
// block size to compensate.  The receiver must understand
// this ahead of time as this is non-standard.
static uint8_t maxBlockSize;

// It is necessary to have both this variable and the XmodemSendState
// indicating whether we are waiting for a data ACK or an EOT ACK.
// In the case where we are retrying the final block, we must
// keep track of the fact that upon successful ACK we move on to
// the EOT state.
static bool lastBlock;

EmberEventControl emberAfPluginXmodemSenderMyEventEventControl;

#define ACK_TIMEOUT_SECONDS 2
#define READY_TIMEOUT_SECONDS 10

#define xmodemPrint(...)    emberAfCorePrint(__VA_ARGS__)
#define xmodemPrintln(...)  emberAfCorePrintln(__VA_ARGS__)

#define XMODEM_MAX_DATA_SIZE 128

// Overhead
// 1 byte control
// 1 byte Block Num
// 1 byte One's complement of Block num
// (data)
// 2 byte CRC (at end)
#define XMODEM_OVERHEAD 5

#define CONTROL_OFFSET      (0)
#define BLOCK_NUM_OFFSET    (1)
#define BLOCK_COMP_OFFSET   (2)
#define DATA_OFFSET         (3)

#define MAX_RETRIES 10

#define ACK_LENGTH 2

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------

static void resetState(bool success)
{
  (*finishedFunction)(success);
  xmodemSendState = XMODEM_SEND_STATE_NONE;
}

static EmberStatus sendNextBlock(bool success)
{
  uint8_t returnLength;
  EmberStatus status;
  uint8_t dataBuffer[XMODEM_MAX_DATA_SIZE + XMODEM_OVERHEAD];
  bool sendEot = (xmodemSendState == XMODEM_SEND_STATE_WAITING_EOT_ACK);

  if (!success) {
    retries--;
    if (retries == 0) {
      xmodemPrintln("Error: Max retry count hit (%d), giving up.", MAX_RETRIES);
      resetState(false);
      return EMBER_ERR_FATAL;
    }
    xmodemPrintln("Error: Block Number %d, Retry count: %d", blockNum, MAX_RETRIES - retries);
  } else {
    retries = MAX_RETRIES;
  }

  MEMSET(dataBuffer, 0xFF, XMODEM_MAX_DATA_SIZE + XMODEM_OVERHEAD);

  if (!sendEot) {
    status = (*getNextBlockFunction)(currentOffset,
                                     maxBlockSize,
                                     &dataBuffer[DATA_OFFSET],
                                     &returnLength,
                                     &lastBlock);
    if (status != EMBER_SUCCESS) {
      xmodemPrintln("Error: Failed to retreive data for offset 0x%4X, size: %d.  Aborting",
                    currentOffset,
                    returnLength);
      resetState(false);
      return status;
    }
  }

  if (sendEot) {
    dataBuffer[CONTROL_OFFSET] = XMODEM_EOT;
    returnLength = 1;
  } else {
    uint8_t i;
    uint16_t crc = 0;
    returnLength = XMODEM_OVERHEAD + maxBlockSize;
    dataBuffer[CONTROL_OFFSET] = XMODEM_SOH;
    dataBuffer[BLOCK_NUM_OFFSET] = blockNum;
    dataBuffer[BLOCK_COMP_OFFSET] = ~blockNum;

    for (i = 0; i < maxBlockSize; i++) {
      crc = halCommonCrc16(dataBuffer[i + DATA_OFFSET], crc);
    }
    dataBuffer[maxBlockSize + XMODEM_OVERHEAD - 2] = HIGH_BYTE(crc);
    dataBuffer[maxBlockSize + XMODEM_OVERHEAD - 1] = LOW_BYTE(crc);
  }

  status = (*sendFunction)(dataBuffer, returnLength);
  if (status != EMBER_SUCCESS) {
    xmodemPrint("Error:  Failed to send ");
    xmodemPrintln((sendEot
                   ? "EOT"
                   : "block %d, aborting."),
                  blockNum);
    resetState(false);
  }

  emberEventControlSetDelayQS(emberAfPluginXmodemSenderMyEventEventControl,
                              ACK_TIMEOUT_SECONDS << 2);
  xmodemSendState = (sendEot
                     ? XMODEM_SEND_STATE_WAITING_EOT_ACK
                     : XMODEM_SEND_STATE_WAITING_DATA_ACK);

  return status;
}

void emberAfPluginXmodemSenderIncomingBlock(uint8_t* data,
                                            uint8_t  length)
{
  bool success = true;

  switch (xmodemSendState) {
    case XMODEM_SEND_STATE_WAITING_DATA_ACK:
    case XMODEM_SEND_STATE_WAITING_EOT_ACK: {
      if (data[CONTROL_OFFSET] == XMODEM_ACK) {
        if (xmodemSendState == XMODEM_SEND_STATE_WAITING_DATA_ACK) {
          if (length < ACK_LENGTH) {
            xmodemPrintln("Error: Response too short (%d < %d).  Aborting.", length, ACK_LENGTH);
            resetState(false);
            return;
          }
          if (data[BLOCK_NUM_OFFSET] != blockNum) {
            xmodemPrintln("Warning: Got ack for wrong packet.  Expected: %d, Actual: %d.  Ignoring.",
                          blockNum,
                          data[BLOCK_NUM_OFFSET]);
            return;
          }
        }

        if (xmodemSendState == XMODEM_SEND_STATE_WAITING_EOT_ACK) {
          // Done
          resetState(true);
          return;;
        }

        if (lastBlock) {
          xmodemSendState = XMODEM_SEND_STATE_WAITING_EOT_ACK;
        } else {
          blockNum++;
          currentOffset += maxBlockSize;
        }
      } else if (data[CONTROL_OFFSET] == XMODEM_NAK) {
        success = false;
      } else if (data[CONTROL_OFFSET] == XMODEM_CANCEL) {
        xmodemPrintln("Received CANCEL.  Aborting.");
        resetState(false);
        return;
      } else {
        xmodemPrintln("Error: Unrecognized xmodem packet received: 0x%X, Ignoring.", data[CONTROL_OFFSET]);
        return;
      }

      sendNextBlock(success);
      break;
    }
    case XMODEM_SEND_STATE_WAITING_READY: {
      if (data[CONTROL_OFFSET] == 'C') {
        sendNextBlock(true);
      }
      // Else, ignore.
      break;
    }
    // Adding a default case makes compilers happier.
    default:
      break;
  }
}

void emberAfPluginXmodemSenderMyEventEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginXmodemSenderMyEventEventControl);

  switch (xmodemSendState) {
    case XMODEM_SEND_STATE_WAITING_DATA_ACK:
    case XMODEM_SEND_STATE_WAITING_EOT_ACK:
    // Fall through

    case XMODEM_SEND_STATE_WAITING_READY: {
      if (xmodemSendState == XMODEM_SEND_STATE_WAITING_READY) {
        xmodemPrintln("Error: Timeout waiting for ready character.  Giving up.");
        resetState(false);
        return;
      }

      sendNextBlock(false);
      break;
    }
    default:
      break;
  }
  ;
}

EmberStatus emberAfPluginXmodemSenderStart(EmberAfXmodemSenderTransmitFunction* sendRoutine,
                                           EmberAfXmodemSenderGetNextBlockFunction* getNextBlockRoutine,
                                           EmberAfXmodemSenderFinishedFunction* finishedRoutine,
                                           uint8_t maxSizeOfBlock,
                                           bool waitForReady)

{
  if (xmodemSendState != XMODEM_SEND_STATE_NONE
      || maxSizeOfBlock > XMODEM_MAX_DATA_SIZE) {
    return EMBER_INVALID_CALL;
  }

  sendFunction = sendRoutine;
  getNextBlockFunction = getNextBlockRoutine;
  finishedFunction = finishedRoutine;
  currentOffset = 0;
  // Per the Xmodem spec, this starts at 1 goes to 255, and wraps back to 0.
  blockNum = 1;
  lastBlock = false;
  maxBlockSize = maxSizeOfBlock;

  if (waitForReady) {
    emberEventControlSetDelayQS(emberAfPluginXmodemSenderMyEventEventControl,
                                READY_TIMEOUT_SECONDS << 2);
    xmodemSendState = XMODEM_SEND_STATE_WAITING_READY;
    return EMBER_SUCCESS;
  }

  return sendNextBlock(true);
}

void emberAfPluginXmodemSenderAbort(void)
{
  resetState(false);
}
