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
 * @brief Routines for bootloading from a host connected via SPI to an NCP
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "hal/micro/bootloader-interface-standalone.h"

#include "ota-bootload-ncp.h"

#if defined(EMBER_AF_PLUGIN_EZSP_SPI)

#if defined(__ICCAVR__)
  #include "hal/micro/avr-atmega/spi-protocol.h"
#elif defined(UNIX_HOST)
  #include "hal/micro/unix/host/spi-protocol-common.h"
#else
  #include "hal/host/spi-protocol-common.h"
#endif

//#define SPI_DEBUG
#if defined(SPI_DEBUG)
  #define DBG(foo) foo
  #define DBGPRINT(...) otaPrintln(__VA_ARGS__)
#else
  #define DBG(foo)
  #define DBGPRINT(...)
#endif

//------------------------------------------------------------------------------
// Globals

// from spi-protocol
extern uint8_t halNcpSpipBuffer[];

#define BOOTLOADER_QUERY      ('Q')
#define BOOTLOADER_QUERYFOUND (0x1A)
#define XMODEM_FILEDONE       (0x17)
#define XMODEM_BLOCKOK        (0x19)
#define XMODEM_SOH            (0x01)
#define XMODEM_EOT            (0x04)

// We assume the watchdog is 2 seconds.  Resetting it 2 more times
// after 1 second intervals will give us a total of 5 seconds for
// the NCP to verify the CRC.  After that, we let the watchdog
// execute normally and reset the host if the NCP hasn't sent
// us any data.
#if !defined(EMBER_AF_NCP_BOOTLOAD_WATCHDOG_TIMER_RESET_COUNT)
  #define WATCHDOG_TIMER_RESET_COUNT 2
#else
  #define WATCHDOG_TIMER_RESET_COUNT EMBER_AF_NCP_BOOTLOAD_WATCHDOG_TIMER_RESET_COUNT
#endif

static uint16_t timeInFuture = 0;
static uint8_t watchdogResetCount;

//------------------------------------------------------------------------------
// Forward declarations

static void spiTick(void);

//------------------------------------------------------------------------------

// pass throuth the block to the NCP via SPI
// returns NULL if error, or pointer to the response starting with the length
static uint8_t *sendRawBlock(const uint8_t *message, uint8_t length)
{
  EzspStatus status;

  //mark the command type
  halNcpSpipBuffer[0] = 0xFD; //Bootloader payload
  halNcpSpipBuffer[1] = length;
  MEMMOVE(&(halNcpSpipBuffer[2]), message, length);

  halResetWatchdog(); // Full time

  //send the xmodem frame to EM260 over spi
  halNcpSendRawCommand();

  //wait for response of EM260.
  halResetWatchdog(); // Full time
  status = halNcpPollForResponse();
  while (status == EZSP_SPI_WAITING_FOR_RESPONSE) {
    status = halNcpPollForResponse();
  }

  //return the EM260 response status
  if ((status != EZSP_SUCCESS) || (halNcpSpipBuffer[0] != 0xFD)) {
    return NULL;
  } else {
    return &halNcpSpipBuffer[1];
  }
}

// returns result of a previous command/message
static uint8_t *getResult(void)
{
  uint8_t queryMessage = BOOTLOADER_QUERY;

  halResetWatchdog();

  watchdogResetCount = WATCHDOG_TIMER_RESET_COUNT;

  // Now poll for response
  while (!halNcpHasData()) {
    // Bug: 13227
    // Sending the last block causes the NCP to do a verification
    // on the EBL file, which involves calculating a 32-bit CRC.
    // This may take a while, especially on the 260.  Often this
    // takes longer than the 2 second watchdog timer on the host.
    // So we have to reset the watchdog a few more times after sending
    // the last block and before receiving a response to give the NCP
    // more time.  However we only do this a couple times and if after
    // that point there is no response from the NCP, we let the watchdog
    // take action and reset the host.

    // A further complication is that it is the EBL 'end' tag that kicks
    // off the EBL verification, which may or may not occur in the
    // last xmodem block.  So we just elongate the watchdog timer for
    // all packets rather thanb try to figure out whether the NCP has gone
    // off to do the EBL verification.
    spiTick();
  }

  // BOOTLOADER_QUERY is overloaded both for sending a QUERY request and for
  //  asking for the result of a previous message
  return sendRawBlock(&queryMessage, 1);
}

// process a bootloader query operation
static uint8_t *bootloaderQuery(void)
{
  uint8_t queryMessage = BOOTLOADER_QUERY;
  uint8_t *response;

  response = sendRawBlock(&queryMessage, 1);
  if (response == NULL) {
    DBGPRINT("CommError\n");
    return NULL;
  } else if (response[1] != BOOTLOADER_QUERYFOUND) {
    DBGPRINT("NoQueryFound\n");
    return NULL;
  }

  return getResult();
}

bool emAfStartNcpBootloaderCommunications(void)
{
  uint8_t *queryResp;
  uint16_t j;
  bool spiActive = false;

  // TODO: make this a more appropriate timeout?
  for (j = 0; (j < 40000) && !halNcpHasData(); j++) {
    halResetWatchdog();
    halCommonDelayMicroseconds(100);
  }
  DBGPRINT("loops 0x%2x\n", j);

  // the first call is expected to fail right after a reset,
  //  since the SPI is reporting the error that a reset occurred.
  halNcpVerifySpiProtocolActive();

  // the second check better return true indicating the SPIP is active
  for (j = 0; j < 10 && !spiActive; j++) {
    halResetWatchdog();
    halCommonDelayMicroseconds(100);
    spiActive = halNcpVerifySpiProtocolActive();
  }
  DBGPRINT("loops %d\n", j);

  if (!spiActive) {
    otaPrintln("Error: SPI protocol failed to start.");
    return false;
  }

  // the third check better return true indicating the proper SPIP version
  if (!halNcpVerifySpiProtocolVersion()) {
    otaPrintln("Error: SPI Protocal version is invalid!");
    return false;
  }
  DBGPRINT("AOK\n");

  // Do an bootloader query to confirm we are in the bootloader
  queryResp = bootloaderQuery();
  // just test for non-null to confirm bootloader response received
  //  detailed contents of the query response are ignored for now.
  // TODO: do we want to verify anything from the response?

  return (queryResp != NULL);
}

void emAfPostNcpBootload(bool success)
{
  // TODO:  Call an application callback.

  emberAfCorePrintln("Rebooting host");
  halReboot();
}

bool emAfRebootNcpAfterBootload(void)
{
  // ezsp-spi bootloader automatically reboots after upload finishes
  return true;
}

bool emAfBootloadSendData(const uint8_t *data, uint16_t length)
{
  uint8_t *response;

  response = sendRawBlock(data, length);

  // I really prefer to not do this sort of hacky test here, but the ezsp-spi
  //  bootloader protocol isn't very consistent in the way it handles initial
  //  acknowledgement
  DBGPRINT("SNDRSP %x,%x\n", data[0], response[1]);
  return  ( ( (data[0] == XMODEM_SOH) && (response[1] == XMODEM_BLOCKOK) )
            || ( (data[0] == XMODEM_EOT) && (response[1] == XMODEM_FILEDONE) ) );
}

bool emAfBootloadSendByte(uint8_t byte)
{
  return emAfBootloadSendData(&byte, 1);
}

bool emAfBootloadWaitChar(uint8_t *data, bool expect, uint8_t expected)
{
  uint8_t *response;

  response = getResult();
  if (response == NULL) {
    return false;
  }

  // all but one char of the full result is ignored.  This char is part of
  //  the xmodem protocol, the rest of the result may be useful for debugging
  //  but isn't used by xmodem
  *data = response[1];
  DBGPRINT("WCHAR 0x%x\n", response[1]);

  if (expect) {
    return ((*data) == expected);
  } else {
    return true;
  }
}

// This tick is not executed except when we are bootloading the NCP.
// We assume it will only execute once during boot since it uses static
// variables initialized at reset.  Regardless of whether the bootload
// succeeded or failed, we expect the host will reset.
// At that point if the bootload failed, the host can try again.

static void calculateTimeInFuture(void)
{
  timeInFuture = (halCommonGetInt16uMillisecondTick()
                  + (uint16_t)MILLISECOND_TICKS_PER_SECOND); // i.e. 1 sec
  // Since we compare this value to 0 to determine if it is uninitialized,
  // we must make sure we don't accidentally get 0 as the current
  // tick time.
  if (timeInFuture == 0) {
    timeInFuture++;
  }
}

static void spiTick(void)
{
  if (watchdogResetCount > 0) {
    uint16_t currentMillisecondTick = halCommonGetInt16uMillisecondTick();
    if (timeInFuture == 0) {
      calculateTimeInFuture();
    }

    if (timeGTorEqualInt16u(currentMillisecondTick,
                            timeInFuture)) {
      DBGPRINT("Resetting watchdog, remaining count: %d\n", watchdogResetCount);
      halResetWatchdog();
      watchdogResetCount--;
      calculateTimeInFuture();
    }
  }
}

#endif // !Gateway app.
