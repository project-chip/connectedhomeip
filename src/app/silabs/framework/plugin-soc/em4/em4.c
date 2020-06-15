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
 * @brief Definitions for the EM4 plugin, which implementes a sleep mode for
 *        EFR32 series devices.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-event.h"
#include "em4.h"

#ifdef EMBER_SCRIPTED_TEST
#include "app/framework/util/af-main.h"
#include "em4-test.h"
RTCCRamData halAfterEM4()
{
  RTCCRamData output;
  MEMSET(&output, 0, sizeof(RTCCRamData));
  return output;
}
#endif

/*
 * this function is called from idle/sleep plugin
 */
bool emberAfPluginIdleSleepPreEm4ResetCallback(uint32_t* sleepDurationMs)
{
  // This is called from the idle-sleep-soc plugin.
  // Needs to be implemented by app.
  if (emberAfPluginEm4EnterCallback(sleepDurationMs)) {
    // The following should be called after application code
    // this only saves a few of the important stack counters and nothing more
    RTCCRamData input;
    input.outgoingNwkFrameCounter = emberGetSecurityFrameCounter();
    input.incomingParentNwkFrameCounter = emberGetParentIncomingNwkFrameCounter();
    input.outgoingLinkKeyFrameCounter = emberGetApsFrameCounter();
    input.incomingLinkKeyFrameCounter = emberGetIncomingTcLinkKeyFrameCounter();

    // Set the timer on the hardware, and save the in/out NWK counter.
    halBeforeEM4(*sleepDurationMs, input);
    return true;
  }
  return false;
}

/*
 * this function is called from af-main-soc
 */
void emberAfPostEm4ResetCallback(void)
{
  // This is called before application code.
  RTCCRamData output = halAfterEM4();

  EmberStatus status = emberSetParentIncomingNwkFrameCounter(output.incomingParentNwkFrameCounter);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("WARNING: Undefined behavior. Entered EM4 sleep mode without or before saving the parent info to the flash.");
  } else {
    emberSetOutgoingNwkFrameCounter(output.outgoingNwkFrameCounter);
    emberSetOutgoingApsFrameCounter(output.outgoingLinkKeyFrameCounter);
    emberSetIncomingTcLinkKeyFrameCounter(output.incomingLinkKeyFrameCounter);
  }

  // This is should restore the outgoing network counter,
  // if app requires any additional form of network initialization then it
  //  needs to be implement the following
  emberAfPluginEm4ExitCallback();
}
