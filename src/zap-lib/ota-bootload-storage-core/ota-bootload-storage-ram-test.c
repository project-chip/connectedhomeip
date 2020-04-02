/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include "ota-bootload-storage-core-test.h"

void deletionTimePassesCallback(void)
{
  // With the RAM based implementation, the Delete() API returns synchronously.
}

int main(int argc, char *argv[])
{
  // From ota-bootload-storage-ram.c.
  extern void emZclOtaBootloadStorageInitCallback(void);
  emZclOtaBootloadStorageInitCallback();

  fprintf(stderr, "[%s ", argv[0]);
  runOtaBootloadStorageCoreTest(true); // runMultipleFilesTest?
  fprintf(stderr, " done]\n");

  return 0;
}
