/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef _CHIP_DATA_MODEL_EVENT_LOGGING_TYPES_H
#define _CHIP_DATA_MODEL_EVENT_LOGGING_TYPES_H

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
    kPriorityLevel_Invalid = 4,

} PriorityLevel;

/**
 * @brief
 *   The structure that defines a schema for event metadata.
 */
struct EventSchema
{
    chip::ClusterId mClusterId; ///< ID of cluster
    uint32_t mStructureType;    ///< Type of structure
    PriorityLevel mPriority;    ///< Priority
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
    kTag_Region  = 1, ///< An 32-bit unsigned indicating the log region, i.e. the module to which the log message pertains.
    kTag_Message = 2  ///< A string containing the actual debug message.
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
    DetailedRootSection(void){};
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
    Timestamp(void) : systemTimestamp(0){};

    /**
     * UTC timestamp constructor.
     */
    Timestamp(utc_timestamp_t aUtc) : utcTimestamp(aUtc){};

    /**
     * System timestamp constructor.
     */
    Timestamp(timestamp_t aSystem) : systemTimestamp(aSystem){};

    timestamp_t systemTimestamp;  ///< System timestamp.
    utc_timestamp_t utcTimestamp; ///< UTC timestamp.
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

    Timestamp timestamp; /**< A union holding either system or UTC timestamp. */

    DetailedRootSection * eventSource; /**< A pointer to the detailed resolution of the cluster instance.  When NULL, the event
                                          source is assumed to come from the resource equal to the local node ID, and from the
                                          default instance of the cluster. */

    PriorityLevel relatedPriority; /**< EventPriority of the Related Event Number.  When this event and the related event are of the
                                           same priority, the field may be omitted.  A value of kPriorityLevel_Invalid implies the
                                           absence of any related event. */

    TimestampType timestampType; /**< An enum indicating if the timestamp is valid and its type. */

    bool urgent; /**< A flag denoting that the event is time sensitive.  When set, it causes the event log to be flushed. */
};

/**
 * @brief
 *   Structure for copying event lists on output.
 */

struct EventLoadOutContext
{
    EventLoadOutContext(chip::TLV::TLVWriter & aWriter, PriorityLevel aPriority, uint32_t aStartingEventID);

    chip::TLV::TLVWriter & mWriter;
    PriorityLevel mPriority;
    uint32_t mStartingEventID;
    uint32_t mCurrentTime;
    uint32_t mCurrentEventID;
    uint64_t mCurrentUTCTime;
    bool mFirstUtc;
    bool mFirst;
};

/**
 *  @brief
 *    A function that supplies eventData element for the event logging subsystem.
 *
 *  Functions of this type are expected to provide the eventData
 *  element for the event logging subsystem. The functions of this
 *  type are called after the event subsystem has generated all
 *  required event metadata. The function is called with a
 *  chip::TLV::TLVWriter object into which it will emit a single TLV element
 *  tagged kTag_EventData; the value of that element MUST be a
 *  structure containing the event data. The event data itself must
 *  be structured using context tags.
 *
 *  @sa PlainTextWriter
 *  @sa EventWriterTLVCopy
 *
 *  @param[inout] aWriter A reference to the chip::TLV::TLVWriter object to be
 *                         used for event data serialization.
 *
 *
 *  @param[in]    apAppState  A pointer to an application specific context.
 *
 *  @retval #CHIP_NO_ERROR  On success.
 *
 *  @retval other           An appropriate error signaling to the
 *                          caller that the serialization of event
 *                          data could not be completed. Errors from
 *                          calls to the aWriter should be propagated
 *                          without remapping. If the function
 *                          returns any type of error, the event
 *                          generation is aborted, and the event is not
 *                          written to the log.
 *
 */

typedef CHIP_ERROR (*EventWriterFunct)(chip::TLV::TLVWriter & aWriter, void * apAppState);

// internal API
typedef CHIP_ERROR (*LoggingBufferHandler)(void * inAppState, chip::System::PacketBuffer * apBuffer);

} // namespace reporting
} // namespace app
} // namespace chip
#endif //_CHIP_DATA_MODEL_EVENT_LOGGING_TYPES_H
