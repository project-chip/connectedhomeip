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

EventOptions::EventOptions(void) : mTimestamp(Timestamp::Type::kInvalid), mpEventSchema(nullptr), mUrgent(false) {}

EventOptions::EventOptions(bool aUrgent) :
    mTimestamp(Timestamp::Type::kInvalid), mpEventSchema(nullptr), mUrgent(aUrgent)
{}

EventOptions::EventOptions(Timestamp aTimestamp) :
    mTimestamp(aTimestamp), mpEventSchema(nullptr), mUrgent(false)
{}

EventOptions::EventOptions(Timestamp aTimestamp, bool aUrgent) :
    mTimestamp(aTimestamp), mpEventSchema(nullptr), mUrgent(aUrgent)
{}

EventLoadOutContext::EventLoadOutContext(chip::TLV::TLVWriter & aWriter, PriorityLevel aPriority,
                                         chip::EventNumber aStartingEventNumber) :
    mWriter(aWriter),
    mPriority(aPriority), mStartingEventNumber(aStartingEventNumber), mCurrentSystemTime(Timestamp::Type::kSystem), mCurrentEventNumber(0), mCurrentUTCTime(Timestamp::Type::kUTC),
    mFirstUtc(true), mFirst(true)
{}

} // namespace reporting
} // namespace app
} // namespace chip
