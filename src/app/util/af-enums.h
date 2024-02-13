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

#include <protocols/interaction_model/StatusCode.h>
#include <stdint.h>

typedef Protocols::InteractionModel::Status EmberAfStatus;

#define EMBER_ZCL_STATUS_SUCCESS Protocols::InteractionModel::Status::Success
#define EMBER_ZCL_STATUS_FAILURE Protocols::InteractionModel::Status::Failure
#define EMBER_ZCL_STATUS_INVALID_SUBSCRIPTION Protocols::InteractionModel::Status::InvalidSubscription
#define EMBER_ZCL_STATUS_UNSUPPORTED_ACCESS Protocols::InteractionModel::Status::UnsupportedAccess
#define EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT Protocols::InteractionModel::Status::UnsupportedEndpoint
#define EMBER_ZCL_STATUS_MALFORMED_COMMAND Protocols::InteractionModel::Status::InvalidAction
#define EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND Protocols::InteractionModel::Status::UnsupportedCommand
#define EMBER_ZCL_STATUS_DEPRECATED82 Protocols::InteractionModel::Status::Deprecated82
#define EMBER_ZCL_STATUS_DEPRECATED83 Protocols::InteractionModel::Status::Deprecated83
#define EMBER_ZCL_STATUS_DEPRECATED84 Protocols::InteractionModel::Status::Deprecated84
#define EMBER_ZCL_STATUS_INVALID_COMMAND Protocols::InteractionModel::Status::InvalidCommand
#define EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE Protocols::InteractionModel::Status::UnsupportedAttribute
#define EMBER_ZCL_STATUS_CONSTRAINT_ERROR Protocols::InteractionModel::Status::ConstraintError
#define EMBER_ZCL_STATUS_UNSUPPORTED_WRITE Protocols::InteractionModel::Status::UnsupportedWrite
#define EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED Protocols::InteractionModel::Status::ResourceExhausted
#define EMBER_ZCL_STATUS_NOT_FOUND Protocols::InteractionModel::Status::NotFound
#define EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE Protocols::InteractionModel::Status::UnreportableAttribute
#define EMBER_ZCL_STATUS_INVALID_DATA_TYPE Protocols::InteractionModel::Status::InvalidDataType
#define EMBER_ZCL_STATUS_DEPRECATED8E Protocols::InteractionModel::Status::Deprecated8e
#define EMBER_ZCL_STATUS_UNSUPPORTED_READ Protocols::InteractionModel::Status::UnsupportedRead
#define EMBER_ZCL_STATUS_DEPRECATED90 Protocols::InteractionModel::Status::Deprecated90
#define EMBER_ZCL_STATUS_DEPRECATED91 Protocols::InteractionModel::Status::Deprecated91
#define EMBER_ZCL_STATUS_DATA_VERSION_MISMATCH Protocols::InteractionModel::Status::DataVersionMismatch
#define EMBER_ZCL_STATUS_DEPRECATED93 Protocols::InteractionModel::Status::Deprecated93
#define EMBER_ZCL_STATUS_TIMEOUT Protocols::InteractionModel::Status::Timeout
#define EMBER_ZCL_STATUS_RESERVED95 Protocols::InteractionModel::Status::Reserved95
#define EMBER_ZCL_STATUS_RESERVED96 Protocols::InteractionModel::Status::Reserved96
#define EMBER_ZCL_STATUS_RESERVED97 Protocols::InteractionModel::Status::Reserved97
#define EMBER_ZCL_STATUS_RESERVED98 Protocols::InteractionModel::Status::Reserved98
#define EMBER_ZCL_STATUS_RESERVED99 Protocols::InteractionModel::Status::Reserved99
#define EMBER_ZCL_STATUS_RESERVED9A Protocols::InteractionModel::Status::Reserved9a
#define EMBER_ZCL_STATUS_BUSY Protocols::InteractionModel::Status::Busy
#define EMBER_ZCL_STATUS_DEPRECATEDC0 Protocols::InteractionModel::Status::Deprecatedc0
#define EMBER_ZCL_STATUS_DEPRECATEDC1 Protocols::InteractionModel::Status::Deprecatedc1
#define EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER Protocols::InteractionModel::Status::UnsupportedCluster
#define EMBER_ZCL_STATUS_DEPRECATEDC4 Protocols::InteractionModel::Status::Deprecatedc4
#define EMBER_ZCL_STATUS_NO_UPSTREAM_SUBSCRIPTION Protocols::InteractionModel::Status::NoUpstreamSubscription
#define EMBER_ZCL_STATUS_NEEDS_TIMED_INTERACTION Protocols::InteractionModel::Status::NeedsTimedInteraction
#define EMBER_ZCL_STATUS_UNSUPPORTED_EVENT Protocols::InteractionModel::Status::UnsupportedEvent
#define EMBER_ZCL_STATUS_PATHS_EXHAUSTED Protocols::InteractionModel::Status::PathsExhausted
#define EMBER_ZCL_STATUS_TIMED_REQUEST_MISMATCH Protocols::InteractionModel::Status::TimedRequestMismatch
#define EMBER_ZCL_STATUS_FAILSAFE_REQUIRED Protocols::InteractionModel::Status::FailsafeRequired
#define EMBER_ZCL_STATUS_INVALID_IN_STATE Protocols::InteractionModel::Status::InvalidInState
#define EMBER_ZCL_STATUS_NO_COMMAND_RESPONSE Protocols::InteractionModel::Status::NoCommandResponse
#define EMBER_ZCL_STATUS_WRITE_IGNORED Protocols::InteractionModel::Status::WriteIgnored // NOT SPEC COMPLIANT FOR TEST ONLY
