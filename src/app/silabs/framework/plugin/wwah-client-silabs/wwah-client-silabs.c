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
 * @brief Routines for the WWAH Client Silabs plugin, which is the client
 *        implementation of the WWAH Silabs cluster.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "app/framework/plugin-host/dido-storage-posix-filesystem/dido-storage-linux.h"

void emberAfSlWwahClusterClientInitCallback(uint8_t endpoint)
{
#ifdef EMBER_AF_PLUGIN_DIDO_STORAGE_POSIX_FILESYSTEM
  emberAfPluginDidoStorageInitCallback();
#endif
}

// Used to handle debug report query response.
bool emberAfSlWwahClusterDebugReportQueryResponseCallback(uint8_t debugReportId,
                                                          uint8_t* debugReportData)
{
  uint16_t bufferLength;
  uint32_t crc;
  uint32_t readCrc;
  uint32_t preCrcLength;
  if (emberAfCurrentCommand()->bufLen > emberAfCurrentCommand()->payloadStartIndex ) {
    bufferLength = emberAfCurrentCommand()->bufLen - emberAfCurrentCommand()->payloadStartIndex - 1;
    // Calculate the CRC over the entire packet except the last 4 (CRC) bytes.
    preCrcLength = bufferLength - sizeof(crc);
    crc = emberAfGetBufferCrc(debugReportData, preCrcLength, 0);
    readCrc = emberFetchLowHighInt32u(&debugReportData[preCrcLength]);
    if ( crc == readCrc ) {
      emberAfCorePrintln("Rx ReportId=%d, data:", debugReportId);
      emberAfCorePrintBuffer(debugReportData, bufferLength, TRUE);
      // Upon removing the print lines below the above data is not being printed
      // on the console during simulation
      emberAfCorePrintln("");
      emberAfCorePrintln("End of Data");
#ifdef EMBER_AF_PLUGIN_DIDO_STORAGE_POSIX_FILESYSTEM
      emberAfDidoStorageWriteReport(debugReportData, bufferLength);
#endif
    } else {
      emberAfCorePrintln("Error: Debug Report CRC Mismatch");
    }
  }
  return true;
}
