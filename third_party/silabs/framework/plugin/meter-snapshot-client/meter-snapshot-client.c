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
 * @brief Code to handle meter snapshot client behavior.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"

#ifndef fieldLength
  #define fieldLength(field) \
  (emberAfCurrentCommand()->bufLen - (field - emberAfCurrentCommand()->buffer))
#endif

bool emberAfSimpleMeteringClusterScheduleSnapshotResponseCallback(uint32_t issuerEventId,
                                                                  uint8_t* snapshotResponsePayload)
{
  emberAfSimpleMeteringClusterPrintln("RX: ScheduleSnapshotResponse 0x%4x",
                                      issuerEventId);

  // according to the documentation, the payload comprises of two uint8_t types
  emberAfSimpleMeteringClusterPrintln("    Payload: 0x%x, 0x%x",
                                      snapshotResponsePayload[0],
                                      snapshotResponsePayload[2]);

  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

bool emberAfSimpleMeteringClusterTakeSnapshotResponseCallback(uint32_t snapshotId,
                                                              uint8_t snapshotConfirmation)
{
  emberAfSimpleMeteringClusterPrintln("RX: TakeSnapshotResponse 0x%4x, 0x%x",
                                      snapshotId,
                                      snapshotConfirmation);

  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

bool emberAfSimpleMeteringClusterPublishSnapshotCallback(uint32_t snapshotId,
                                                         uint32_t snapshotTime,
                                                         uint8_t totalSnapshotsFound,
                                                         uint8_t commandIndex,
                                                         uint8_t totalCommands,
                                                         uint32_t snapshotCause,
                                                         uint8_t snapshotPayloadType,
                                                         uint8_t* snapshotPayload)
{
  uint8_t i;
  uint16_t snapshotPayloadLength = fieldLength(snapshotPayload);

  emberAfSimpleMeteringClusterPrintln("RX: PublishSnapshotResponse 0x%4x, 0x%4x, 0x%x, 0x%x, 0x%x, 0x%4x, 0x%x",
                                      snapshotId,
                                      snapshotTime,
                                      totalSnapshotsFound,
                                      commandIndex,
                                      totalCommands,
                                      snapshotCause,
                                      snapshotPayloadType);

  // Documentation only mentions payload type for debt/credit status, which comprises of 6 int32 types
  // so I am printing rows of four int8 values at a time, until the end of payload
  // Not passing proper payload structure may result in garbage print values
  if (snapshotPayloadType == EMBER_ZCL_PREPAY_SNAPSHOT_PAYLOAD_TYPE_DEBT_CREDIT_STATUS) {
    emberAfSimpleMeteringClusterPrintln("    Payload:");

    for (i = 0; i < snapshotPayloadLength; i += 4) {
      emberAfSimpleMeteringClusterPrintln("            0x%x 0x%x 0x%x 0x%x",
                                          snapshotPayload[i],
                                          snapshotPayload[i + 1],
                                          snapshotPayload[i + 2],
                                          snapshotPayload[i + 3]);
    }
  }

  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}
