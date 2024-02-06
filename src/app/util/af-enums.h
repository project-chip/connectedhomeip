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
    MATTER_CL_STATUS_SUCCESS                  = 0x00,
    MATTER_CL_STATUS_FAILURE                  = 0x01,
    MATTER_CL_STATUS_INVALID_SUBSCRIPTION     = 0x7D,
    MATTER_CL_STATUS_UNSUPPORTED_ACCESS       = 0x7E,
    MATTER_CL_STATUS_UNSUPPORTED_ENDPOINT     = 0x7F,
    MATTER_CL_STATUS_MALFORMED_COMMAND        = 0x80,
    MATTER_CL_STATUS_UNSUPPORTED_COMMAND      = 0x81,
    MATTER_CL_STATUS_DEPRECATED82             = 0x82,
    MATTER_CL_STATUS_DEPRECATED83             = 0x83,
    MATTER_CL_STATUS_DEPRECATED84             = 0x84,
    MATTER_CL_STATUS_INVALID_COMMAND          = 0x85,
    MATTER_CL_STATUS_UNSUPPORTED_ATTRIBUTE    = 0x86,
    MATTER_CL_STATUS_CONSTRAINT_ERROR         = 0x87,
    MATTER_CL_STATUS_UNSUPPORTED_WRITE        = 0x88,
    MATTER_CL_STATUS_RESOURCE_EXHAUSTED       = 0x89,
    MATTER_CL_STATUS_DUPLICATE_EXISTS         = 0x8A,
    MATTER_CL_STATUS_NOT_FOUND                = 0x8B,
    MATTER_CL_STATUS_UNREPORTABLE_ATTRIBUTE   = 0x8C,
    MATTER_CL_STATUS_INVALID_DATA_TYPE        = 0x8D,
    MATTER_CL_STATUS_DEPRECATED8E             = 0x8E,
    MATTER_CL_STATUS_UNSUPPORTED_READ         = 0x8F,
    MATTER_CL_STATUS_DEPRECATED90             = 0x90,
    MATTER_CL_STATUS_DEPRECATED91             = 0x91,
    MATTER_CL_STATUS_DATA_VERSION_MISMATCH    = 0x92,
    MATTER_CL_STATUS_DEPRECATED93             = 0x93,
    MATTER_CL_STATUS_TIMEOUT                  = 0x94,
    MATTER_CL_STATUS_RESERVED95               = 0x95,
    MATTER_CL_STATUS_RESERVED96               = 0x96,
    MATTER_CL_STATUS_RESERVED97               = 0x97,
    MATTER_CL_STATUS_RESERVED98               = 0x98,
    MATTER_CL_STATUS_RESERVED99               = 0x99,
    MATTER_CL_STATUS_RESERVED9A               = 0x9A,
    MATTER_CL_STATUS_BUSY                     = 0x9C,
    MATTER_CL_STATUS_DEPRECATEDC0             = 0xC0,
    MATTER_CL_STATUS_DEPRECATEDC1             = 0xC1,
    MATTER_CL_STATUS_UNSUPPORTED_CLUSTER      = 0xC3,
    MATTER_CL_STATUS_DEPRECATEDC4             = 0xC4,
    MATTER_CL_STATUS_NO_UPSTREAM_SUBSCRIPTION = 0xC5,
    MATTER_CL_STATUS_NEEDS_TIMED_INTERACTION  = 0xC6,
    MATTER_CL_STATUS_UNSUPPORTED_EVENT        = 0xC7,
    MATTER_CL_STATUS_PATHS_EXHAUSTED          = 0xC8,
    MATTER_CL_STATUS_TIMED_REQUEST_MISMATCH   = 0xC9,
    MATTER_CL_STATUS_FAILSAFE_REQUIRED        = 0xCA,
    MATTER_CL_STATUS_INVALID_IN_STATE         = 0xCB,
    MATTER_CL_STATUS_NO_COMMAND_RESPONSE      = 0xCC,
    MATTER_CL_STATUS_WRITE_IGNORED            = 0xF0, // NOT SPEC COMPLIANT FOR TEST ONLY
};
