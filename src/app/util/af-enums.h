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
    EMBER_ZCL_STATUS_SUCCESS                  = 0x00,
    EMBER_ZCL_STATUS_FAILURE                  = 0x01,
    EMBER_ZCL_STATUS_INVALID_SUBSCRIPTION     = 0x7D,
    EMBER_ZCL_STATUS_UNSUPPORTED_ACCESS       = 0x7E,
    EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT     = 0x7F,
    EMBER_ZCL_STATUS_MALFORMED_COMMAND        = 0x80,
    EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND      = 0x81,
    EMBER_ZCL_STATUS_DEPRECATED82             = 0x82,
    EMBER_ZCL_STATUS_DEPRECATED83             = 0x83,
    EMBER_ZCL_STATUS_DEPRECATED84             = 0x84,
    EMBER_ZCL_STATUS_INVALID_COMMAND          = 0x85,
    EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE    = 0x86,
    EMBER_ZCL_STATUS_CONSTRAINT_ERROR         = 0x87,
    EMBER_ZCL_STATUS_UNSUPPORTED_WRITE        = 0x88,
    EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED       = 0x89,
    EMBER_ZCL_STATUS_DUPLICATE_EXISTS         = 0x8A,
    EMBER_ZCL_STATUS_NOT_FOUND                = 0x8B,
    EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE   = 0x8C,
    EMBER_ZCL_STATUS_INVALID_DATA_TYPE        = 0x8D,
    EMBER_ZCL_STATUS_DEPRECATED8E             = 0x8E,
    EMBER_ZCL_STATUS_UNSUPPORTED_READ         = 0x8F,
    EMBER_ZCL_STATUS_DEPRECATED90             = 0x90,
    EMBER_ZCL_STATUS_DEPRECATED91             = 0x91,
    EMBER_ZCL_STATUS_DATA_VERSION_MISMATCH    = 0x92,
    EMBER_ZCL_STATUS_DEPRECATED93             = 0x93,
    EMBER_ZCL_STATUS_TIMEOUT                  = 0x94,
    EMBER_ZCL_STATUS_RESERVED95               = 0x95,
    EMBER_ZCL_STATUS_RESERVED96               = 0x96,
    EMBER_ZCL_STATUS_RESERVED97               = 0x97,
    EMBER_ZCL_STATUS_RESERVED98               = 0x98,
    EMBER_ZCL_STATUS_RESERVED99               = 0x99,
    EMBER_ZCL_STATUS_RESERVED9A               = 0x9A,
    EMBER_ZCL_STATUS_BUSY                     = 0x9C,
    EMBER_ZCL_STATUS_DEPRECATEDC0             = 0xC0,
    EMBER_ZCL_STATUS_DEPRECATEDC1             = 0xC1,
    EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER      = 0xC3,
    EMBER_ZCL_STATUS_DEPRECATEDC4             = 0xC4,
    EMBER_ZCL_STATUS_NO_UPSTREAM_SUBSCRIPTION = 0xC5,
    EMBER_ZCL_STATUS_NEEDS_TIMED_INTERACTION  = 0xC6,
    EMBER_ZCL_STATUS_UNSUPPORTED_EVENT        = 0xC7,
    EMBER_ZCL_STATUS_PATHS_EXHAUSTED          = 0xC8,
    EMBER_ZCL_STATUS_TIMED_REQUEST_MISMATCH   = 0xC9,
    EMBER_ZCL_STATUS_FAILSAFE_REQUIRED        = 0xCA,
    EMBER_ZCL_STATUS_WRITE_IGNORED            = 0xF0, // NOT SPEC COMPLIANT FOR TEST ONLY
};
