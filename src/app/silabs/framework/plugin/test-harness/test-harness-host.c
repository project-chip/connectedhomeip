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
 * @brief Test harness code specific to the SOC.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

#include "test-harness.h"

#include "app/xncp-test-harness/xncp-test-harness.h"

//------------------------------------------------------------------------------
// Globals

// For easier readability
#define RESET_FRAME_COUNTER false
#define ADVANCE_FRAME_COUNTER true

EmberEventControl emberAfPluginTestHarnessFlashPageHashEventControl;

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Functions

// Returns false if XNCP is not the right software and doesn't support our
// custom EZSP frames, true if it does.
static bool checkXncpSoftware(void)
{
  uint16_t manufacturerId;
  uint16_t versionNumber;

  EmberStatus status;

  status  = ezspGetXncpInfo(&manufacturerId, &versionNumber);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Error: XNCP test harness software not present on NCP.");
    return false;
  } else if (manufacturerId != EMBER_MANUFACTURER_ID
             || versionNumber != EMBER_XNCP_TEST_HARNESS_VERSION_NUMBER) {
    emberAfCorePrintln("Error: Wrong XNCP software loaded on NCP.");
    emberAfCorePrintln("  Wanted Manuf ID: 0x%2X, Version: 0x%2X",
                       EMBER_MANUFACTURER_ID,
                       EMBER_XNCP_TEST_HARNESS_VERSION_NUMBER);
    emberAfCorePrintln("     Got Manuf ID: 0x%2X, Version: 0x%2X",
                       manufacturerId,
                       versionNumber);
    return false;
  }

  return true;
}

static void resetOrAdvanceApsFrameCounter(bool advance)
{
  uint8_t customEzspMessage[1];
  uint8_t reply[1];
  uint8_t replyLength = 1;
  EmberStatus status;

  customEzspMessage[0] = (advance
                          ? EMBER_XNCP_TEST_HARNESS_COMMAND_ADVANCE_APS_FRAME_COUNTER
                          : EMBER_XNCP_TEST_HARNESS_COMMAND_RESET_APS_FRAME_COUNTER);

  if (!checkXncpSoftware()) {
    return;
  }

  status = ezspCustomFrame(1, // length of custom EZSP message
                           customEzspMessage,
                           &replyLength,
                           reply);
  emberAfCorePrintln("%p APS Frame counter: %p (0x%X)",
                     (advance
                      ? "Advancing"
                      : "Reset"),
                     ((status == EMBER_SUCCESS)
                      ? "Success"
                      : "FAILED"),
                     status);
}

void emAfTestHarnessResetApsFrameCounter(void)
{
  // Reseting the outgoing APS frame counter is non-standard and not
  // a good idea, especially on the TC.  This is necessary for Smart Energy
  // Key establihsment tests 15.39 and 15.40.  It is only necessary for a test
  // harness device.

  // In the case of the Host, the XNCP test harness software must be loaded
  // or this cannot be done.  Since resetting frame counters is non-standard
  // and can break things, it is not part of the default NCP binary.

  resetOrAdvanceApsFrameCounter(RESET_FRAME_COUNTER);
}

void emAfTestHarnessAdvanceApsFrameCounter(void)
{
  resetOrAdvanceApsFrameCounter(ADVANCE_FRAME_COUNTER);
}

void emAfTestHarnessStartImageStampCalculation(void)
{
  emberAfCorePrintln("Not supported");
}

void emberAfPluginTestHarnessFlashPageHashEventHandler(void)
{
}

//------------------------------------------------------------------------------
