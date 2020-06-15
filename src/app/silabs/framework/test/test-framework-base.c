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
 * @brief Base test code that can be used to parse a list of tests and execute
 * the right one.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"

#include "app/framework/test/test-framework.h"
#include "plugin/serial/ember-printf-convert.h"

#include "app/framework/util/af-main.h"

#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

//------------------------------------------------------------------------------
// Globals

static bool debugOn = false;
const char debugString[] = "--debug";
bool testFrameworkAllPrintingOff = false;
static const char* expectedCheckpointString = NULL;

#define MAX_FORMATTED_STRING_LENGTH 200

static const char* testFileForExpect = NULL;
static int lineNumberForExpect = 0;
static const char* functionNameForExpect = NULL;

static const uint32_t watchdogTimeoutSeconds = 2;
static bool  watchdogEnabled = false;

//------------------------------------------------------------------------------
// Functions

static void watchdogFunction(int signalNumber)
{
  // SIGALRM doesn't cause a core dump and I couldn't find a way to change that.
  // So we just catch it and assert so we can get a nice core dump with a backtrace.
  note("\nFATAL: WATCHDOG timer fired after %d seconds\n", watchdogTimeoutSeconds);
  noteFlush();
  assert(0);
}

static void installSignalHandler(void)
{
  static bool installed = false;
  if (!installed) {
    signal(SIGVTALRM, watchdogFunction);
    installed = true;
  }
}

#if defined(__CYGWIN__)
// Googling around I found that Cygwin doesn't appear to implement ITIMER_VIRTUAL,
// therefore we use ITIMER_REAL.  ITIMER_VIRTUAL only measures time while the process
// is running and that is more important in Unix where the user could Ctrl-Z or
// run with a debugger connected.
  #define TIMER_TYPE ITIMER_REAL
#else
// Only decrement when process is executing, not when for example we suspend
// the process with Ctrl-Z.
  #define TIMER_TYPE ITIMER_VIRTUAL
#endif

static void setWatchdogTimer(uint32_t timeValueSeconds)
{
  int result;
  struct itimerval timerStruct = {
    // it_interval (what happens when timer is reset)
    // We don't care about this since the behavior is to assert()
    // when the watchdog fires.
    {
      0,  // seconds
      0,  // micro seconds
    },

    // it_value (amount of time remaining on the timer)
    {
      timeValueSeconds,
      0,  // micro seconds
    },
  };
  result = setitimer(TIMER_TYPE,
                     &timerStruct,
                     NULL);  // previous timer value struct
  if (result) {
    note("setitimer() failed: %d (%s)\n", result, strerror(errno));
  }
  assert(result == 0);
}

void enableExpectWatchdog(void)
{
  installSignalHandler();
  watchdogEnabled = true;
  resetExpectWatchdog();
}

void disableExpectWatchdog(void)
{
  setWatchdogTimer(0);
}

void resetExpectWatchdog(void)
{
  if (watchdogEnabled) {
    setWatchdogTimer(watchdogTimeoutSeconds);
  }
}

bool isWatchdogEnabled(void)
{
  return watchdogEnabled;
}

bool isTestFrameworkDebugOn(void)
{
  return debugOn;
}

// Most scripts will enable this via the '--debug' command-line option.
// This should only be necessary in unusual circumstances where the debug
// must be changed dynamically during the script's execution.
void setTestFrameworkDebug(bool enable)
{
  debugOn = enable;
}

static void myVfprintf(FILE *stream, const char *formatString, va_list ap)
{
  char* newFormat = transformEmberPrintfToStandardPrintf(formatString,
                                                         CONVERT_SLASH_R);
  assert(newFormat);
  if (!testFrameworkAllPrintingOff) {
    vfprintf(stream, newFormat, ap);
  }
  free(newFormat);
}

void expectFunction(bool assertTest,
                    const char* file,
                    int lineNumber,
                    const char* functionName,
                    const char* failureMessage,
                    ...)
{
  va_list ap;
  va_start(ap, failureMessage);

  if (assertTest) {
    debug("\nMatched expect() from: %s:%d - %s() \n",
          file,
          lineNumber,
          functionName);
    note(".");
  } else {
    note("\nexpect() failed: %s:%d - %s() \n",
         file,
         lineNumber,
         functionName);
    if (failureMessage) {
      myVfprintf(stderr, failureMessage, ap);
    }
    assert(0);
  }

  if (expectedCheckpointString != NULL) {
    note("Error: Did not get expected string.\n  String: %s\n",
         expectedCheckpointString);
    assert(0);
  }

  resetExpectWatchdog();
}

void expectComparison(uint32_t valueA,
                      uint32_t valueB,
                      bool printHex,
                      const char* nameA,
                      const char* nameB,
                      const char* file,
                      int lineNumber,
                      const char* functionName)
{
  const char formatHex[] = "%s (0x%08X) != %s (0x%08X)\n";
  const char formatDec[] = "%s (%d) != %s (%d)\n";
  expectFunction(valueA == valueB,
                 file,
                 lineNumber,
                 functionName,
                 (printHex ? formatHex : formatDec),
                 nameA,
                 valueA,
                 nameB,
                 valueB);
}

void setCheckpointFunction(const char* string,
                           const char* file,
                           int lineNumber,
                           const char* function)
{
  // Multiple calls to set a checkpoint is not allowed.
  // If that needs to change because multiple checkpoints occur
  // in a single function, we would have to change the expected strings
  // to be an array.

  if (expectedCheckpointString != NULL) {
    note("Error: Multiple calls to setCheckpointFunction() not allowed.\n");
  }
  assert(expectedCheckpointString == NULL);
  expectedCheckpointString = string;

  testFileForExpect = file;
  lineNumberForExpect = lineNumber;
  functionNameForExpect = function;

  // Make sure we aren't setting a checkpoint bigger than we can handle.
  assert(MAX_FORMATTED_STRING_LENGTH
         >= strnlen(string, MAX_FORMATTED_STRING_LENGTH));
}

void expectCheckpointFunction(const char* file,
                              int lineNumber,
                              const char* formatString,
                              ...)
{
  char formattedMessage[MAX_FORMATTED_STRING_LENGTH];
  va_list argPointer;
  va_start(argPointer, formatString);
  vsnprintf(formattedMessage,
            MAX_FORMATTED_STRING_LENGTH,
            formatString,
            argPointer);

  if (expectedCheckpointString == NULL) {
    note("\nError: Unexpected scriptTestCheckpoint() from %s:%d\n  Got: %s",
         file,
         lineNumber,
         formattedMessage);
    assert(0);
  }
  if (0 == strncmp(formattedMessage,
                   expectedCheckpointString,
                   MAX_FORMATTED_STRING_LENGTH)) {
    debug("Got expected string from  %s:%d\n"
          "setCheckpoint() test code %s:%d: %s\n",
          file,
          lineNumber,
          testFileForExpect,
          lineNumberForExpect,
          expectedCheckpointString);
    expectedCheckpointString = NULL;
  } else {
    note("\nError: Mismatched script checkpoint.\n"
         "Actual string from:      %s:%d\n"
         "Expected setCheckpoint() %s:%d\n",
         file,
         lineNumber,
         testFileForExpect,
         lineNumberForExpect);
    note("  Expected: %s\n", expectedCheckpointString);
    note("  Actual:   %s\n", formattedMessage);
    assert(0);
  }

  testFileForExpect = NULL;
  lineNumberForExpect = 0;

  note(".");
}

void verifyCheckpoint(void)
{
  if (expectedCheckpointString != NULL) {
    note("Error: Did not get expected checkpoint from test code %s:%d.\n  Expected: %s\n",
         testFileForExpect,
         lineNumberForExpect,
         expectedCheckpointString);
    assert(0);
  }

  testFileForExpect = NULL;
  lineNumberForExpect = 0;

  note(".");
}

void note(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  myVfprintf(stderr, format, ap);
  va_end(ap);
}

void noteFlush(void)
{
  fflush(stderr);
}

void debugFlush(void)
{
  if (debugOn) {
    fflush(stderr);
  }
}

void vDebug(const char* format, va_list ap)
{
  if (debugOn) {
    myVfprintf(stderr, format, ap);
  }
}

void debug(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  vDebug(format, ap);
  va_end(ap);
}

static void printUsage(const char* programName, const TestCase* allTests)
{
  int i = 0;
  note("%s <test> [ %s ]\n",
       programName,
       debugString);
  note("Suppored Tests:\n");

  while (allTests[i].name != NULL) {
    note("  %s\n", allTests[i].name);
    i++;
  }
}

bool parseCommandLineAndExecuteTest(int argc,
                                    char* argv[],
                                    const char* programName,
                                    const TestCase* allTests)
{
  int i = 0;

  if (argc == 3) {
    if (0 == strncmp(argv[2], debugString, strlen(debugString))) {
      debugOn = true;
    } else {
      printUsage(programName, allTests);
      return -1;
    }
  } else if (argc != 2) {
    printUsage(programName, allTests);
    return -1;
  }

  const TestCase* testToRun = NULL;
  while (allTests[i].name != NULL && testToRun == NULL) {
    if (0 == strncmp(allTests[i].name, argv[1], strlen(allTests[i].name))) {
      testToRun = &(allTests[i]);
      continue;
    }
    i++;
  }

  if (testToRun == NULL) {
    note("Error: Unknown test '%s'\n", argv[1]);
    printUsage(programName, allTests);
    return -1;
  }

  note("[Testing %s %s ", programName, testToRun->name);
  (testToRun->functionPtr)();
  note(" done]\n");
  return 0;
}
