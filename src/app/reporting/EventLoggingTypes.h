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

typedef enum PriorityLevel
{

    kPriorityLevel_First = 1,
    /**
     * Critical priority denotes events whose loss would
     * directly impact customer-facing features. Applications may use
     * loss of Production Critical events to indicate system failure.
     * On constrained devices, entries logged with Critical
     * priority must be accounted for in the power and memory budget,
     * as it is expected that they are always logged and offloaded
     * from the device.
     */
    Critical = kPriorityLevel_First,
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
    Debug                  = 3,
    kPriorityLevel_Last    = Debug,
    kPriorityLevel_Invalid = kPriorityLevel_Last + 1,

} PriorityLevel;

/**
 * @brief
 *   The structure that defines a schema for event metadata.
 */
struct EventSchema
{
    chip::ClusterId mClusterId; //< ID of cluster
    chip::EventId mEventId;     //< Event Id
    PriorityLevel mPriority;    //< Priority
};

/**
 * @typedef timestamp_t
 * Type used to describe the timestamp in milliseconds.
 */
typedef uint32_t timestamp_t;

/**
 * @typedef duration_t
 * Type used to describe the duration, in milliseconds.
 */
typedef uint32_t duration_t;

/**
 * @typedef utc_timestamp_t
 * Type used to describe the UTC timestamp in milliseconds.
 */
typedef uint64_t utc_timestamp_t;

enum
{
    kTag_Region  = 1, //< An 32-bit unsigned indicating the log region, i.e. the module to which the log message pertains.
    kTag_Message = 2  //< A string containing the actual debug message.
};

// Structures used to describe in detail additional options for event encoding

/**
 * @brief
 *   The structure that provides a full resolution of the cluster.
 */
struct DetailedRootSection
{
    /**
     * Default constructor
     */
    DetailedRootSection(chip::NodeId aNodeId, chip::EndpointId aEndpointId) : mNodeId(aNodeId), mEndpointId(aEndpointId){};
    chip::NodeId mNodeId;
    chip::EndpointId mEndpointId;
};

/**
 * @brief
 *   The validity and type of timestamp included in EventOptions.
 */
typedef enum TimestampType
{
    kTimestampType_Invalid = 0,
    kTimestampType_System,
    kTimestampType_UTC
} TimestampType;

/**
 * @brief
 *   The union that provides an application set system or UTC timestamp.
 */
union Timestamp
{
    /**
     * Default constructor.
     */
    Timestamp(void) : systemTimestamp(0){}

    /**
     * UTC timestamp constructor.
     */
    Timestamp(utc_timestamp_t aUtc) : utcTimestamp(aUtc){}

    /**
     * System timestamp constructor.
     */
    Timestamp(timestamp_t aSystem) : systemTimestamp(aSystem){}

    timestamp_t systemTimestamp;  //< System timestamp.
    utc_timestamp_t utcTimestamp; //< UTC timestamp.
};

/**
 *   The structure that provides options for the different event fields.
 */
class EventOptions
{
public:
    EventOptions(void);
    EventOptions(bool);
    EventOptions(timestamp_t);
    EventOptions(utc_timestamp_t);
    EventOptions(timestamp_t, bool);
    EventOptions(utc_timestamp_t, bool);

    Timestamp timestamp;

    DetailedRootSection * eventSource; /**< A pointer to the detailed resolution of the cluster instance.  When NULL, the event
                                          source is assumed to come from the resource equal to the local node ID, and from the
                                          default instance of the cluster. */

    TimestampType timestampType; /**< An enum indicating if the timestamp is valid and its type. */

    bool urgent; /**< A flag denoting that the event is time sensitive.  When set, it causes the event log to be flushed. */
};

/**
 * @brief
 *   Structure for copying event lists on output.
 */

struct EventLoadOutContext
{
    EventLoadOutContext(chip::TLV::TLVWriter & aWriter, PriorityLevel aPriority, chip::EventNumber aStartingEventNumber);

    chip::TLV::TLVWriter & mWriter;
    PriorityLevel mPriority;
    chip::EventNumber mStartingEventNumber;
    timestamp_t mCurrentTime;
    chip::EventNumber mCurrentEventNumber;
    utc_timestamp_t mCurrentUTCTime;
    bool mFirstUtc;
    bool mFirst;
};

typedef CHIP_ERROR (*InternalLoggingBufferHandler)(void * inAppState, chip::System::PacketBuffer * apBuffer);

} // namespace reporting
} // namespace app
} // namespace chip
