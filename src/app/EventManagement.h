/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *   Management of the CHIP Event Logging.
 *
 */
#pragma once

#include "EventLoggingDelegate.h"
#include <access/SubjectDescriptor.h>
#include <app/EventLoggingTypes.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/MessageDef/StatusIB.h>
#include <app/data-model-provider/EventsGenerator.h>
#include <app/util/basic-types.h>
#include <lib/core/TLVCircularBuffer.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/LinkedList.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceConfig.h>
#include <system/SystemClock.h>

/**
 * Events are stored in the LogStorageResources provided to
 * EventManagement::Init.
 *
 * A newly generated event will be placed in the lowest-priority (in practice
 * DEBUG) buffer, the one associated with the first LogStorageResource.  If
 * there is no space in that buffer, space will be created by evicting the
 * oldest event currently in that buffer, until enough space is available.
 *
 * When an event is evicted from a buffer, there are two possibilities:
 *
 * 1) If the next LogStorageResource has a priority that is no higher than the
 *    event's priority, the event will be moved to that LogStorageResource's
 *    buffer.  This may in turn require events to be evicted from that buffer.
 * 2) If the next LogStorageResource has a priority that is higher than the
 *    event's priority, then the event is just dropped.
 *
 * This means that LogStorageResources at a given priority level are reserved
 * for events of that priority level or higher priority.
 *
 * As a simple example, assume there are only two priority levels, DEBUG and
 * CRITICAL, and two LogStorageResources with those priorities.  In that case,
 * old CRITICAL events will not start getting dropped until both buffers are
 * full, while old DEBUG events will start getting dropped once the DEBUG
 * LogStorageResource buffer is full.
 */

#define CHIP_CONFIG_EVENT_GLOBAL_PRIORITY PriorityLevel::Debug

namespace chip {
namespace app {
inline constexpr const uint32_t kEventManagementProfile = 0x1;
inline constexpr const uint32_t kFabricIndexTag         = 0x1;
inline constexpr size_t kMaxEventSizeReserve            = 512;
constexpr uint16_t kRequiredEventField =
    (1 << to_underlying(EventDataIB::Tag::kPriority)) | (1 << to_underlying(EventDataIB::Tag::kPath));

/**
 * @brief
 *   Internal event buffer, built around the TLV::TLVCircularBuffer
 */

class CircularEventBuffer : public TLV::TLVCircularBuffer
{
public:
    /**
     * @brief
     *   A constructor for the CircularEventBuffer (internal API).
     */
    CircularEventBuffer() : TLVCircularBuffer(nullptr, 0){};

    /**
     * @brief
     *   A Init for the CircularEventBuffer (internal API).
     *
     * @param[in] apBuffer       The actual storage to use for event storage.
     *
     * @param[in] aBufferLength The length of the \c apBuffer in bytes.
     *
     * @param[in] apPrev         The pointer to CircularEventBuffer storing
     *                           events of lesser priority.
     *
     * @param[in] apNext         The pointer to CircularEventBuffer storing
     *                           events of greater priority.
     *
     * @param[in] aPriorityLevel CircularEventBuffer priority level
     */
    void Init(uint8_t * apBuffer, uint32_t aBufferLength, CircularEventBuffer * apPrev, CircularEventBuffer * apNext,
              PriorityLevel aPriorityLevel);

    /**
     * @brief
     *   A helper function that determines whether the event of
     *   specified priority is final destination
     *
     * @param[in]   aPriority   Priority of the event.
     *
     * @retval true/false event's priority is same as current buffer's priority, otherwise, false
     */
    bool IsFinalDestinationForPriority(PriorityLevel aPriority) const;

    PriorityLevel GetPriority() { return mPriority; }

    CircularEventBuffer * GetPreviousCircularEventBuffer() { return mpPrev; }
    CircularEventBuffer * GetNextCircularEventBuffer() { return mpNext; }

    void SetRequiredSpaceforEvicted(size_t aRequiredSpace) { mRequiredSpaceForEvicted = aRequiredSpace; }
    size_t GetRequiredSpaceforEvicted() const { return mRequiredSpaceForEvicted; }

    ~CircularEventBuffer() override = default;

private:
    CircularEventBuffer * mpPrev = nullptr; ///< A pointer CircularEventBuffer storing events less important events
    CircularEventBuffer * mpNext = nullptr; ///< A pointer CircularEventBuffer storing events more important events

    PriorityLevel mPriority = PriorityLevel::Invalid; ///< The buffer is the final bucket for events of this priority.  Events of
                                                      ///< lesser priority are dropped when they get bumped out of this buffer

    size_t mRequiredSpaceForEvicted = 0; ///< Required space for previous buffer to evict event to new buffer

    CHIP_ERROR OnInit(TLV::TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) override;
};

class CircularEventReader;

/**
 * @brief
 *   A CircularEventBufferWrapper which has a pointer to the "current CircularEventBuffer". When trying to locate next buffer,
 *   if nothing left there update its CircularEventBuffer until the buffer with data has been found,
 *   the tlv reader will have a pointer to this impl.
 */
class CircularEventBufferWrapper : public TLV::TLVCircularBuffer
{
public:
    CircularEventBufferWrapper() : TLVCircularBuffer(nullptr, 0), mpCurrent(nullptr){};
    CircularEventBuffer * mpCurrent;

private:
    CHIP_ERROR GetNextBuffer(chip::TLV::TLVReader & aReader, const uint8_t *& aBufStart, uint32_t & aBufLen) override;
};

enum class EventManagementStates
{
    Idle       = 1, // No log offload in progress, log offload can begin without any constraints
    InProgress = 2, // Log offload in progress
    Shutdown   = 3  // Not capable of performing any logging operation
};

/**
 * @brief
 *   A helper class used in initializing logging management.
 *
 * The class is used to encapsulate the resources allocated by the caller and denotes
 * resources to be used in logging events of a particular priority.  Note that
 * while resources referring to the counters are used exclusively by the
 * particular priority level, the buffers are shared between `this` priority
 * level and events that are "more" important.
 */

struct LogStorageResources
{
    // TODO: Update TLVCircularBuffer with size_t for buffer size, then use ByteSpan
    uint8_t * mpBuffer =
        nullptr; // Buffer to be used as a storage at the particular priority level and shared with more important events.
                 // Must not be nullptr.  Must be large enough to accommodate the largest event emitted by the system.
    uint32_t mBufferSize = 0; ///< The size, in bytes, of the `mBuffer`.
    PriorityLevel mPriority =
        PriorityLevel::Invalid; // Log priority level associated with the resources provided in this structure.
};

/**
 * @brief
 *   A class for managing the in memory event logs.  See documentation at the
 *   top of the file describing the eviction policy for events when there is no
 *   more space for new events.
 */

class EventManagement : public DataModel::EventsGenerator
{
public:
    /**
     * Initialize the EventManagement with an array of LogStorageResources and
     * an equal-length array of CircularEventBuffers that correspond to those
     * LogStorageResources. The array of LogStorageResources must provide a
     * resource for each valid priority level, the elements of the array must be
     * in increasing numerical value of priority (and in increasing priority);
     * the first element in the array corresponds to the resources allocated for
     * least important events, and the last element corresponds to the most
     * critical events.
     *
     * @param[in] apExchangeManager         ExchangeManager to be used with this logging subsystem
     *
     * @param[in] aNumBuffers  Number of elements in the apLogStorageResources
     *                         and apCircularEventBuffer arrays.
     *
     * @param[in] apCircularEventBuffer  An array of CircularEventBuffer for each priority level.
     *
     * @param[in] apLogStorageResources  An array of LogStorageResources for each priority level.
     *
     * @param[in] apEventNumberCounter   A counter to use for event numbers.
     *
     * @param[in] aMonotonicStartupTime  Time we should consider as "monotonic
     *                                   time 0" for cases when we use
     *                                   system-time event timestamps.
     *
     */
    void Init(Messaging::ExchangeManager * apExchangeManager, uint32_t aNumBuffers, CircularEventBuffer * apCircularEventBuffer,
              const LogStorageResources * const apLogStorageResources,
              MonotonicallyIncreasingCounter<EventNumber> * apEventNumberCounter,
              System::Clock::Milliseconds64 aMonotonicStartupTime);

    static EventManagement & GetInstance();

    /**
     * @brief Create EventManagement object and initialize the logging management
     *   subsystem with provided resources.
     *
     * Initialize the EventManagement with an array of LogStorageResources.  The
     * array must provide a resource for each valid priority level, the elements
     * of the array must be in increasing numerical value of priority (and in
     * decreasing priority); the first element in the array corresponds to the
     * resources allocated for the most critical events, and the last element
     * corresponds to the least important events.
     *
     * @param[in] apExchangeManager         ExchangeManager to be used with this logging subsystem
     *
     * @param[in] aNumBuffers  Number of elements in inLogStorageResources array
     *
     * @param[in] apCircularEventBuffer  An array of CircularEventBuffer for each priority level.
     * @param[in] apLogStorageResources  An array of LogStorageResources for each priority level.
     *
     * @param[in] apEventNumberCounter   A counter to use for event numbers.
     *
     * @param[in] aMonotonicStartupTime  Time we should consider as "monotonic
     *                                   time 0" for cases when we use
     *                                   system-time event timestamps.
     *
     * @note This function must be called prior to the logging being used.
     */
    static void
    CreateEventManagement(Messaging::ExchangeManager * apExchangeManager, uint32_t aNumBuffers,
                          CircularEventBuffer * apCircularEventBuffer, const LogStorageResources * const apLogStorageResources,
                          MonotonicallyIncreasingCounter<EventNumber> * apEventNumberCounter,
                          System::Clock::Milliseconds64 aMonotonicStartupTime = System::SystemClock().GetMonotonicMilliseconds64());

    static void DestroyEventManagement();

    /**
     * @brief
     *   Log an event via a EventLoggingDelegate, with options.
     *
     * The EventLoggingDelegate writes the event metadata and calls the `apDelegate`
     * with an TLV::TLVWriter reference so that the user code can emit
     * the event data directly into the event log.  This form of event
     * logging minimizes memory consumption, as event data is serialized
     * directly into the target buffer.  The event data MUST contain
     * context tags to be interpreted within the schema identified by
     * `ClusterID` and `EventId`. The tag of the first element will be
     * ignored; the event logging system will replace it with the
     * eventData tag.
     *
     * The event is logged if the schema priority exceeds the logging
     * threshold specified in the LoggingConfiguration.  If the event's
     * priority does not meet the current threshold, it is dropped and
     * the function returns a `0` as the resulting event ID.
     *
     * This variant of the invocation permits the caller to set any
     * combination of `EventOptions`:
     * - timestamp, when 0 defaults to the current time at the point of
     *   the call,
     * - "root" section of the event source (event source and cluster ID);
     *   if NULL, it defaults to the current device. the event is marked as
     *   relating to the device that is making the call,
     *
     * @param[in] apDelegate The EventLoggingDelegate to serialize the event data
     *
     * @param[in] aEventOptions    The options for the event metadata.
     *
     * @param[out] aEventNumber The event Number if the event was written to the
     *                         log, 0 otherwise.
     *
     * @return CHIP_ERROR  CHIP Error Code
     */
    CHIP_ERROR LogEvent(EventLoggingDelegate * apDelegate, const EventOptions & aEventOptions, EventNumber & aEventNumber);

    /**
     * @brief
     *   A helper method to get tlv reader along with buffer has data from particular priority
     *
     * @param[in,out] aReader A reference to the reader that will be
     *                        initialized with the backing storage from
     *                        the event log
     *
     * @param[in] aPriority The starting priority for the reader.
     *                         Note that in this case the starting
     *                         priority is somewhat counter intuitive:
     *                         more important events share the buffers
     *                         with less priority events, in addition to
     *                         their dedicated buffers.  As a result, the
     *                         reader will traverse the least data when
     *                         the Debug priority is passed in.
     *
     * @param[in] apBufWrapper CircularEventBufferWrapper
     * @return                 #CHIP_NO_ERROR Unconditionally.
     */
    CHIP_ERROR GetEventReader(chip::TLV::TLVReader & aReader, PriorityLevel aPriority,
                              app::CircularEventBufferWrapper * apBufWrapper);

    /**
     * @brief
     *   A function to retrieve events of specified priority since a specified event ID.
     *
     * Given a TLV::TLVWriter, an priority type, and an event ID, the
     * function will fetch events since the
     * specified event number.  The function will continue fetching events until
     * it runs out of space in the TLV::TLVWriter or in the log. The function
     * will terminate the event writing on event boundary. The function would filter out event based upon interested path
     * specified by read/subscribe request.
     *
     * @param[in] aWriter     The writer to use for event storage
     * @param[in] apEventPathList the interested EventPathParams list
     *
     * @param[in,out] aEventMin On input, the Event number is the one we're fetching.  On
     *                         completion, the event number of the next one we plan to fetch.
     *
     * @param[out] aEventCount The number of fetched event
     * @param[in] aSubjectDescriptor Subject descriptor for current read handler
     * @retval #CHIP_END_OF_TLV             The function has reached the end of the
     *                                       available log entries at the specified
     *                                       priority level
     *
     * @retval #CHIP_ERROR_NO_MEMORY        The function ran out of space in the
     *                                       aWriter, more events in the log are
     *                                       available.
     *
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL The function ran out of space in the
     *                                       aWriter, more events in the log are
     *                                       available.
     *
     */
    CHIP_ERROR FetchEventsSince(chip::TLV::TLVWriter & aWriter, const SingleLinkedListNode<EventPathParams> * apEventPathList,
                                EventNumber & aEventMin, size_t & aEventCount,
                                const Access::SubjectDescriptor & aSubjectDescriptor);
    /**
     * @brief brief Iterate all events and invalidate the fabric-sensitive events whose associated fabric has the given fabric
     * index.
     */
    CHIP_ERROR FabricRemoved(FabricIndex aFabricIndex);

    /**
     * @brief
     *   Fetch the most recently vended Number for a particular priority level
     *
     * @return EventNumber most recently vended event Number for that event priority
     */
    EventNumber GetLastEventNumber() const { return mLastEventNumber; }

    /**
     * @brief
     *   IsValid returns whether the EventManagement instance is valid
     */
    bool IsValid(void) { return EventManagementStates::Shutdown != mState; };

    /**
     *  Logger would save last logged event number and initial written event bytes number into schedule event number array
     */
    void SetScheduledEventInfo(EventNumber & aEventNumber, uint32_t & aInitialWrittenEventBytes) const;

    /* EventsGenerator implementation */
    CHIP_ERROR GenerateEvent(EventLoggingDelegate * eventPayloadWriter, const EventOptions & options,
                             EventNumber & generatedEventNumber) override;

private:
    /**
     * @brief
     *  Internal structure for traversing events.
     */
    struct EventEnvelopeContext
    {
        EventEnvelopeContext() {}

        int mFieldsToRead = 0;
        /* PriorityLevel and DeltaTime are there if that is not first event when putting events in report*/
#if CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
        Timestamp mCurrentTime = Timestamp::Epoch(System::Clock::kZero);
#else  // CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
        Timestamp mCurrentTime = Timestamp::System(System::Clock::kZero);
#endif // CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
        PriorityLevel mPriority  = PriorityLevel::First;
        ClusterId mClusterId     = 0;
        EndpointId mEndpointId   = 0;
        EventId mEventId         = 0;
        EventNumber mEventNumber = 0;
        Optional<FabricIndex> mFabricIndex;
    };

    void VendEventNumber();
    CHIP_ERROR CalculateEventSize(EventLoggingDelegate * apDelegate, const EventOptions * apOptions, uint32_t & requiredSize);
    /**
     * @brief Helper function for writing event header and data according to event
     *   logging protocol.
     *
     * @param[in,out] apContext EventLoadOutContext, initialized with stateful
     *                          information for the buffer. State is updated
     *                          and preserved by ConstructEvent using this context.
     *
     * @param[in] apDelegate The EventLoggingDelegate to serialize the event data
     *
     * @param[in] apOptions     EventOptions describing timestamp and other tags
     *                          relevant to this event.
     *
     */
    CHIP_ERROR ConstructEvent(EventLoadOutContext * apContext, EventLoggingDelegate * apDelegate, const EventOptions * apOptions);

    // Internal function to log event
    CHIP_ERROR LogEventPrivate(EventLoggingDelegate * apDelegate, const EventOptions & aEventOptions, EventNumber & aEventNumber);

    /**
     * @brief copy the event outright to next buffer with higher priority
     *
     * @param[in] apEventBuffer  CircularEventBuffer
     *
     */
    CHIP_ERROR CopyToNextBuffer(CircularEventBuffer * apEventBuffer);

    /**
     * @brief Ensure that:
     *
     * 1) There could be aRequiredSpace bytes available (if enough things were
     *    evicted) in all buffers that can hold events with priority aPriority.
     *
     * 2) There are in fact aRequiredSpace bytes available in our
     *    lowest-priority buffer.  This might involve evicting some events to
     *    higher-priority buffers or dropping them.
     *
     * @param[in] aRequiredSpace  required space
     * @param[in] aPriority       priority of the event we are making space for.
     *
     */
    CHIP_ERROR EnsureSpaceInCircularBuffer(size_t aRequiredSpace, PriorityLevel aPriority);

    /**
     * @brief Iterate the event elements inside event tlv and mark the fabric index as kUndefinedFabricIndex if
     * it matches the FabricIndex apFabricIndex points to.
     *
     * @param[in] aReader  event tlv reader
     * @param[in] apFabricIndex   A FabricIndex* pointing to the fabric index for which we want to effectively evict events.
     *
     */
    static CHIP_ERROR FabricRemovedCB(const TLV::TLVReader & aReader, size_t, void * apFabricIndex);

    /**
     * @brief
     *   Internal API used to implement #FetchEventsSince
     *
     * Iterator function to used to copy an event from the log into a
     * TLVWriter. The included apContext contains the context of the copy
     * operation, including the TLVWriter that will hold the copy of an
     * event.  If event cannot be written as a whole, the TLVWriter will
     * be rolled back to event boundary.
     *
     * @retval #CHIP_END_OF_TLV             Function reached the end of the event
     * @retval #CHIP_ERROR_NO_MEMORY        Function could not write a portion of
     *                                       the event to the TLVWriter.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL Function could not write a
     *                                       portion of the event to the TLVWriter.
     */
    static CHIP_ERROR CopyEventsSince(const TLV::TLVReader & aReader, size_t aDepth, void * apContext);

    /**
     * @brief Internal iterator function used to scan and filter though event logs
     *
     * The function is used to scan through the event log to find events matching the spec in the supplied context.
     * Particularly, it would check against mStartingEventNumber, and skip fetched event.
     */
    static CHIP_ERROR EventIterator(const TLV::TLVReader & aReader, size_t aDepth, EventLoadOutContext * apEventLoadOutContext,
                                    EventEnvelopeContext * event);

    /**
     * @brief Internal iterator function used to fetch event into EventEnvelopeContext, then EventIterator would filter event
     * based upon EventEnvelopeContext
     *
     */
    static CHIP_ERROR FetchEventParameters(const TLV::TLVReader & aReader, size_t aDepth, void * apContext);

    /**
     * @brief Internal iterator function used to scan and filter though event logs
     * First event gets a timestamp, subsequent ones get a delta T
     * First event in the sequence gets a event number neatly packaged
     */
    static CHIP_ERROR CopyAndAdjustDeltaTime(const TLV::TLVReader & aReader, size_t aDepth, void * apContext);

    /**
     * @brief checking if the tail's event can be moved to higher priority, if not, dropped, if yes, note how much space it
     * requires, and return.
     */
    static CHIP_ERROR EvictEvent(chip::TLV::TLVCircularBuffer & aBuffer, void * apAppData, TLV::TLVReader & aReader);
    static CHIP_ERROR AlwaysFail(chip::TLV::TLVCircularBuffer & aBuffer, void * apAppData, TLV::TLVReader & aReader)
    {
        return CHIP_ERROR_NO_MEMORY;
    };

    /**
     * @brief Check whether the event instance represented by the EventEnvelopeContext should be included in the report.
     *
     * @retval CHIP_ERROR_UNEXPECTED_EVENT This path should be excluded in the generated event report.
     * @retval CHIP_EVENT_ID_FOUND This path should be included in the generated event report.
     * @retval CHIP_ERROR_ACCESS_DENIED This path should be included in the generated event report, but the client does not have
     * .       enough privilege to access it.
     *
     * TODO: Consider using CHIP_NO_ERROR, CHIP_ERROR_SKIP_EVENT, CHIP_ERROR_ACCESS_DENINED or some enum to represent the checking
     * result.
     */
    static CHIP_ERROR CheckEventContext(EventLoadOutContext * eventLoadOutContext, const EventEnvelopeContext & event);

    /**
     * @brief copy event from circular buffer to target buffer for report
     */
    static CHIP_ERROR CopyEvent(const TLV::TLVReader & aReader, TLV::TLVWriter & aWriter, EventLoadOutContext * apContext);

    /**
     * @brief
     *   A function to get the circular buffer for particular priority
     *
     * @param aPriority PriorityLevel
     *
     * @return A pointer for the CircularEventBuffer
     */
    CircularEventBuffer * GetPriorityBuffer(PriorityLevel aPriority) const;

    // EventBuffer for debug level,
    CircularEventBuffer * mpEventBuffer        = nullptr;
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    EventManagementStates mState               = EventManagementStates::Shutdown;
    uint32_t mBytesWritten                     = 0;

    // The counter we're going to use for event numbers.
    MonotonicallyIncreasingCounter<EventNumber> * mpEventNumberCounter = nullptr;

    EventNumber mLastEventNumber = 0; ///< Last event Number vended
    Timestamp mLastEventTimestamp;    ///< The timestamp of the last event in this buffer

    System::Clock::Milliseconds64 mMonotonicStartupTime;
};

} // namespace app
} // namespace chip
