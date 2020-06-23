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
 * @brief This file integrates a light's status in the network with the Exosite
 * cloud service.  It polls frequenctly to detect a change to the light
 * and then sends that to a connected Zigbee Internet Client.
 *******************************************************************************
   ******************************************************************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "app/framework/include/af.h"
#include "app/framework/plugin/exosite-demo/exosite-demo.h"
#include <errno.h>

//=============================================================================
// Globals

// This assumes the program is run from the root of the stack install.
const char script[] = "app/framework/plugin/exosite-demo/exosite-demo.py";
const char readOption[] = "--read";
const char writeOption[] = "--write";

//#define DEBUG_ON
#if defined(DEBUG_ON)
  #define debugPrintln(...) emberAfCorePrintln(__VA_ARGS__)
#else
  #define debugPrintln(...)
#endif

#define EXOSITE_READ_INTERVAL_QS (3 << 2)

#define MAX_LINE_LENGTH 5000

// "state=X"
#define MIN_READ_RESULT_LENGTH 7

#define MAX_COMMAND_LENGTH 500

static bool lightState = false;

EmberEventControl emberAfPluginExositeDemoCheckInEventControl;

static bool eventEnabled = true;

//=============================================================================

void emberAfPluginExositeDemoCheckInEventHandler(void);

//=============================================================================

void emberAfPluginExositeDemoInitCallback(void)
{
  emberAfPluginExositeDemoCheckInEventHandler();
}

static int runProgramCaptureResult(const char* command,
                                   char* resultText,
                                   bool* resultTruncated,
                                   int maxLength,
                                   int minLength)
{
  int pcloseStatus;
  int statusCode = 0;
  debugPrintln("Running Commond: %p", command);
  FILE* resultStream = popen(command, "r");
  if (resultStream == NULL) {
    emberAfCorePrintln("FATAL:  Could not open pipe for reading program execution:  %p", strerror(errno));
    return -1;
  }

  MEMSET(resultText, 0, maxLength);
  int readBytes = fread(resultText, 1, maxLength, resultStream);
  if (ferror(resultStream)) {
    emberAfCorePrintln("Error: Cannot read from pipe.");
    statusCode = -1;
    goto runProgramCaptureResultDone;
  }
  if (!feof(resultStream)) {
    debugPrintln("<-- Output truncated -->");
    *resultTruncated = true;
  }
  if (readBytes < minLength) {
    emberAfCorePrintln("Error: Result length too short (%d < %d).", readBytes, minLength);
    statusCode = -1;
    goto runProgramCaptureResultDone;
  }

  resultText[maxLength - 1] = '\0';
  debugPrintln("Results:\n%p", resultText);

  runProgramCaptureResultDone:
  pcloseStatus = pclose(resultStream);
  if (statusCode == 0) {
    statusCode = pcloseStatus;
  }
  return statusCode;
}

EmberStatus emberAfPluginExositeDemoGetLightStatus(bool* lightOn)
{
  bool truncated;
  char command[MAX_COMMAND_LENGTH];
  snprintf(command, MAX_COMMAND_LENGTH, "%s %s", script, readOption);

  char resultText[MAX_LINE_LENGTH];
  if (runProgramCaptureResult(command,
                              resultText,
                              &truncated,
                              MAX_LINE_LENGTH,
                              MIN_READ_RESULT_LENGTH)) {
    emberAfCorePrintln("Error:  Script read execution returned error: ", resultText);
    return EMBER_ERR_FATAL;
  }

  if (0 == strncmp("state=1", resultText, MIN_READ_RESULT_LENGTH)) {
    *lightOn = true;
  } else if (0 == strncmp("state=0", resultText, MIN_READ_RESULT_LENGTH)) {
    *lightOn = false;
  } else {
    return EMBER_ERR_FATAL;
  }

  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginExositeDemoSetLightStatus(bool turnLightOn)
{
  bool truncated;
  char command[MAX_COMMAND_LENGTH];
  snprintf(command,
           MAX_COMMAND_LENGTH,
           "%s %s %s",
           script,
           writeOption,
           (turnLightOn
            ? "on"
            : "off"));
  char resultText[MAX_LINE_LENGTH];
  if (runProgramCaptureResult(command,
                              resultText,
                              &truncated,
                              MAX_LINE_LENGTH,
                              0)) {  // min write result length
    emberAfCorePrintln("Error: Script write execution returned error: ", resultText);
    return EMBER_ERR_FATAL;
  }

  return EMBER_SUCCESS;
}

void emberAfPluginExositeDemoCheckInEventHandler(void)
{
  bool oldLightState = lightState;
  emberAfPluginExositeDemoGetLightStatus(&lightState);
  if (eventEnabled) {
    emberEventControlSetDelayQS(emberAfPluginExositeDemoCheckInEventControl,
                                EMBER_AF_PLUGIN_EXOSITE_DEMO_CHECK_IN_TIME_SECONDS << 2);
  }

  if (oldLightState != lightState) {
    emberAfCorePrintln("Light state changed to %p", (lightState ? "on" : "off"));
    emberAfPluginExositeDemoLightChangedStateCallback(lightState);
  }
}
#endif // DOXYGEN_SHOULD_SKIP_THIS
