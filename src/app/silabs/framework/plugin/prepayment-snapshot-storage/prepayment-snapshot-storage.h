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
 * @brief APIs and defines for the Prepayment Snapshot Storage plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PREPAYMENT_SNAPSHOT_STORAGE_H
#define SILABS_PREPAYMENT_SNAPSHOT_STORAGE_H

// 0 is technically valid, but we'll designate it to be our "invalid" value.
#define INVALID_SNAPSHOT_ID           0
#define INVALID_SNAPSHOT_SCHEDULE_ID  0

void emberAfPluginPrepaymentSnapshotStorageInitCallback(uint8_t endpoint);

uint32_t emberAfPluginPrepaymentSnapshotStorageTakeSnapshot(uint8_t endpoint, uint32_t snapshotCause);

uint8_t emberAfPluginPrepaymentServerGetSnapshotCallback(EmberNodeId nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                                         uint32_t startTime,
                                                         uint32_t endTime,
                                                         uint8_t  snapshotOffset,
                                                         uint32_t snapshotCause);

/** @brief Defined snapshot causes.
 */
enum {
  SNAPSHOT_CAUSE_GENERAL                        = ( ((uint32_t)1) << 0),
  SNAPSHOT_CAUSE_END_OF_BILLING_PERIOD          = ( ((uint32_t)1) << 1),
  SNAPSHOT_CAUSE_CHANGE_OF_TARIFF_INFO          = ( ((uint32_t)1) << 3),
  SNAPSHOT_CAUSE_CHANGE_OF_PRICE_MATRIX         = ( ((uint32_t)1) << 4),
  SNAPSHOT_CAUSE_MANUALLY_TRIGGERED_FROM_CLIENT = ( ((uint32_t)1) << 10),
  SNAPSHOT_CAUSE_CHANGE_OF_TENANCY              = ( ((uint32_t)1) << 12),
  SNAPSHOT_CAUSE_CHANGE_OF_SUPPLIER             = ( ((uint32_t)1) << 13),
  SNAPSHOT_CAUSE_CHANGE_OF_METER_MODE           = ( ((uint32_t)1) << 14),
  SNAPSHOT_CAUSE_TOP_UP_ADDITION                = ( ((uint32_t)1) << 18),
  SNAPSHOT_CAUSE_DEBT_OR_CREDIT_ADDITION        = ( ((uint32_t)1) << 19),
};
#define SNAPSHOT_CAUSE_ALL_SNAPSHOTS  0xFFFFFFFF

typedef struct {
  uint32_t snapshotId;
  uint32_t snapshotCauseBitmap;
  uint32_t snapshotTime;    // Time snapshot was taken.
  uint8_t  snapshotType;
//  EmberNodeId requestingId;

  // For now, only 1 snapshot type exists (Debt/Credit), so there is only 1 payload.
  int32_t accumulatedDebt;
  uint32_t type1DebtRemaining;
  uint32_t type2DebtRemaining;
  uint32_t type3DebtRemaining;
  int32_t emergencyCreditRemaining;
  int32_t creditRemaining;
} EmberAfPrepaymentSnapshotPayload;

typedef struct {
  // Stores fields needed to schedule a new snapshot.
  uint32_t snapshotScheduleId;
  uint32_t snapshotStartTime;
  uint32_t snapshotCauseBitmap;
  EmberNodeId requestingId;
  uint8_t srcEndpoint;
  uint8_t dstEndpoint;
  uint8_t snapshotPayloadType;
} EmberAfPrepaymentSnapshotSchedulePayload;

EmberStatus emberAfPluginPrepaymentSnapshotStoragePublishSnapshot(EmberNodeId nodeId,
                                                                  uint8_t srcEndpoint,
                                                                  uint8_t dstEndpoint,
                                                                  uint32_t snapshotTableIndex);

#endif  // #ifndef _PREPAYMENT_SNAPSHOT_STORAGE_H_
