/***************************************************************************//**
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
