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
 * @brief APIs and defines for the Meter Snapshot Storage plugin, which handles
 *        snapshots of meter states in a network.
 *******************************************************************************
   ******************************************************************************/

#define INVALID_SNAPSHOT_SCHEDULE_ID 0

// 0 is technically valid, but we'll designate it to be our "invalid" id for init purposes
#define INVALID_SNAPSHOT_ID 0

#define SNAPSHOT_SCHEDULE_PAYLOAD_SIZE 15

#define SNAPSHOT_CAUSE_MANUAL 0x00000400

#define SUMMATION_TIERS EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SUM_TIERS_SUPPORTED
#define BLOCK_TIERS  EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_BLOCK_TIERS_SUPPORTED
#define SNAPSHOT_PAYLOAD_SIZE SUMMATION_TIERS + BLOCK_TIERS + 94

typedef struct {
  uint8_t tierSummation[SUMMATION_TIERS * 6];
  uint8_t tierBlockSummation[BLOCK_TIERS * 6];
  uint8_t currentSummation[6];
  uint32_t billToDate;
  uint32_t billToDateTimeStamp;
  uint32_t projectedBill;
  uint32_t projectedBillTimeStamp;
  uint32_t snapshotId;
  uint32_t snapshotTime;
  uint32_t snapshotCause;
  EmberNodeId requestingId;
  uint8_t tiersInUse;
  uint8_t tiersAndBlocksInUse;
  uint8_t srcEndpoint;
  uint8_t dstEndpoint;
  uint8_t billTrailingDigit;
  uint8_t payloadType;
} EmberAfSnapshotPayload;

typedef struct {
  uint32_t snapshotStartDate;
  uint32_t snapshotSchedule;
  uint32_t snapshotCause;
  EmberNodeId requestingId;
  uint8_t srcEndpoint;
  uint8_t dstEndpoint;
  uint8_t snapshotPayloadType;
  uint8_t snapshotScheduleId;
} EmberAfSnapshotSchedulePayload;
