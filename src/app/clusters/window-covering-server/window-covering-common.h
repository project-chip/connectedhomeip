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

/****************************************************************************
 * @file
 * @brief Routines for the Window Covering Common between Client/Server cluster
 *******************************************************************************
 ******************************************************************************/


#pragma once

#include <app/util/af-enums.h>
#include <app/common/gen/enums.h>


/* Boundaries Checker */
#define CHECK_BOUNDS_INVALID(MIN, VAL, MAX) ((VAL < MIN) || (VAL > MAX))
#define CHECK_BOUNDS_VALID(MIN, VAL, MAX)   (!CHECK_BOUNDS_INVALID(MIN, VAL, MAX))

#define WC_PERCENTAGE_COEF   100         // Percentage Coef
#define WC_PERCENT100THS_MAX 10000       // Transfer Coef
#define WC_DEFAULT_EP        1           // EndPoint default

typedef uint16_t posPercent100ths_t;

typedef struct Mode {
    uint8_t motorDirReversed        : 1; // bit 0
    uint8_t calibrationMode         : 1; // bit 1
    uint8_t maintenanceMode         : 1; // bit 2
    uint8_t ledDisplay              : 1; // bit 3
} Mode_t;
static_assert (sizeof(Mode_t) == sizeof(uint8_t), "Mode_t Size is not correct");

typedef struct ConfigStatus {
    uint8_t operational             : 1; // bit 0 M
    uint8_t online                  : 1; // bit 1 M
    uint8_t liftIsReversed          : 1; // bit 2 LF
    uint8_t liftIsPA                : 1; // bit 3 LF & PA
    uint8_t tiltIsPA                : 1; // bit 4 TL & PA
    uint8_t liftIsEncoderControlled : 1; // bit 5 LF & PA
    uint8_t tiltIsEncoderControlled : 1; // bit 6 LF & PA
} ConfigStatus_t;
static_assert (sizeof(ConfigStatus_t) == sizeof(uint8_t), "ConfigStatus_t Size is not correct");

// Match directly with OperationalStatus 2 bits Fields
typedef enum OperationalState {
    Stall             = 0x00,            // currently not moving
    MovingUpOrOpen    = 0x01,            // is currently opening
    MovingDownOrClose = 0x02,            // is currently closing
    Reserved          = 0x03,            // dont use
} __attribute__((__packed__)) OperationalState_e;
static_assert (sizeof(OperationalState_e) == sizeof(uint8_t), "OperationalState_e Size is not correct");

typedef struct OperationalStatus {
    OperationalState_e global       : 2; // bit 0-1 M
    OperationalState_e lift         : 2; // bit 2-3 LF
    OperationalState_e tilt         : 2; // bit 4-5 TL
} OperationalStatus_t;
static_assert (sizeof(OperationalStatus_t) == sizeof(uint8_t), "OperationalStatus_t Size is not correct");

typedef struct SafetyStatus {
    uint16_t remoteLockout          : 1; // bit 0
    uint16_t tamperDetection        : 1; // bit 1
    uint16_t failedCommunication    : 1; // bit 2
    uint16_t positionFailure        : 1; // bit 3
    uint16_t thermalProtection      : 1; // bit 4
    uint16_t obstacleDetected       : 1; // bit 5
    uint16_t powerIssue             : 1; // bit 6
    uint16_t stopInput              : 1; // bit 7
    uint16_t motorJammed            : 1; // bit 8
    uint16_t hardwareFailure        : 1; // bit 9
    uint16_t manualOperation        : 1; // bit 10
} SafetyStatus_t;
static_assert (sizeof(SafetyStatus_t) == sizeof(uint16_t), "SafetyStatus_t Size is not correct");

/* Values Abs <-> Rel Converting functions */
posPercent100ths_t wcAbsPositionToRelPercent100ths(uint16_t openLimit, uint16_t closedLimit, uint16_t position);
uint16_t           wcRelPercent100thsToAbsPosition(uint16_t openLimit, uint16_t closedLimit, posPercent100ths_t percent100ths);
