/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

/******************************************************************************

 @file dmm_priority_ble_thread.c

 @brief Dual Mode Manager Global Priority for OpenThread Stack and BLE Stack
 *****************************************************************************/

#include <dmm_policy.h>
#include <dmm_priority_ble_matter.h>

/* BLE Activity */
typedef enum
{
    DMM_BLE_CONNECTION   = 0x07D0,
    DMM_BLE_CON_EST      = 0x03E8,
    DMM_BLE_BROADCASTING = 0x0BB8,
    DMM_BLE_OBSERVING    = 0x0FA0,
} DMMStackActivityBLE;

/* Thread Activity */
typedef enum
{
    DMM_THREAD_TX_DATA = 0x0001,
    DMM_THREAD_TX_POLL = 0x0002,
    DMM_THREAD_RX_POLL = 0x0003,
    DMM_THREAD_RX_SCAN = 0x0004,
    DMM_THREAD_RX_IDLE = 0x0005
} DMMStackActivityThread;

/* Global Priority Table: BLE connection is lower than Thread data */
StackActivity activityBLE_bleLthreadH[ACTIVITY_NUM_BLE * PRIORITY_NUM] = {
    DMM_GLOBAL_PRIORITY(DMM_BLE_CONNECTION, DMM_StackPNormal, 70),
    DMM_GLOBAL_PRIORITY(DMM_BLE_CONNECTION, DMM_StackPHigh, 170),
    DMM_GLOBAL_PRIORITY(DMM_BLE_CONNECTION, DMM_StackPUrgent, 250),

    DMM_GLOBAL_PRIORITY(DMM_BLE_CON_EST, DMM_StackPNormal, 70),
    DMM_GLOBAL_PRIORITY(DMM_BLE_CON_EST, DMM_StackPHigh, 195),
    DMM_GLOBAL_PRIORITY(DMM_BLE_CON_EST, DMM_StackPUrgent, 220),

    DMM_GLOBAL_PRIORITY(DMM_BLE_BROADCASTING, DMM_StackPNormal, 60),
    DMM_GLOBAL_PRIORITY(DMM_BLE_BROADCASTING, DMM_StackPHigh, 160),
    DMM_GLOBAL_PRIORITY(DMM_BLE_BROADCASTING, DMM_StackPUrgent, 210),

    DMM_GLOBAL_PRIORITY(DMM_BLE_OBSERVING, DMM_StackPNormal, 60),
    DMM_GLOBAL_PRIORITY(DMM_BLE_OBSERVING, DMM_StackPHigh, 160),
    DMM_GLOBAL_PRIORITY(DMM_BLE_OBSERVING, DMM_StackPUrgent, 210),
};

StackActivity activityThread_bleLthreadH[ACTIVITY_NUM_THREAD * PRIORITY_NUM] = {
    DMM_GLOBAL_PRIORITY(DMM_THREAD_TX_DATA, DMM_StackPNormal, 80),  DMM_GLOBAL_PRIORITY(DMM_THREAD_TX_DATA, DMM_StackPHigh, 180),
    DMM_GLOBAL_PRIORITY(DMM_THREAD_TX_DATA, DMM_StackPUrgent, 240),

    DMM_GLOBAL_PRIORITY(DMM_THREAD_TX_POLL, DMM_StackPNormal, 90),  DMM_GLOBAL_PRIORITY(DMM_THREAD_TX_POLL, DMM_StackPHigh, 190),
    DMM_GLOBAL_PRIORITY(DMM_THREAD_TX_POLL, DMM_StackPUrgent, 215),

    DMM_GLOBAL_PRIORITY(DMM_THREAD_RX_POLL, DMM_StackPNormal, 90),  DMM_GLOBAL_PRIORITY(DMM_THREAD_RX_POLL, DMM_StackPHigh, 190),
    DMM_GLOBAL_PRIORITY(DMM_THREAD_RX_POLL, DMM_StackPUrgent, 215),

    DMM_GLOBAL_PRIORITY(DMM_THREAD_RX_SCAN, DMM_StackPNormal, 90),  DMM_GLOBAL_PRIORITY(DMM_THREAD_RX_SCAN, DMM_StackPHigh, 190),
    DMM_GLOBAL_PRIORITY(DMM_THREAD_RX_SCAN, DMM_StackPUrgent, 215),

    DMM_GLOBAL_PRIORITY(DMM_THREAD_RX_IDLE, DMM_StackPNormal, 80),  DMM_GLOBAL_PRIORITY(DMM_THREAD_RX_IDLE, DMM_StackPHigh, 80),
    DMM_GLOBAL_PRIORITY(DMM_THREAD_RX_IDLE, DMM_StackPUrgent, 150),

};

/* the order of stacks in policy table and global table must be the same */
GlobalTable globalPriorityTable_bleLthreadH[DMMPOLICY_NUM_STACKS] = {
    {
        .globalTableArray = activityBLE_bleLthreadH,
        .tableSize        = (uint8_t) (ACTIVITY_NUM_BLE * PRIORITY_NUM),
        .stackRole        = DMMPolicy_StackRole_BlePeripheral,
    },

    {
        .globalTableArray = activityThread_bleLthreadH,
        .tableSize        = (uint8_t) (ACTIVITY_NUM_THREAD * PRIORITY_NUM),
        .stackRole        = DMMPolicy_StackRole_threadFtd,
    },
};
