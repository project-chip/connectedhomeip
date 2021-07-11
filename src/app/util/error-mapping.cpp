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

#include "error-mapping.h"

namespace chip {
namespace app {

EmberAfStatus ToEmberAfStatus(Protocols::InteractionModel::ProtocolCode code)
{
    using imcode = Protocols::InteractionModel::ProtocolCode;
    switch (code)
    {
    case imcode::Success: // 0x00
        return EMBER_ZCL_STATUS_SUCCESS;
    case imcode::Failure: // 0x01
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::InvalidSubscription: // 0x7d
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::UnsupportedAccess: // 0x7e
        return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
    case imcode::UnsupportedEndpoint: // 0x7f
        return EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
    case imcode::InvalidAction: // 0x80
        return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
    case imcode::UnsupportedCommand: // 0x81
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
    case imcode::Deprecated82: // 0x82
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
    case imcode::Deprecated83: // 0x83
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
    case imcode::Deprecated84: // 0x84
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
    case imcode::InvalidCommand: // 0x85
        return EMBER_ZCL_STATUS_INVALID_FIELD;
    case imcode::UnsupportedAttribute: // 0x86
        return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    case imcode::InvalidValue: // 0x87
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    case imcode::UnsupportedWrite: // 0x88
        return EMBER_ZCL_STATUS_READ_ONLY;
    case imcode::ResourceExhausted: // 0x89
        return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    case imcode::Deprecated8a:
        return EMBER_ZCL_STATUS_SUCCESS;
    case imcode::NotFound: // 0x8b
        return EMBER_ZCL_STATUS_NOT_FOUND;
    case imcode::UnreportableAttribute: // 0x8c
        return EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE;
    case imcode::InvalidDataType: // 0x8d
        return EMBER_ZCL_STATUS_INVALID_DATA_TYPE;
    case imcode::Deprecated8e: // 0x8e
        return EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE;
    case imcode::UnsupportedRead: // 0x8f
        return EMBER_ZCL_STATUS_WRITE_ONLY;
    case imcode::Deprecated90: // 0x90
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Deprecated91: // 0x91
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Reserved92: // 0x92
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Deprecated93: // 0x93
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Timeout: // 0x94
        return EMBER_ZCL_STATUS_TIMEOUT;
    case imcode::Reserved95: // 0x95
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Reserved96: // 0x96
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Reserved97: // 0x97
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Reserved98: // 0x98
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Reserved99: // 0x99
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Reserved9a: // 0x9a
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::ConstraintError: // 0x9b
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Busy: // 0x9c
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Deprecatedc0: // 0xc0
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Deprecatedc1: // 0xc1
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::Deprecatedc2: // 0xc2
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::UnsupportedCluster: // 0xc3
        return EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
    case imcode::Deprecatedc4: // 0xc4
        return EMBER_ZCL_STATUS_SUCCESS;
    case imcode::NoUpstreamSubscription: // 0xc5
        return EMBER_ZCL_STATUS_FAILURE;
    case imcode::InvalidArgument: // 0xc6
        return EMBER_ZCL_STATUS_INVALID_ARGUMENT;
        // Default case is omitted intentionally so we can guarantee that we exhaust all of the error codes.
    }
    return EMBER_ZCL_STATUS_FAILURE;
}

Protocols::InteractionModel::ProtocolCode ToInteractionModelProtocolCode(EmberAfStatus code)
{
    using imcode = Protocols::InteractionModel::ProtocolCode;
    switch (code)
    {
    case EMBER_ZCL_STATUS_SUCCESS: // 0x00
        return imcode::Success;
    case EMBER_ZCL_STATUS_FAILURE: // 0x01
        return imcode::Failure;
    case EMBER_ZCL_STATUS_NOT_AUTHORIZED: // 0x7E
        return imcode::UnsupportedAccess;
    case EMBER_ZCL_STATUS_MALFORMED_COMMAND: // 0x80
        return imcode::InvalidAction;
    case EMBER_ZCL_STATUS_UNSUP_COMMAND: // 0x81
        return imcode::UnsupportedCommand;
    case EMBER_ZCL_STATUS_UNSUP_GENERAL_COMMAND: // 0x82
        return imcode::UnsupportedCommand;
    case EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND: // 0x83
        return imcode::UnsupportedCommand;
    case EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND: // 0x84
        return imcode::UnsupportedCommand;
    case EMBER_ZCL_STATUS_INVALID_FIELD: // 0x85
        return imcode::InvalidCommand;
    case EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE: // 0x86
        return imcode::UnsupportedAttribute;
    case EMBER_ZCL_STATUS_INVALID_VALUE: // 0x87
        return imcode::InvalidValue;
    case EMBER_ZCL_STATUS_READ_ONLY: // 0x88
        return imcode::UnsupportedWrite;
    case EMBER_ZCL_STATUS_INSUFFICIENT_SPACE: // 0x89
        return imcode::ResourceExhausted;
    case EMBER_ZCL_STATUS_DUPLICATE_EXISTS: // 0x8A
        return imcode::Success;
    case EMBER_ZCL_STATUS_NOT_FOUND: // 0x8B
        return imcode::NotFound;
    case EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE: // 0x8C
        return imcode::UnreportableAttribute;
    case EMBER_ZCL_STATUS_INVALID_DATA_TYPE: // 0x8D
        return imcode::InvalidDataType;
    case EMBER_ZCL_STATUS_INVALID_SELECTOR: // 0x8E
        return imcode::Failure;
    case EMBER_ZCL_STATUS_WRITE_ONLY: // 0x8F
        return imcode::UnsupportedRead;
    case EMBER_ZCL_STATUS_INCONSISTENT_STARTUP_STATE: // 0x90
        return imcode::Failure;
    case EMBER_ZCL_STATUS_DEFINED_OUT_OF_BAND: // 0x91
        return imcode::Failure;
    case EMBER_ZCL_STATUS_ACTION_DENIED: // 0x93
        return imcode::Failure;
    case EMBER_ZCL_STATUS_TIMEOUT: // 0x94
        return imcode::Timeout;
    case EMBER_ZCL_STATUS_ABORT: // 0x95
        return imcode::Failure;
    case EMBER_ZCL_STATUS_INVALID_IMAGE: // 0x96
        return imcode::Failure;
    case EMBER_ZCL_STATUS_WAIT_FOR_DATA: // 0x97
        return imcode::Failure;
    case EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE: // 0x98
        return imcode::Failure;
    case EMBER_ZCL_STATUS_REQUIRE_MORE_IMAGE: // 0x99
        return imcode::Failure;
    case EMBER_ZCL_STATUS_NOTIFICATION_PENDING: // 0x9A
        return imcode::Failure;
    case EMBER_ZCL_STATUS_HARDWARE_FAILURE: // 0xC0
        return imcode::Failure;
    case EMBER_ZCL_STATUS_SOFTWARE_FAILURE: // 0xC1
        return imcode::Failure;
    case EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER: // 0xC3
        return imcode::UnsupportedCluster;
    case EMBER_ZCL_STATUS_LIMIT_REACHED: // 0xC4
        return imcode::Success;
    case EMBER_ZCL_STATUS_INVALID_ARGUMENT: // 0xC6
        return imcode::InvalidArgument;
    default:
        return imcode::Failure;
    }
}

} // namespace app
} // namespace chip
