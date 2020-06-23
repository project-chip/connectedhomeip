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
 * @brief Code to handle meter snapshot storage behavior.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "meter-snapshot-storage.h"

static EmberAfSnapshotPayload snapshots[EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY];

static EmberAfSnapshotSchedulePayload schedules[EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY];

static uint32_t snapshotIdCounter;

static void initSnapshots(void);
static void initSchedules(void);
static EmberAfSnapshotPayload *allocateSnapshot(void);
static EmberAfSnapshotSchedulePayload *allocateSchedule(void);
static void processTiers(uint8_t endpoint,
                         EmberAfSnapshotPayload *snapshot,
                         bool delivered);
static void processTiersAndBlocks(uint8_t endpoint,
                                  EmberAfSnapshotPayload *snapshot,
                                  bool delivered);
static EmberAfSnapshotPayload *findSnapshot(uint32_t startTime,
                                            uint32_t endTime,
                                            uint8_t snapshotOffset,
                                            uint32_t snapshotCause);
static uint16_t fillPayloadBuffer(uint8_t *buffer, EmberAfSnapshotPayload *payload);

static void initSnapshots(void)
{
  uint8_t i;

  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++) {
    snapshots[i].snapshotId = INVALID_SNAPSHOT_ID;
  }
}

static void initSchedules(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY; i++) {
    schedules[i].snapshotScheduleId = INVALID_SNAPSHOT_SCHEDULE_ID;
  }
}

static EmberAfSnapshotPayload *allocateSnapshot(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++) {
    if (snapshots[i].snapshotId == INVALID_SNAPSHOT_ID) {
      return &snapshots[i];
    }
  }

  return NULL;
}

static EmberAfSnapshotSchedulePayload *allocateSchedule(void)
{
  uint8_t i;

  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY; i++) {
    if (schedules[i].snapshotScheduleId == INVALID_SNAPSHOT_SCHEDULE_ID) {
      return &schedules[i];
    }
  }

  return NULL;
}

void emberAfPluginMeterSnapshotStorageInitCallback(uint8_t endpoint)
{
  snapshotIdCounter = INVALID_SNAPSHOT_ID;
  initSchedules();
  initSnapshots();
}

void emberAfPluginMeterSnapshotServerScheduleSnapshotCallback(uint8_t srcEndpoint,
                                                              uint8_t dstEndpoint,
                                                              EmberNodeId dest,
                                                              uint8_t *snapshotPayload,
                                                              uint8_t *responsePayload)
{
  EmberAfSnapshotSchedulePayload *schedule;
  uint8_t index = 0;
  EmberAfSnapshotScheduleConfirmation confirmation = EMBER_ZCL_SNAPSHOT_SCHEDULE_CONFIRMATION_ACCEPTED;

  // Attempt to allocate a schedule
  schedule = allocateSchedule();

  if (schedule == NULL) {
    confirmation = EMBER_ZCL_SNAPSHOT_SCHEDULE_CONFIRMATION_INSUFFICIENT_SPACE_FOR_SNAPSHOT_SCHEDULE;
    goto kickout;
  }

  // Set the schedule
  schedule->snapshotScheduleId = snapshotPayload[index];
  if (schedule->snapshotScheduleId == INVALID_SNAPSHOT_SCHEDULE_ID) {
    confirmation = EMBER_ZCL_SNAPSHOT_SCHEDULE_CONFIRMATION_SNAPSHOT_SCHEDULE_NOT_CURRENTLY_AVAILABLE;
    goto kickout;
  }
  index++;

  schedule->snapshotStartDate = emberAfGetInt32u(snapshotPayload,
                                                 index,
                                                 SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);
  index += 4;
  schedule->snapshotSchedule = emberAfGetInt24u(snapshotPayload,
                                                index,
                                                SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);
  index += 3;
  schedule->snapshotPayloadType = emberAfGetInt8u(snapshotPayload,
                                                  index,
                                                  SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);
  index++;
  schedule->snapshotCause = emberAfGetInt32u(snapshotPayload,
                                             index,
                                             SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);

  // Log where to send the snapshot(s) when the time is right
  schedule->requestingId = dest;
  schedule->srcEndpoint = srcEndpoint;
  schedule->dstEndpoint = dstEndpoint;

  kickout:
  // Fill the response payload
  if (schedule == NULL) {
    responsePayload[0] = INVALID_SNAPSHOT_SCHEDULE_ID;
  } else {
    responsePayload[0] = schedule->snapshotScheduleId;
  }
  responsePayload[1] = confirmation;
}

uint32_t emberAfPluginMeterSnapshotServerTakeSnapshotCallback(uint8_t endpoint,
                                                              uint32_t snapshotCause,
                                                              uint8_t *snapshotConfirmation)
{
  EmberAfSnapshotPayload *snapshot;
  uint32_t snapshotId = INVALID_SNAPSHOT_ID;
  uint8_t manualType = EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_MANUAL_SNAPSHOT_TYPE;
  bool delivered = (manualType == 0 || manualType == 2);

  // Attempt to allocate a snapshot
  snapshot = allocateSnapshot();

  if (snapshot == NULL) {
    // FAIL
    goto kickout;
  }

  // Set up snapshot identification information
  snapshot->snapshotId = ++snapshotIdCounter;

  if (snapshot->snapshotId == INVALID_SNAPSHOT_ID) {
    *snapshotConfirmation = 0x01;
    goto kickout;
  }

  snapshotId = snapshot->snapshotId;

  snapshot->payloadType = manualType;
  snapshot->tiersInUse = SUMMATION_TIERS;
  snapshot->tiersAndBlocksInUse = BLOCK_TIERS;
  snapshot->snapshotTime = emberAfGetCurrentTime();
  snapshot->snapshotCause = snapshotCause | SNAPSHOT_CAUSE_MANUAL;
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID
                        : ZCL_CURRENT_SUMMATION_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (uint8_t *)&(snapshot->currentSummation),
                       sizeof(snapshot->currentSummation),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_BILL_TO_DATE_DELIVERED_ATTRIBUTE_ID
                        : ZCL_BILL_TO_DATE_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (uint8_t *)&(snapshot->billToDate),
                       sizeof(snapshot->billToDate),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_BILL_TO_DATE_TIME_STAMP_DELIVERED_ATTRIBUTE_ID
                        : ZCL_BILL_TO_DATE_TIME_STAMP_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (uint8_t *)&(snapshot->billToDateTimeStamp),
                       sizeof(snapshot->billToDateTimeStamp),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_PROJECTED_BILL_DELIVERED_ATTRIBUTE_ID
                        : ZCL_PROJECTED_BILL_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (uint8_t *)&(snapshot->projectedBill),
                       sizeof(snapshot->projectedBill),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_PROJECTED_BILL_TIME_STAMP_DELIVERED_ATTRIBUTE_ID
                        : ZCL_PROJECTED_BILL_TIME_STAMP_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (uint8_t *)&(snapshot->projectedBillTimeStamp),
                       sizeof(snapshot->projectedBillTimeStamp),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_BILL_DELIVERED_TRAILING_DIGIT_ATTRIBUTE_ID
                        : ZCL_BILL_RECEIVED_TRAILING_DIGIT_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (uint8_t *)&(snapshot->billTrailingDigit),
                       sizeof(snapshot->billTrailingDigit),
                       NULL);

  processTiers(endpoint, snapshot, delivered);
  processTiersAndBlocks(endpoint, snapshot, delivered);

  *snapshotConfirmation = 0x00;
  kickout:
  return snapshotId;
}

void emberAfPluginMeterSnapshotServerGetSnapshotCallback(uint8_t srcEndpoint,
                                                         uint8_t dstEndpoint,
                                                         EmberNodeId dest,
                                                         uint8_t *snapshotCriteria)
{
  uint8_t snapshotPayload[SNAPSHOT_PAYLOAD_SIZE];
  uint16_t payloadSize;
  EmberAfSnapshotPayload* payload;
  uint32_t startTime = emberAfGetInt32u(snapshotCriteria, 0, 13);
  uint32_t endTime = emberAfGetInt32u(snapshotCriteria, 4, 13);
  uint8_t snapshotOffset = emberAfGetInt8u(snapshotCriteria, 8, 13);
  uint32_t snapshotCause = emberAfGetInt32u(snapshotCriteria, 9, 13);

  emberAfCorePrintln("Start Time %u snapshot Offset %u SnapShotCause %u", startTime, snapshotOffset, snapshotCause);

  // Find the snapshot
  payload = findSnapshot(startTime,
                         endTime,
                         snapshotOffset,
                         snapshotCause);
  if (payload == NULL) {
    EmberAfClusterCommand const * const cmd = emberAfCurrentCommand();
    emberAfCorePrintln("No snapshot matches specified criteria");
    if (cmd != NULL) {
      // Send default response only if this is a response to a snapshot request.
      // Not if this is an unsolicited command (e.g. from a CLI).
      // Using emberAfSendDefaultResponseWithCallback() instead of
      // emberAfSendImmediateDefaultResponse() because the latter expands to
      // former anyway and calls emberAfCurrentCommand(). This way we have to
      // call emberAfCurrentCommand() only once.
      emberAfSendDefaultResponseWithCallback(emberAfCurrentCommand(),
                                             EMBER_ZCL_STATUS_NOT_FOUND,
                                             NULL);
    }
    return;
  }

  // Fill our payload buffer
  payloadSize = fillPayloadBuffer(snapshotPayload, payload);

  emberAfFillCommandSimpleMeteringClusterPublishSnapshot(payload->snapshotId,
                                                         payload->snapshotTime,
                                                         1,
                                                         0,
                                                         1,
                                                         payload->snapshotCause,
                                                         payload->payloadType,
                                                         (uint8_t *)payload,
                                                         payloadSize);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dest);
}

static void processTiers(uint8_t endpoint,
                         EmberAfSnapshotPayload *snapshot,
                         bool delivered)
{
  uint16_t i, attr = (delivered
                      ? ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID
                      : ZCL_CURRENT_TIER1_SUMMATION_RECEIVED_ATTRIBUTE_ID);

  for (i = 0; i < SUMMATION_TIERS; i++) {
    emberAfReadAttribute(endpoint,
                         ZCL_SIMPLE_METERING_CLUSTER_ID,
                         attr + i,
                         CLUSTER_MASK_SERVER,
                         (uint8_t *)(&(snapshot->tierSummation[i * 6])),
                         6,
                         NULL);
  }
}

static void processTiersAndBlocks(uint8_t endpoint,
                                  EmberAfSnapshotPayload *snapshot,
                                  bool delivered)
{
  uint16_t i, attr = (delivered
                      ? ZCL_CURRENT_TIER1_BLOCK1_SUMMATION_DELIVERED_ATTRIBUTE_ID
                      : ZCL_CURRENT_TIER1_BLOCK1_SUMMATION_RECEIVED_ATTRIBUTE_ID);

  for (i = 0; i < BLOCK_TIERS; i++) {
    emberAfReadAttribute(endpoint,
                         ZCL_SIMPLE_METERING_CLUSTER_ID,
                         attr + i,
                         CLUSTER_MASK_SERVER,
                         (uint8_t *)(&(snapshot->tierBlockSummation[i * 6])),
                         6,
                         NULL);
  }
}

static EmberAfSnapshotPayload *findSnapshot(uint32_t startTime,
                                            uint32_t endTime,
                                            uint8_t snapshotOffset,
                                            uint32_t snapshotCause)
{
  uint8_t i, offsetCount = snapshotOffset;
  EmberAfSnapshotPayload *snapshot;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++) {
    snapshot = &(snapshots[i]);
    if (snapshot->snapshotTime >= startTime && snapshot->snapshotTime <= endTime) {
      if ((snapshot->snapshotCause & snapshotCause)) {
        if (offsetCount == 0) {
          return &(snapshots[i]);
        }
        offsetCount--;
      }
    }
  }

  return NULL;
}

static uint16_t fillPayloadBuffer(uint8_t *buffer, EmberAfSnapshotPayload *payload)
{
  uint16_t index = 0;
  uint16_t i;
  bool block = (payload->payloadType == 2 || payload->payloadType == 3);

  MEMMOVE(buffer, payload->currentSummation, 6);
  index += 6;

  emberAfCopyInt32u(buffer, index, payload->billToDate);
  index += 4;

  emberAfCopyInt32u(buffer, index, payload->billToDateTimeStamp);
  index += 4;

  emberAfCopyInt32u(buffer, index, payload->projectedBill);
  index += 4;

  emberAfCopyInt32u(buffer, index, payload->projectedBillTimeStamp);
  index += 4;

  emberAfCopyInt8u(buffer, index, payload->billTrailingDigit);
  index++;

  emberAfCopyInt8u(buffer, index, payload->tiersInUse);
  index++;

  for (i = 0; i < SUMMATION_TIERS; i++) {
    MEMMOVE((buffer + index), &(payload->tierSummation[i * 6]), 6);
    index += 6;
  }

  // Our payload is bigger if we're operating on the block information set
  if (block) {
    emberAfCopyInt8u(buffer, index, payload->tiersAndBlocksInUse);
    index++;

    for (i = 0; i < BLOCK_TIERS; i++) {
      MEMMOVE((buffer + index), &(payload->tierBlockSummation[i * 6]), 6);
      index += 6;
    }
  }

  return index;
}
