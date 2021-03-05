/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *  @file
 *
 *  @brief
 *    API definitions for Chip Event Logging
 *
 *  This file contains the definitions for the Chip Event Logging.
 *  The file defines the API for configuring and controlling the
 *  logging subsystem as well as the API for emitting the individual
 *  log entries.
 */

#pragma once

#include "EventLoggingDelegate.h"
#include "EventLoggingTypes.h"

namespace chip {
namespace app {
namespace reporting {
/**
 * @brief
 *  Log an event via a delegate, with options.
 * The EventLoggingDelegate writes the event metadata and calls the `apDelegate`
 * with an chip::TLV::TLVWriter reference so that the user code can emit
 * the event data directly into the event log.  This form of event
 * logging minimizes memory consumption, as event data is serialized
 * directly into the target buffer.  The event data MUST contain
 * context tags to be interpreted within the schema identified by
 * `ClusterID` and `EventId`. The tag of the first element will be
 * ignored; the event logging system will replace it with the
 * eventData tag.
 *
 * The event is logged if its aPriority exceeds the logging
 * threshold specified in the LoggingConfiguration.  If the event's
 * priority does not meet the current threshold, it is dropped and
 * the function returns a `0` as the resulting event Number.
 *
 * This variant of the invocation permits the caller to set any
 * combination of `EventOptions`:
 * - timestamp, when 0 defaults to the current time at the point of
 *   the call,
 * - "root" section of the event source (event source and cluster ID);
 *   if NULL, it defaults to the current device. the event is marked as
 *   relating to the device that is making the call,
 * - urgency; by default non-urgent.
 *
 *
 * @param[in] apDelegate  The EventLoggingDelegate that is used to
 *                         serialize the event data
 *
 * @param[out] aEventNumber The event Number if the event was written to the
 *                         log, 0 otherwise.
 *
 * @return CHIP_ERROR  CHIP Error Code
 */
CHIP_ERROR LogEvent(EventLoggingDelegate * apDelegate, const EventOptions * apOptions, chip::EventNumber & aEventNumber);

} // namespace reporting
} // namespace app
} // namespace chip
