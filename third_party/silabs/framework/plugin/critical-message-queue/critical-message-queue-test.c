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
 * @brief Critical Message Queue Unit tests
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "critical-message-queue.h"

#include "app/framework/test/test-framework.h"

//------------------------------------------------------------------------------
// Globals

#define MILLISECOND_TICKS_SINCE_BOOT \
  (MILLISECOND_TICKS_PER_SECOND      \
   * 60 /* seconds */                \
   * 60 /* minutes */                \
   * 3) /* hours */

// Forward Declarations

//------------------------------------------------------------------------------
// Stubs
uint16_t emberAfLongStringLength(const uint8_t *buffer)
{
  // Should not be called during unit testing
  assert(0);
  return 0;
}

uint8_t emberAfStringLength(const uint8_t *buffer)
{
  // Should not be called during unit testing
  assert(0);
  return 0;
}

//------------------------------------------------------------------------------
// Implementation

void runAllTests(void)
{
  emberAfPluginCriticalMessageQueueInit();
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "all-tests", runAllTests },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "af-plugin-critical-message-queue-test",
                                        allTests);
}
