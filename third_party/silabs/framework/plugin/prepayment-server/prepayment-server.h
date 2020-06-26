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
 * @brief APIs and defines for the Prepayment Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PREPAYMENT_SERVER_H
#define SILABS_PREPAYMENT_SERVER_H

// Snapshot payload type, D7.2.4.2.2 - Publish Prepay Snapshot Command
enum {
  SNAPSHOT_PAYLOAD_TYPE_DEBT_OR_CREDIT_STATUS = 0x00,
};

#define SNAPSHOT_PAYLOAD_LEN 24

/**
 * @brief Sends a Publish Prepay Snapshot command sent in response to
 * the Get Prepay Snapshot command.
 * @param nodeId The short address of the destination device.
 * @param srcEndpoint The source endpoint used in the ZigBee transmission.
 * @param dstEndpoint The destination endpoint used in the ZigBee transmission.
 * @param snapshotId A unique identifier allocated by the device that created the snapshot.
 * @param snapshotTime The UTC time when the snapshot was taken.
 * @param totalSnapshotsFound The number of snapshots that matched the criteria in the
 * received Get Prepay Snapshot command.
 * @param commandIndex Indicates a fragment number if the entire payload won't fit into 1 message.
 * @param totalNumberOfCommands The total number of subcommands that will be sent.
 * @param snapshotCause A 32-bit bitmap that indicates the cause of the snapshot.
 * @param snapshotPayloadType An 8-bit enumeration that defines the format of the snapshot payload.
 * @param snapshotPayload Data that was created with the snapshot.
 *
 **/
void emberAfPluginPrepaymentServerPublishPrepaySnapshot(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                        uint32_t snapshotId, uint32_t snapshotTime,
                                                        uint8_t totalSnapshotsFound, uint8_t commandIndex,
                                                        uint8_t totalNumberOfCommands,
                                                        uint32_t snapshotCause,
                                                        uint8_t  snapshotPayloadType,
                                                        uint8_t *snapshotPayload);

/**
 * @brief Sends a Publish Top Up Log command, sent when a top up is performed,
 * or in response to a Get Top Up Log command.
 * @param nodeId  The short address of the destination device.
 * @param srcEndpoint The source endpoint used in the ZigBee transmission.
 * @param dstEndpoint The destination endpoint used in the ZigBee transmission.
 * @param commandIndex Indicates a fragment number if the entire payload won't fit into 1 message.
 * @param totalNumberOfCommands The total number of subcommands that will be sent.
 * @param topUpPayload Information that is sent from each top up log entry.
 *
 **/
void emberAfPluginPrepaymentServerPublishTopUpLog(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                  uint8_t commandIndex, uint8_t totalNumberOfCommands,
                                                  TopUpPayload *topUpPayload);

/**
 * @brief Sends a Publish Debt Log command.
 * @param nodeId The short address of the destination device.
 * @param srcEndpoint The source endpoint used in the ZigBee transmission.
 * @param dstEndpoint The destination endpoint used in the ZigBee transmission.
 * @param commandIndex Indicates a fragment number if the entire payload won't fit into 1 message.
 * @param totalNumberOfCommands The total number of subcommands that will be sent.
 * @param debtPayload Includes the contents of a debt record from the log.
 *
 **/
void emberAfPluginPrepaymentServerPublishDebtLog(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                 uint8_t commandIndex, uint8_t totalNumberOfCommands,
                                                 DebtPayload *debtPayload);

#endif  // #ifndef _PREPAYMENT_SERVER_H_
