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
 * @brief CLI for the Meter Snapshot Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#ifndef EMBER_AF_GENERATE_CLI
  #error The Meter Snapshot Server plugin is not compatible with the legacy CLI.
#endif

// plugin meter-snapshot-server take
void emAfMeterSnapshotServerCliTake(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t cause = (uint32_t)emberUnsignedCommandArgument(1);
  uint8_t snapshotConfirmation;

  // Attempt to take the snapshot
  emberAfPluginMeterSnapshotServerTakeSnapshotCallback(endpoint,
                                                       cause,
                                                       &snapshotConfirmation);
}

// plugin meter-snapshot-server publish
void emAfMeterSnapshotServerCliPublish(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  uint32_t startTime = (uint32_t)emberUnsignedCommandArgument(3);
  uint32_t endTime = (uint32_t)emberUnsignedCommandArgument(4);
  uint32_t offset = (uint8_t)emberUnsignedCommandArgument(5);
  uint32_t cause = (uint32_t)emberUnsignedCommandArgument(6);
  uint8_t snapshotCriteria[13];

  // Package the snapshot criteria for our callback to process
  emberAfCopyInt32u((uint8_t *)snapshotCriteria, 0, startTime);
  emberAfCopyInt32u((uint8_t *)snapshotCriteria, 4, endTime);
  snapshotCriteria[8] = offset;
  emberAfCopyInt32u((uint8_t *)snapshotCriteria, 9, cause);

  emberAfPluginMeterSnapshotServerGetSnapshotCallback(srcEndpoint,
                                                      dstEndpoint,
                                                      nodeId,
                                                      snapshotCriteria);
}
