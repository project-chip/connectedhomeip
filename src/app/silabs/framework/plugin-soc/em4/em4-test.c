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
 * @brief Unit tests for APS link key authorization
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "em4-test.h"
#include "app/framework/util/common.h"

#include "app/framework/plugin-soc/idle-sleep/idle-sleep.h"
#include "app/framework/test/test-framework.h"
// -----------------------------------------------------------------------------
// Variables
// -----------------------------------------------------------------------------
static bool expectEm4Reset = false;
static bool postEm4Reset = false;
// -----------------------------------------------------------------------------
// Interface for callbacks implemeted in the plugin and tested here
// -----------------------------------------------------------------------------
void emberAfPluginIdleSleepTickCallback(void);
// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// stubbed out functions: In general, we don't care about these functions; they
// just need to compile and always return success when called at runtime
// -----------------------------------------------------------------------------
bool emAfOkToIdleOrSleep(void)
{
  return true;
}

EmberTaskId emAfTaskId()
{
  return 0;
}

void emMarkTaskActive(EmberTaskId taskid)
{
}

void emberAfEepromNoteInitializedStateCallback(bool state)
{
}
void emberAfEepromShutdownCallback()
{
}

EmberStatus emSetFrameCounter(EmberKeyType keyType, uint32_t desiredValue)
{
  return EMBER_SUCCESS;
}

void emberAfPluginIdleSleepActiveCallback()
{
}
bool emberAfPluginIdleSleepOkToIdleCallback()
{
  return true;
}
bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  return true;
}
bool emberAfPluginIdleSleepRtosCallback(uint32_t *durationMs, bool sleepOk)
{
  return false;
}

uint16_t emberCurrentStackTasks()
{
  return 0;
}

void emberAfPluginIdleSleepWakeUpCallback(uint32_t durationMs)
{
}

uint32_t emberGetApsFrameCounter()
{
  return 0;
}
uint32_t emberGetIncomingTcLinkKeyFrameCounter()
{
  return 0;
}
uint32_t emberGetParentIncomingNwkFrameCounter()
{
  return 0;
}
uint32_t emberGetSecurityFrameCounter(void)
{
  return 0;
}
bool emberMarkTaskIdle(EmberTaskId taskid)
{
  return true;
}
uint32_t emberMsToNextEvent(EmberEventData *events, uint32_t maxMs)
{
  return 0;
}
uint32_t emberMsToNextStackEvent()
{
  return 0;
}

EmberStatus emberNetworkInit(EmberNetworkInitStruct* networkInitStruct)
{
  return EMBER_SUCCESS;
}
bool emberParentTokenSet()
{
  return true;
}
void emberSetIncomingTcLinkKeyFrameCounter(uint32_t frameCounter)
{
}
EmberStatus emberSetParentIncomingNwkFrameCounter(uint32_t value)
{
  return EMBER_SUCCESS;
}
void emberStackPowerDown()
{
}
void emberStackPowerUp()
{
}
uint32_t halCommonGetInt32uMillisecondTick()
{
  return 0;
}
void halSleep(SleepModes sleepMode)
{
}

void emberAfPostEm4ResetCallback()
{
  postEm4Reset = true;
};

bool emberAfPluginIdleSleepPreEm4ResetCallback(uint32_t* sleepDurationMs)
{
  //making sure this function gets called from idle-sleep plugin
  expectEm4Reset = true;
  return true;
};
EmberEventData emAfEvents[] = {
  { NULL, NULL }
};
// -----------------------------------------------------------------------------
// test cases: These are the test cases called by the test framework when unit
// tests are run.
// -----------------------------------------------------------------------------

static void em4PreResetTest(void)
{
  expectEm4Reset = false;
  emberAfPluginIdleSleepTickCallback();
  assert(expectEm4Reset == true);
}

int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "em4-pre-reset-test", em4PreResetTest },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc, argv, "EM4", tests);
}
