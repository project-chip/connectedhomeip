/**
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

#pragma once

#include <app/ClusterInfo.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <system/SystemPacketBuffer.h>

constexpr size_t kNumPriorityLevel = 3;
namespace chip {
namespace app {

/**
 * @brief
 *   The Priority of the log entry.
 *
 * @details
 * Priority is used as a way to filter events before they are
 * actually emitted into the log. After the event is in the log, we
 * make no further provisions to expunge it from the log.
 * The priority level serves to prioritize event storage. If an
 * event of high priority is added to a full buffer, events are
 * dropped in order of priority (and age) to accommodate it. As such,
 * priority levels only have relative value. If a system is
 * using only one priority level, events are dropped only in order
 * of age, like a ring buffer.
 */

enum class PriorityLevel : uint8_t
{

    First = 0,
    /**
     *  Debug priority denotes log entries of interest to the
     *  developers of the system and is used primarily in the
     *  development phase. Debug priority logs are
     *  not accounted for in the bandwidth or power budgets of the
     *  constrained devices; as a result, they must be used only over
     *  a limited time span in production systems.
     */

    Debug = First,
    /**
     * Info priority denotes log entries that provide extra insight
     * and diagnostics into the running system. Info logging level may
     * be used over an extended period of time in a production system,
     * or may be used as the default log level in a field trial. On
     * the constrained devices, the entries logged with Info level must
     * be accounted for in the bandwidth and memory budget, but not in
     * the power budget.
     */
    Info = 1,

    /**
     * Critical priority denotes events whose loss would
     * directly impact customer-facing features. Applications may use
     * loss of Production Critical events to indicate system failure.
     * On constrained devices, entries logged with Critical
     * priority must be accounted for in the power and memory budget,
     * as it is expected that they are always logged and offloaded
     * from the device.
     */
    Critical = 2,
    Last     = Critical,
    Invalid  = Last + 1,

};

static_assert(sizeof(std::underlying_type_t<PriorityLevel>) <= sizeof(unsigned),
              "Logging that converts PriorityLevel to unsigned will be lossy");

/**
 * @brief
 *   The structure that provides a full resolution of the cluster.
 */
struct EventSchema
{
    EventSchema(NodeId aNodeId, EndpointId aEndpointId, ClusterId aClusterId, EventId aEventId, PriorityLevel aPriority) :
        mNodeId(aNodeId), mEndpointId(aEndpointId), mClusterId(aClusterId), mEventId(aEventId), mPriority(aPriority)
    {}
    NodeId mNodeId          = 0;
    EndpointId mEndpointId  = 0;
    ClusterId mClusterId    = 0;
    EventId mEventId        = 0;
    PriorityLevel mPriority = PriorityLevel::Invalid;
};

/**
 * @brief
 *   The struct that provides an application set system or UTC timestamp.
 */
struct Timestamp
{
    enum class Type
    {
        kInvalid = 0,
        kSystem,
        kUTC
    };
    Timestamp() {}
    Timestamp(Type aType) : mType(aType) { mValue = 0; }
    Timestamp(Type aType, uint64_t aValue) : mType(aType), mValue(aValue) {}
    static Timestamp UTC(uint64_t aValue)
    {
        Timestamp timestamp(Type::kUTC, aValue);
        return timestamp;
    }
    static Timestamp System(uint64_t aValue)
    {
        Timestamp timestamp(Type::kSystem, aValue);
        return timestamp;
    }

    Type mType      = Type::kInvalid;
    uint64_t mValue = 0;
};

/**
 *   The structure that provides options for the different event fields.
 */
class EventOptions
{
public:
    enum class Type
    {
        kUrgent = 0,
        kNotUrgent,
    };
    EventOptions(void) : mTimestamp(Timestamp::Type::kInvalid), mpEventSchema(nullptr), mUrgent(Type::kNotUrgent) {}

    EventOptions(Type aType) : mTimestamp(Timestamp::Type::kInvalid), mpEventSchema(nullptr), mUrgent(aType) {}

    EventOptions(Timestamp aTimestamp) : mTimestamp(aTimestamp), mpEventSchema(nullptr), mUrgent(Type::kNotUrgent) {}

    EventOptions(Timestamp aTimestamp, Type aUrgent) : mTimestamp(aTimestamp), mpEventSchema(nullptr), mUrgent(aUrgent) {}
    Timestamp mTimestamp;

    EventSchema * mpEventSchema = nullptr; /**< A pointer to the schema of the cluster instance.*/

    Type mUrgent = Type::kNotUrgent; /**< A flag denoting that the event is time sensitive.  When set, it causes the event log to be
                                        flushed. */
};

/**
 * @brief
 *   Structure for copying event lists on output.
 */
struct EventLoadOutContext
{
    EventLoadOutContext(TLV::TLVWriter & aWriter, PriorityLevel aPriority, EventNumber aStartingEventNumber) :
        mWriter(aWriter), mPriority(aPriority), mStartingEventNumber(aStartingEventNumber),
        mCurrentSystemTime(Timestamp::Type::kSystem), mCurrentEventNumber(0), mCurrentUTCTime(Timestamp::Type::kUTC), mFirst(true)
    {}

    TLV::TLVWriter & mWriter;
    PriorityLevel mPriority          = PriorityLevel::Invalid;
    EventNumber mStartingEventNumber = 0;
    Timestamp mPreviousSystemTime;
    Timestamp mCurrentSystemTime;
    EventNumber mCurrentEventNumber = 0;
    size_t mEventCount              = 0;
    Timestamp mCurrentUTCTime;
    ClusterInfo * mpInterestedEventPaths = nullptr;
    bool mFirst                          = true;
};
} // namespace app
} // namespace chip
