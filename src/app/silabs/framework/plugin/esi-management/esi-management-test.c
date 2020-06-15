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
 * @brief Test for the ESI Management plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"
#include "app/framework/util/af-main.h"
#include "app/framework/test/test-framework.h"
#include "app/framework/plugin/esi-management/esi-management.h"

//------------------------------------------------------------------------------
// Function prototypes.
void emberAfPluginEsiManagementInitCallback(void);

//------------------------------------------------------------------------------
// Globals

EmberEUI64 longIds[] = {
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F },
};

EmberEUI64 bogusLongId = { 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF };
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

//------------------------------------------------------------------------------
// Functions

static uint8_t deleteCallbackCounter = 0;

void deleteCallBack(uint8_t index)
{
  deleteCallbackCounter++;
}

void esiTableTest(void)
{
  uint8_t i;
  EmberAfPluginEsiManagementEsiEntry* entry;

  // Initializing the table.
  emberAfPluginEsiManagementInitCallback();

  // Filling the ESI table with 0-age entries.
  for (i = 0; i < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    entry = emberAfPluginEsiManagementGetFreeEntry();
    expect(entry != NULL);
    entry->age = 0;
    entry->endpoint = 0;
    entry->nodeId = i;
    MEMMOVE(entry->eui64, longIds[i], EUI64_SIZE);
  }

  // Requesting an extra free entry should fail.
  entry = emberAfPluginEsiManagementGetFreeEntry();
  expect(entry == NULL);

  // Aging all entries n-1 times: we should not reuse any entry when a
  // freeEntry request arrives.
  for (i = 0; i < EMBER_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE - 1; i++) {
    emberAfPluginEsiManagementAgeAllEntries();
    entry = emberAfPluginEsiManagementGetFreeEntry();
    expect(entry == NULL);
  }

  // Aging one more time: we should start reusing entries.
  emberAfPluginEsiManagementAgeAllEntries();
  entry = emberAfPluginEsiManagementGetFreeEntry();
  expect(entry != NULL);

  // Re-initializing the table and filling the ESI table with 1-age entries.
  emberAfPluginEsiManagementInitCallback();
  for (i = 0; i < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    entry = emberAfPluginEsiManagementGetFreeEntry();
    expect(entry != NULL);
    entry->age = 1;
    entry->endpoint = 0;
    entry->nodeId = i;
    MEMMOVE(entry->eui64, longIds[i], EUI64_SIZE);
  }

  // Test deletion.
  emberAfPluginEsiManagementDeleteEntry(0);
  entry = emberAfPluginEsiManagementGetFreeEntry();
  expect(entry != NULL);
  // Re-adding the first entry.
  entry->age = 1;
  entry->endpoint = 0;
  entry->nodeId = 0;
  MEMMOVE(entry->eui64, longIds[0], EUI64_SIZE);

  // Test emberAfPluginEsiManagementEsiLookUpByShortIdAndEndpoint().
  entry = emberAfPluginEsiManagementEsiLookUpByShortIdAndEndpoint(0x0, 0);
  expect(entry != NULL);
  // Wrong short id.
  entry = emberAfPluginEsiManagementEsiLookUpByShortIdAndEndpoint(0xFF, 0);
  expect(entry == NULL);
  // Wrong endpoint.
  entry = emberAfPluginEsiManagementEsiLookUpByShortIdAndEndpoint(0x0, 1);
  expect(entry == NULL);

  // Test emberAfPluginEsiManagementEsiLookUpByLongIdAndEndpoint().
  entry = emberAfPluginEsiManagementEsiLookUpByLongIdAndEndpoint(longIds[0], 0);
  expect(entry != NULL);
  // Wrong long id.
  entry = emberAfPluginEsiManagementEsiLookUpByLongIdAndEndpoint(bogusLongId, 0);
  expect(entry == NULL);
  // Wrong endpoint.
  entry = emberAfPluginEsiManagementEsiLookUpByLongIdAndEndpoint(longIds[0], 1);
  expect(entry == NULL);

  // Test emberAfPluginEsiManagementIndexLookUpByShortIdAndEndpoint().
  i = emberAfPluginEsiManagementIndexLookUpByShortIdAndEndpoint(0x0, 0);
  expect(i != 0xFF);
  // Wrong short id.
  i = emberAfPluginEsiManagementIndexLookUpByShortIdAndEndpoint(0xFF, 0);
  expect(i == 0xFF);
  // Wrong endpoint.
  i = emberAfPluginEsiManagementIndexLookUpByShortIdAndEndpoint(0x0, 1);
  expect(i == 0xFF);

  // Test emberAfPluginEsiManagementIndexLookUpByLongIdAndEndpoint().
  i = emberAfPluginEsiManagementIndexLookUpByLongIdAndEndpoint(longIds[0], 0);
  expect(i != 0xFF);
  // Wrong long id.
  i = emberAfPluginEsiManagementIndexLookUpByLongIdAndEndpoint(bogusLongId, 0);
  expect(i == 0xFF);
  // Wrong endpoint.
  i = emberAfPluginEsiManagementIndexLookUpByLongIdAndEndpoint(longIds[0], 1);
  expect(i == 0xFF);

  //Test emberAfPluginEsiManagementEsiLookUpByIndex();
  entry = emberAfPluginEsiManagementEsiLookUpByIndex(0);
  expect(entry != NULL);
  entry = emberAfPluginEsiManagementEsiLookUpByIndex(EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE);
  expect(entry == NULL);

  // Test iterator function emberAfPluginEsiManagementGetNextEntry().

  // All entries have age 1, so this call should return NULL.
  entry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
  expect(entry == NULL);
  // Get first entry.
  entry = emberAfPluginEsiManagementGetNextEntry(NULL, 1);
  expect(entry != NULL);
  for (i = 1; i < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    entry = emberAfPluginEsiManagementGetNextEntry(entry, 1);
    expect(entry != NULL);
  }
  // We got to the end of the table, we should get NULL now.
  entry = emberAfPluginEsiManagementGetNextEntry(entry, 1);
  expect(entry == NULL);

  // We make the first entry with age 2, so now we should iterate through n-1
  // entries.
  entry = emberAfPluginEsiManagementGetNextEntry(NULL, 1);
  entry->age = 2;

  // Get first entry.
  entry = emberAfPluginEsiManagementGetNextEntry(NULL, 1);
  expect(entry != NULL);
  for (i = 1; i < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE - 1; i++) {
    entry = emberAfPluginEsiManagementGetNextEntry(entry, 1);
    expect(entry != NULL);
  }
  // We got to the end of the table, we should get NULL now.
  entry = emberAfPluginEsiManagementGetNextEntry(entry, 1);
  expect(entry == NULL);
}

void deletionCallbackTableTest(void)
{
  uint8_t i;
  EmberAfPluginEsiManagementEsiEntry* entry;

  // Initializing and filling the table with 0-age entries.
  emberAfPluginEsiManagementInitCallback();
  for (i = 0; i < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE; i++) {
    entry = emberAfPluginEsiManagementGetFreeEntry();
    expect(entry != NULL);
    entry->age = 0;
    entry->endpoint = 0;
    entry->nodeId = i;
    MEMMOVE(entry->eui64, longIds[i], EUI64_SIZE);
  }

  // Check that there are no empty entries.
  entry = emberAfPluginEsiManagementGetFreeEntry();
  expect(entry == NULL);

  // Subscribing plugins with the respective deletion callback functions until
  // we fill the table.
  for (i = 0; i < EMBER_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE; i++) {
    expect(emberAfPluginEsiManagementSubscribeToDeletionAnnouncements(deleteCallBack) == true);
  }
  // Subscribing an extra plugin should fail.
  expect(emberAfPluginEsiManagementSubscribeToDeletionAnnouncements(deleteCallBack) == false);

  // Test deletion callback by aging and free entry request.
  deleteCallbackCounter = 0;
  entry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
  entry->age = EMBER_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE;
  // The aged entry should be reused.
  entry = emberAfPluginEsiManagementGetFreeEntry();
  expect(entry != NULL);
  entry->age = 0;
  entry->endpoint = 0;
  entry->nodeId = i;
  MEMMOVE(entry->eui64, longIds[i], EUI64_SIZE);
  // Check that the callback has been called as many times as the number of
  // callback entries in the table.
  expect(deleteCallbackCounter == EMBER_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE);

  // Test deletion callback by direct deletion.
  deleteCallbackCounter = 0;
  emberAfPluginEsiManagementDeleteEntry(0);
  expect(deleteCallbackCounter == EMBER_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE);
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "esi-table", esiTableTest },
    { "callback-table", deletionCallbackTableTest },
    { NULL },
  };

  // For some reason the ESI Management tests only work with index 0.
  // Not sure if this is a bug in the tests or a bug in the plugin.
  // Ideally someone should look at this in the future.
  emberSetCurrentNetwork(0);

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "af-esi-management",
                                        allTests);
}
