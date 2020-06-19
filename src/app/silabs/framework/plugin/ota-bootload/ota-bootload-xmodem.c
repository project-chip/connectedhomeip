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

#include "app/framework/include/af.h"
#include "app/framework/plugin/ota-bootload/ota-bootload-ncp.h"

#ifdef HAL_HOST
  #include "hal/host/crc.h"
#else //HAL_HOST
  #include "hal/micro/crc.h"
#endif //HAL_HOST

//------------------------------------------------------------------------------

//#define DEBUG_PRINT_ON
#if defined DEBUG_PRINT_ON
  #define DBGPRINT(...) emberAfOtaBootloadClusterPrintln(__VA_ARGS__)
#else
  #define DBGPRINT(...)
#endif

#define SOH   (0x01)
#define EOT   (0x04)
#define ACK   (0x06)
#define NAK   (0x15)
#define CAN   (0x18)

#define DATA_SIZE        (128)
#define HEADER_SIZE      (3)
#define FOOTER_SIZE      (2)
#define FULL_BLOCK_SIZE  (HEADER_SIZE + DATA_SIZE + FOOTER_SIZE)

#define CONTROL_OFFSET      (0)
#define BLOCK_NUM_OFFSET    (1)
#define BLOCK_COMP_OFFSET   (2)
#define DATA_OFFSET         (3)
#define CRC_H_OFFSET        (131)
#define CRC_L_OFFSET        (132)

#define UNINITIALIZED       (0)
#define START_TRANSMISSION  (1)
#define SENDING             (2)

static uint8_t state = UNINITIALIZED;
static uint8_t buffFinger;
static uint8_t dataBuff[DATA_SIZE];
static uint8_t blockNum;

//------------------------------------------------------------------------------

static bool sendBlock(uint8_t blockNum, const uint8_t *data)
{
  int i;
  int retry = 5;
  uint16_t crc = 0;
  uint8_t status = NAK;
  uint8_t fullBlock[FULL_BLOCK_SIZE];

  fullBlock[CONTROL_OFFSET] = SOH;
  fullBlock[BLOCK_NUM_OFFSET] = blockNum;
  fullBlock[BLOCK_COMP_OFFSET] = ~blockNum;
  for (i = 0; i < DATA_SIZE; i++) {
    crc = halCommonCrc16(data[i], crc);
    fullBlock[DATA_OFFSET + i] = data[i];
  }
  fullBlock[CRC_H_OFFSET] = HIGH_BYTE(crc);
  fullBlock[CRC_L_OFFSET] = LOW_BYTE(crc);

  while ( (status == NAK) && (retry > 0) ) {
    //print "block %d (" % num,
    //for i in range(0,len(data)):
    //  print "%02x" % ord(data[i]),
    if (!emAfBootloadSendData(fullBlock, FULL_BLOCK_SIZE)) {
      DBGPRINT("sendBlock() fail 1");
      return false;
    }
    retry--;
    if (!emAfBootloadWaitChar(&status, false, 0)) {
      DBGPRINT("sendBlock() fail 2");
      return false;
    }
    while ( status == 'C' ) {
      // may have leftover C characters from start of transmission
      if (!emAfBootloadWaitChar(&status, false, 0)) {
        DBGPRINT("sendBlock() fail 3");
        return false;
      }
    }
  }

  return (status == ACK);
}

void emAfInitXmodemState(bool startImmediately)
{
  if (startImmediately) {
    // skip checking for 'C' characters
    state = SENDING;
  } else {
    state = START_TRANSMISSION;
  }

  buffFinger = 0;
  blockNum = 1;
}

bool emAfSendXmodemData(const uint8_t *data, uint16_t length, bool finished)
{
  uint8_t rxData;
  uint16_t i;

  if (state == START_TRANSMISSION) {
    if (emAfBootloadWaitChar(&rxData, true, 'C')) {
      DBGPRINT("sending\n");
      state = SENDING;
    } else {
      DBGPRINT("NoC\n");
      return false;
    }
  }

  if (state == SENDING) {
    for (i = 0; i < length; i++) {
      dataBuff[buffFinger++] = data[i];
      if (buffFinger >= DATA_SIZE) {
        DBGPRINT("block %d\n", blockNum);
        emberAfCoreFlush();
        if (!sendBlock(blockNum, dataBuff)) {
          DBGPRINT("sendblock err\n");
          emberAfCoreFlush();
          return false;
        }
        buffFinger = 0;
        blockNum++;
      }
    }
    if ( finished ) {
      if ( buffFinger != 0 ) {
        // pad and send final block
        bool result;
        while (buffFinger < DATA_SIZE) {
          dataBuff[buffFinger++] = 0xFF;
        }
        DBGPRINT("final block %d\n", blockNum);
        result = sendBlock(blockNum, dataBuff);
        if (!result) {
          return false;
        }
      }
      DBGPRINT("EOT\n", blockNum);
      emAfBootloadSendByte(EOT);
      if (!emAfBootloadWaitChar(&rxData, true, ACK)) {
        DBGPRINT("NoEOTAck\n");
        return false;
      }
    }
  } else {
    DBGPRINT("badstate\n");
    return false;
  }
  return true;
}
