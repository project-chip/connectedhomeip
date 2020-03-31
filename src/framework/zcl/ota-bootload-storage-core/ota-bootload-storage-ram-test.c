/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
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
