/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *  Contains standard enum values (non-generated)
 */
#pragma once

#include <stdint.h>

enum EmberAfStatus : uint8_t
{
    EMBER_ZCL_STATUS_SUCCESS                     = 0x00,
    EMBER_ZCL_STATUS_FAILURE                     = 0x01,
    EMBER_ZCL_STATUS_NOT_AUTHORIZED              = 0x7E,
    EMBER_ZCL_STATUS_MALFORMED_COMMAND           = 0x80,
    EMBER_ZCL_STATUS_UNSUP_COMMAND               = 0x81,
    EMBER_ZCL_STATUS_UNSUP_GENERAL_COMMAND       = 0x82,
    EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND = 0x83,
    EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND = 0x84,
    EMBER_ZCL_STATUS_INVALID_FIELD               = 0x85,
    EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE       = 0x86,
    EMBER_ZCL_STATUS_INVALID_VALUE               = 0x87,
    EMBER_ZCL_STATUS_READ_ONLY                   = 0x88,
    EMBER_ZCL_STATUS_INSUFFICIENT_SPACE          = 0x89,
    EMBER_ZCL_STATUS_DUPLICATE_EXISTS            = 0x8A,
    EMBER_ZCL_STATUS_NOT_FOUND                   = 0x8B,
    EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE      = 0x8C,
    EMBER_ZCL_STATUS_INVALID_DATA_TYPE           = 0x8D,
    EMBER_ZCL_STATUS_INVALID_SELECTOR            = 0x8E,
    EMBER_ZCL_STATUS_WRITE_ONLY                  = 0x8F,
    EMBER_ZCL_STATUS_INCONSISTENT_STARTUP_STATE  = 0x90,
    EMBER_ZCL_STATUS_DEFINED_OUT_OF_BAND         = 0x91,
    EMBER_ZCL_STATUS_ACTION_DENIED               = 0x93,
    EMBER_ZCL_STATUS_TIMEOUT                     = 0x94,
    EMBER_ZCL_STATUS_ABORT                       = 0x95,
    EMBER_ZCL_STATUS_INVALID_IMAGE               = 0x96,
    EMBER_ZCL_STATUS_WAIT_FOR_DATA               = 0x97,
    EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE          = 0x98,
    EMBER_ZCL_STATUS_REQUIRE_MORE_IMAGE          = 0x99,
    EMBER_ZCL_STATUS_NOTIFICATION_PENDING        = 0x9A,
    EMBER_ZCL_STATUS_HARDWARE_FAILURE            = 0xC0,
    EMBER_ZCL_STATUS_SOFTWARE_FAILURE            = 0xC1,
    EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER         = 0xC3,
    EMBER_ZCL_STATUS_LIMIT_REACHED               = 0xC4,
    EMBER_ZCL_STATUS_INVALID_ARGUMENT            = 0xC6,
};

typedef enum
{
    EMBER_ZCL_REPORTING_DIRECTION_REPORTED = 0x00,
    EMBER_ZCL_REPORTING_DIRECTION_RECEIVED = 0x01,
} EmberAfReportingDirection;
