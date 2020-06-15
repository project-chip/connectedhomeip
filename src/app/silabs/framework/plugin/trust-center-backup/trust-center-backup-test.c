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
 * @brief Test code for the Trust Center Backup plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"
#include "app/framework/util/af-main.h"

#include "app/framework/test/test-framework.h"
#include "app/framework/test/test-framework-security.h"
#include "app/framework/plugin/trust-center-backup/trust-center-backup.h"

//------------------------------------------------------------------------------
// Globals

static const char testFile[] = "/tmp/trust-center-backup-test-file.txt";

bool emberAfClearLinkKeyTableUponFormingOrJoining = true;

//------------------------------------------------------------------------------
// Stubs

uint8_t emberCopyStringArgument(int8_t argNum,
                                uint8_t *destination,
                                uint8_t maxLength,
                                bool leftPad)
{
  // Unimplemented stub.  Should not be called.
  assert(0);
  return 0;
}

uint8_t emberCopyBigEndianEui64Argument(int8_t index, EmberEUI64 destination)
{
  // Unimplemented stub.  Should not be called.
  assert(0);
}

uint32_t emberUnsignedCommandArgument(uint8_t argNum)
{
  // Unimplemented stub.  Should not be called.
  assert(0);
}

uint16_t halInternalStartSystemTimer(void)
{
  // Unimplemented stub.  Should not be called.
  assert(0);
  return 0;
}

void halInternalStartSymbolTimer(void)
{
  // Unimplemented stub.  Should not be called.
  assert(0);
}

EmberStatus emberAfFindUnusedPanIdAndFormCallback(void)
{
  return EMBER_SUCCESS;
}

void emberAfGetFormAndJoinExtendedPanIdCallback(uint8_t *resultLocation)
{
}

void emberAfSetFormAndJoinExtendedPanIdCallback(const uint8_t *extendedPanId)
{
}

static EmberKeyStruct oldTestFrameworkKeyTable[TEST_FRAMEWORK_KEY_TABLE_SIZE];

//------------------------------------------------------------------------------
// Functions

static void importFileTest(void)
{
  // The assumption with this test is that the testFile has already been
  // created with data from app/framework/test/test-framework-security.c.

  MEMMOVE(oldTestFrameworkKeyTable,
          testFrameworkKeyTable,
          sizeof(EmberKeyStruct) * TEST_FRAMEWORK_KEY_TABLE_SIZE);

  // Clear the key table so we can verify it was imported correctly.
  MEMSET(testFrameworkKeyTable,
         0,
         sizeof(EmberKeyStruct) * TEST_FRAMEWORK_KEY_TABLE_SIZE);

  expect(EMBER_SUCCESS == emberAfTrustCenterImportBackupFromFile(testFile));

  uint8_t i;

  // We assume that the exported key table is exactly matched to the import
  // key table.  In other words, all entries in the table are filled with
  // keys and there are no blank entries.
  for (i = 0; i < emberAfGetKeyTableSize(); i++) {
    EmberKeyStruct keyStruct;
    EmberStatus status = emberGetKeyTableEntry(i,
                                               &keyStruct);
    expect(status == EMBER_SUCCESS);
    expect(0 == MEMCOMPARE(oldTestFrameworkKeyTable[i].partnerEUI64,
                           testFrameworkKeyTable[i].partnerEUI64,
                           EUI64_SIZE));
    expect(EMBER_SUCCESS == emberAesHashSimple(EMBER_ENCRYPTION_KEY_SIZE,
                                               emberKeyContents(&(oldTestFrameworkKeyTable[i].key)),
                                               emberKeyContents(&(oldTestFrameworkKeyTable[i].key))));
    expect(0 == MEMCOMPARE(emberKeyContents(&(oldTestFrameworkKeyTable[i].key)),
                           emberKeyContents(&(testFrameworkKeyTable[i].key)),
                           EMBER_ENCRYPTION_KEY_SIZE));
  }
}

static void exportFileTest(void)
{
  // Must make the device the coordinator
  testFrameworkNodeId = 0x0000;

  expect(EMBER_SUCCESS == emberAfTrustCenterExportBackupToFile(testFile));
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "import-file", importFileTest },
    { "export-file", exportFileTest },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "af-trust-center-backup",
                                        allTests);
}
