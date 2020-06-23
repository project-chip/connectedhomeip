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
 * @brief Code to handle meter snapshot server behavior.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"

bool emberAfSimpleMeteringClusterScheduleSnapshotCallback(uint32_t issuerEventId,
                                                          uint8_t commandIndex,
                                                          uint8_t commandCount,
                                                          uint8_t* snapshotSchedulePayload)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  uint8_t responsePayload[2];

  // Attempt to schedule the snapshot
  emberAfPluginMeterSnapshotServerScheduleSnapshotCallback(cmd->apsFrame->destinationEndpoint,
                                                           cmd->apsFrame->sourceEndpoint,
                                                           cmd->source,
                                                           snapshotSchedulePayload,
                                                           (uint8_t *)responsePayload);

  emberAfFillCommandSimpleMeteringClusterScheduleSnapshotResponse(issuerEventId,
                                                                  responsePayload,
                                                                  2);

  emberAfSendResponse();
  return true;
}

bool emberAfSimpleMeteringClusterTakeSnapshotCallback(uint32_t snapshotCause)
{
  uint8_t endpoint = emberAfCurrentEndpoint();
  uint8_t snapshotConfirmation;
  uint32_t snapshotId;

  // Attempt to take the snapshot
  snapshotId = emberAfPluginMeterSnapshotServerTakeSnapshotCallback(endpoint,
                                                                    snapshotCause,
                                                                    &snapshotConfirmation);

  emberAfFillCommandSimpleMeteringClusterTakeSnapshotResponse(snapshotId,
                                                              snapshotConfirmation);
  emberAfSendResponse();
  return true;
}

bool emberAfSimpleMeteringClusterGetSnapshotCallback(uint32_t startTime,
                                                     uint32_t latestEndTime,
                                                     uint8_t numberOfSnapshots,
                                                     uint32_t snapshotCause)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  uint8_t snapshotCriteria[13];

  // Package the snapshot criteria for our callback to process
  emberAfCopyInt32u((uint8_t *)snapshotCriteria, 0, startTime);
  emberAfCopyInt32u((uint8_t *)snapshotCriteria, 4, latestEndTime);
  snapshotCriteria[8] = numberOfSnapshots;
  emberAfCopyInt32u((uint8_t *)snapshotCriteria, 9, snapshotCause);
  emberAfCorePrintln("snapshotCause %u", snapshotCause);
  emberAfCorePrintln("snapshotCause %u", snapshotCause);

  emberAfCorePrintln("Start Time %u Endpoint %u snapshot Offset %u SnapShotCause %u", startTime, latestEndTime, numberOfSnapshots, snapshotCause);
  // Get / publish the snapshot
  emberAfPluginMeterSnapshotServerGetSnapshotCallback(cmd->apsFrame->destinationEndpoint,
                                                      cmd->apsFrame->sourceEndpoint,
                                                      cmd->source,
                                                      (uint8_t *)snapshotCriteria);

  return true;
}
