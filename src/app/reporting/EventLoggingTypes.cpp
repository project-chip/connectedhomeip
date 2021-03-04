/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 * @file
 *
 * @brief
 *   Implementations of enums, types, and tags used in Chip Event Logging.
 *
 */

#include <app/reporting/EventLoggingTypes.h>

namespace chip {
namespace app {
namespace reporting {

EventOptions::EventOptions(void) : mTimestamp(), mpEventSchema(nullptr), mTimestampType(TimestampType::Invalid), mUrgent(false) {}

EventOptions::EventOptions(bool aUrgent) :
    mTimestamp(), mpEventSchema(nullptr), mTimestampType(TimestampType::Invalid), mUrgent(aUrgent)
{}

EventOptions::EventOptions(timestamp_t aSystemTimestamp) :
    mTimestamp(aSystemTimestamp), mpEventSchema(nullptr), mTimestampType(TimestampType::System), mUrgent(false)
{}

EventOptions::EventOptions(utc_timestamp_t aUtcTimestamp) :
    mTimestamp(aUtcTimestamp), mpEventSchema(nullptr), mTimestampType(TimestampType::UTC), mUrgent(false)
{}

EventOptions::EventOptions(timestamp_t aSystemTimestamp, bool aUrgent) :
    mTimestamp(aSystemTimestamp), mpEventSchema(nullptr), mTimestampType(TimestampType::System), mUrgent(aUrgent)
{}

EventOptions::EventOptions(utc_timestamp_t aUtcTimestamp, bool aUrgent) :
    mTimestamp(aUtcTimestamp), mpEventSchema(nullptr), mTimestampType(TimestampType::UTC), mUrgent(aUrgent)
{}

EventLoadOutContext::EventLoadOutContext(chip::TLV::TLVWriter & aWriter, PriorityLevel aPriority,
                                         chip::EventNumber aStartingEventNumber) :
    mWriter(aWriter),
    mPriority(aPriority), mStartingEventNumber(aStartingEventNumber), mCurrentTime(0), mCurrentEventNumber(0), mCurrentUTCTime(0),
    mFirstUtc(true), mFirst(true)
{}

} // namespace reporting
} // namespace app
} // namespace chip
