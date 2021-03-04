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
 * @file
 *
 * @brief
 *   Enums, types, and tags used in Chip Event Logging.
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <system/SystemPacketBuffer.h>
#include <util/basic-types.h>

#define CHIP_NUM_PRIORITY_LEVEL 3

namespace chip {
namespace app {
namespace reporting {

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

    First = 1,
    /**
     * Critical priority denotes events whose loss would
     * directly impact customer-facing features. Applications may use
     * loss of Production Critical events to indicate system failure.
     * On constrained devices, entries logged with Critical
     * priority must be accounted for in the power and memory budget,
     * as it is expected that they are always logged and offloaded
     * from the device.
     */
    Critical = First,
    /**
     * Info priority denotes log entries that provide extra insight
     * and diagnostics into the running system. Info logging level may
     * be used over an extended period of time in a production system,
     * or may be used as the default log level in a field trial. On
     * the constrained devices, the entries logged with Info level must
     * be accounted for in the bandwidth and memory budget, but not in
     * the power budget.
     */
    Info = 2,

    /**
     *  Debug priority denotes log entries of interest to the
     *  developers of the system and is used primarily in the
     *  development phase. Debug priority logs are
     *  not accounted for in the bandwidth or power budgets of the
     *  constrained devices; as a result, they must be used only over
     *  a limited time span in production systems.
     */
    Debug   = 3,
    Last    = Debug,
    Invalid = Last + 1,

};

/**
 * @brief
 *   The structure that provides a full resolution of the cluster.
 */
struct EventSchema
{
    EventSchema(chip::NodeId aNodeId, chip::EndpointId aEndpointId, chip::ClusterId aClusterId, chip::EventId aEventId,
                PriorityLevel aPriority) :
        mNodeId(aNodeId),
        mEndpointId(aEndpointId), mClusterId(aClusterId), mEventId(aEventId), mPriority(aPriority){};
    chip::NodeId mNodeId = 0;
    chip::EndpointId mEndpointId = 0;
    chip::ClusterId mClusterId = 0;
    chip::EventId mEventId = 0;
    PriorityLevel mPriority = PriorityLevel::Invalid;
};

/**
 * @brief
 *   The union that provides an application set system or UTC timestamp.
 */
struct Timestamp {
    enum class Type
    {
        kInvalid = 0,
        kSystem,
        kUTC
    };
    Timestamp(Type aType) : mType(aType) { mValue = 0;};
    Timestamp(Type aType, uint64_t aValue) : mType(aType), mValue(aValue) {};
    void Init(Type aType, uint64_t aValue)
    {
        mType = aType;
        mValue = aValue;
    }
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

    Type mType = Type::kInvalid;
    uint64_t mValue = 0;
};

/**
 *   The structure that provides options for the different event fields.
 */
class EventOptions
{
public:
    EventOptions(void);
    EventOptions(bool);
    EventOptions(Timestamp);
    EventOptions(Timestamp, bool);

    Timestamp mTimestamp;

    EventSchema * mpEventSchema; /**< A pointer to the detailed shcema of the cluster instance.*/

    bool mUrgent; /**< A flag denoting that the event is time sensitive.  When set, it causes the event log to be flushed. */
};

/**
 * @brief
 *   Structure for copying event lists on output.
 */

struct EventLoadOutContext
{
    EventLoadOutContext(chip::TLV::TLVWriter & aWriter, PriorityLevel aPriority, chip::EventNumber aStartingEventNumber);

    chip::TLV::TLVWriter & mWriter;
    PriorityLevel mPriority                = PriorityLevel::Invalid;
    chip::EventNumber mStartingEventNumber = 0;
    Timestamp mCurrentSystemTime;
    chip::EventNumber mCurrentEventNumber  = 0;
    Timestamp mCurrentUTCTime;
    bool mFirstUtc                         = true;
    bool mFirst                            = true;
};

using InternalLoggingBufferHandler = CHIP_ERROR(void * inAppState, chip::System::PacketBuffer * apBuffer);

} // namespace reporting
} // namespace app
} // namespace chip
