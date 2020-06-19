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
 * @brief Common test code and stubs for unit tests of the Afv2 code.
 *******************************************************************************
   ******************************************************************************/

#include "test-framework-network.h"

typedef void (FunctionPtr)(void);

typedef struct {
  const char* name;
  FunctionPtr* functionPtr;
} TestCase;

bool parseCommandLineAndExecuteTest(int argc,
                                    char* argv[],
                                    const char* programName,
                                    const TestCase* allTests);

void note(const char* format, ...);
void debug(const char* format, ...);
void vDebug(const char* format, va_list ap);

void noteFlush(void);
void debugFlush(void);

#define TEST_ZCL_SEQUENCE 0xAB  // arbitrarily chosen just for testing

extern bool giveEuiLookupResponse;
extern EmberEUI64 euiLookupResponse;
extern bool expectIeeeRequest;

void verifyMessage(const char* name, uint16_t length, const uint8_t* message);

void verifyDataWithFileInfo(const char* name,
                            uint16_t expectedLength,
                            uint8_t* expectedData,
                            uint16_t actualLength,
                            uint8_t* actualData,
                            const char* file,
                            int lineNumber);

#define verifyData(name, expectedLength, expectedData, actualLength, actualData) \
  verifyDataWithFileInfo(name, expectedLength, expectedData, actualLength, actualData, __FILE__, __LINE__)

typedef struct {
  bool read;
  uint8_t endpoint;
  EmberAfClusterId cluster;
  EmberAfAttributeId attributeId;
  uint8_t *dataPtr;
  uint8_t maxLength;
  EmberAfAttributeType *dataType;
} AttributeCallbackData;

typedef EmberAfStatus (AttributeCallbackFunction)(AttributeCallbackData*);

extern AttributeCallbackFunction* attributeCallbackFunction;

void testAfSetNetworkState(EmberNetworkStatus networkState);

void expectFunction(bool assertTest,
                    const char* file,
                    int lineNumber,
                    const char* functionName,
                    const char* failureMessage,
                    ...);

void expectComparison(uint32_t valueA,
                      uint32_t valueB,
                      bool printHex,
                      const char* nameA,
                      const char* nameB,
                      const char* file,
                      int lineNumber,
                      const char* functionName);

#define expect(test) \
  expectFunction((test), __FILE__, __LINE__, __PRETTY_FUNCTION__, NULL)

#define expectMessage(test, ...) \
  expectFunction((test), __FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)

#define expectComparisonDecimal(valueA, valueB, nameA, nameB) \
  expectComparison((valueA), (valueB), false, nameA, nameB, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define expectComparisonHex(valueA, valueB, nameA, nameB) \
  expectComparison((valueA), (valueB), true, nameA, nameB, __FILE__, __LINE__, __PRETTY_FUNCTION__)

bool isTestFrameworkDebugOn(void);
void setTestFrameworkDebug(bool enable);

extern bool testFrameworkAllPrintingOff;

void emEmberTestSetExternalBuffer(void);

void setCheckpointFunction(const char* string,
                           const char* file,
                           int lineNumber,
                           const char* function);
#define setCheckpoint(string) \
  setCheckpointFunction(string, __FILE__, __LINE__, __PRETTY_FUNCTION__)

void expectCheckpointFunction(const char* file,
                              int lineNumber,
                              const char* string,
                              ...);

#define expectCheckpoint(...) \
  expectCheckpointFunction(__FILE__, __LINE__, __VA_ARGS__)

void verifyCheckpoint(void);

void enableExpectWatchdog(void);
void disableExpectWatchdog(void);
void resetExpectWatchdog(void);

bool isWatchdogEnabled(void);
