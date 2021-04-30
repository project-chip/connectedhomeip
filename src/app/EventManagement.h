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
#include "EventLoggingTypes.h"
#include <app/util/basic-types.h>
#include <core/CHIPCircularTLVBuffer.h>
#include <messaging/ExchangeMgr.h>
#include <support/PersistedCounter.h>

#define CHIP_CONFIG_EVENT_GLOBAL_PRIORITY PriorityLevel::Debug

constexpr size_t kEventSizeRezerve = 196;

namespace chip {
namespace app {

/**
 * @brief
 *   Internal event buffer, built around the chip::TLV::CHIPCircularTLVBuffer
 */

class CircularEventBuffer : public TLV::CHIPCircularTLVBuffer
{
public:
    /**
     * @brief
     *   A constructor for the CircularEventBuffer (internal API).
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
     * @return CircularEventBuffer
     */
    CircularEventBuffer();

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
     */
    void Init(uint8_t * apBuffer, uint32_t aBufferLength, CircularEventBuffer * apPrev, CircularEventBuffer * apNext);

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

    /**
     * @brief
     *   Allocate a new event Number based on the event priority, and advance the counter
     *   if we have one.
     *
     * @return chip::EventNumber Event Number for this priority.
     */
    chip::EventNumber VendEventNumber();

    /**
     * @brief
     *   Remove the number of event
     *
     *   @param[in]   aNumEvents   the number of the event.
     */
    void RemoveEvent(uint16_t aNumEvents);

    /**
     * @brief
     *   Given a timestamp of an event, compute the delta time to store in the log
     *
     * @param aEventTimestamp The event timestamp.
     *
     */
    void AddEvent(Timestamp aEventTimestamp);

    PriorityLevel GetPriorityLevel() { return mPriority; }

    CircularEventBuffer * GetPreviousCircularEventBuffer() { return mpPrev; }
    CircularEventBuffer * GetNextCircularEventBuffer() { return mpNext; }

    virtual ~CircularEventBuffer() = default;

private:
    friend class EventManagement;
    friend class CircularEventReader;

    CircularEventBuffer * mpPrev = nullptr; //< A pointer CircularEventBuffer storing events less important events
    CircularEventBuffer * mpNext = nullptr; //< A pointer CircularEventBuffer storing events more important events

    PriorityLevel mPriority =
        PriorityLevel::Invalid; //< The buffer is the final bucket for events of this priority.  Events of lesser priority are
    //< dropped when they get bumped out of this buffer

    chip::EventNumber mFirstEventNumber = 0; //< First event Number stored in the logging subsystem for this priority
    chip::EventNumber mLastEventNumber  = 0; //< Last event Number vended for this priority

    Timestamp mFirstEventSystemTimestamp; //< The timestamp of the first event in this buffer
    Timestamp mLastEventSystemTimestamp;  //< The timestamp of the last event in this buffer

    // The counter we're going to actually use.
    chip::MonotonicallyIncreasingCounter * mpEventNumberCounter = nullptr;

    // The backup counter to use if no counter is provided for us.
    chip::MonotonicallyIncreasingCounter mNonPersistedCounter;
};

class CircularEventReader;

/**
 * @brief
 *   A CircularEventBufferWrapper which has a pointer to the "current CircularEventBuffer". When trying to locate next buffer,
 *   if nothing left there update its CircularEventBuffer until the buffer with data has been found,
 *   the tlv reader will have a pointer to this impl.
 */
class CircularEventBufferWrapper : public chip::TLV::CHIPCircularTLVBuffer
{
public:
    CircularEventBufferWrapper() : CHIPCircularTLVBuffer(nullptr, 0), mpCurrent(nullptr){};
    CircularEventBuffer * mpCurrent;

private:
    CHIP_ERROR GetNextBuffer(chip::TLV::TLVReader & aReader, const uint8_t *& aBufStart, uint32_t & aBufLen) override;
};

enum class EventManagementStates
{
    Idle       = 1, //< No log offload in progress, log offload can begin without any constraints
    InProgress = 2, //< Log offload in progress
    Shutdown   = 3  //< Not capable of performing any logging operation
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
    void * mpBuffer =
        nullptr; // Buffer to be used as a storage at the particular priority level and shared with more important events.
                 // Must not be nullptr.  Must be large enough to accommodate the largest event emitted by the system.
    size_t mBufferSize = 0; //< The size, in bytes, of the `mBuffer`.
    chip::Platform::PersistedStorage::Key * mCounterKey =
        nullptr;                // Name of the key naming persistent counter for events of this priority.  When NULL, the persistent
                                // counters will not be used for this priority level.
    uint32_t mCounterEpoch = 0; // The interval used in incrementing persistent counters.  When 0, the persistent counters will not
                                // be used for this priority level.
    chip::PersistedCounter * mpCounterStorage =
        nullptr; // application provided storage for persistent counter for this priority level.
    PriorityLevel mPriority =
        PriorityLevel::Invalid; // Log priority level associated with the resources provided in this structure.
};

/**
 * @brief
 *   A class for managing the in memory event logs.
 *   Assume we have two importance levels. DEBUG and CRITICAL, for simplicity,
 *   A new incoming event will always get logged in buffer with debug buffer no matter it is Debug or CRITICAL event.
 *   In order for it to get logged, we need to free up space.  So we look at the tail of the buffer.
 *   If the tail of the buffer contains a DEBUG event, that will be dropped.  If the tail of the buffer contains
 *   a CRITICAL event, that  event will be 'promoted' to the next level of buffers, where it will undergo the same procedure
 *   The outcome of this management policy is that the critical buffer is dedicated to the critical events, and the
 *   debug buffer is shared between critical and debug events.
 */

class EventManagement
{
public:
    /**
     * @brief
     *   EventManagement constructor
     *
     * Initialize the EventManagement with an array of LogStorageResources.  The
     * array must provide a resource for each valid priority level, the elements
     * of the array must be in increasing numerical value of priority (and in
     * increasing priority); the first element in the array corresponds to the
     * resources allocated for least important events, and the last element
     * corresponds to the most critical events.
     *
     * @param[in] apExchangeManager         ExchangeManager to be used with this logging subsystem
     *
     * @param[in] aNumBuffers  Number of elements in inLogStorageResources array
     *
     * @param[in] apCircularEventBuffer  An array of CircularEventBuffer for each priority level.
     *
     * @param[in] apLogStorageResources  An array of LogStorageResources for each priority level.
     *
     */
    EventManagement(Messaging::ExchangeManager * apExchangeManager, int aNumBuffers, CircularEventBuffer * apCircularEventBuffer,
                    const LogStorageResources * const apLogStorageResources);

    void Init(Messaging::ExchangeManager * apExchangeManager, int aNumBuffers, CircularEventBuffer * apCircularEventBuffer,
              const LogStorageResources * const apLogStorageResources);
    /**
     * @brief
     *   EventManagement default constructor. Provided primarily to make the compiler happy.
     *

     * @return EventManagement
     */
    EventManagement(){};

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
     * @note This function must be called prior to the logging being used.
     */
    static void CreateEventManagement(Messaging::ExchangeManager * apExchangeManager, int aNumBuffers,
                                      CircularEventBuffer * apCircularEventBuffer,
                                      const LogStorageResources * const apLogStorageResources);

    static void DestroyEventManagement();

    /**
     * @brief
     *   Log an event via a EventLoggingDelegate, with options.
     *
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
     * @param[in] apOptions    The options for the event metadata.
     *
     * @param[out] aEventNumber The event Number if the event was written to the
     *                         log, 0 otherwise.
     *
     * @return CHIP_ERROR  CHIP Error Code
     */
    CHIP_ERROR LogEvent(EventLoggingDelegate * apDelegate, const EventOptions * apOptions, chip::EventNumber & aEventNumber);

    /**
     * @brief
     *   A helper method to get tlv reader along with buffer has data from particular priority
     *
     * @param[inout] aReader A reference to the reader that will be
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
                              chip::app::CircularEventBufferWrapper * apBufWrapper);

    /**
     * @brief
     *   A function to retrieve events of specified priority since a specified event ID.
     *
     * Given a chip::TLV::TLVWriter, an priority type, and an event ID, the
     * function will fetch events of specified priority since the
     * specified event.  The function will continue fetching events until
     * it runs out of space in the chip::TLV::TLVWriter or in the log. The function
     * will terminate the event writing on event boundary.
     *
     * @param[in] aWriter     The writer to use for event storage
     *
     * @param[in] aPriority The priority of events to be fetched
     *
     * @param[inout] aEventNumber On input, the Event number immediately
     *                         prior to the one we're fetching.  On
     *                         completion, the event number of the last event
     *                         fetched.
     *
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
    CHIP_ERROR FetchEventsSince(chip::TLV::TLVWriter & aWriter, PriorityLevel aPriority, chip::EventNumber & aEventNumber);

    /**
     * @brief
     *  Schedule a log offload task.
     *
     * The function decides whether to schedule a task offload process,
     * and if so, it schedules the `LoggingFlushHandler` to be run
     * asynchronously on the Chip thread.
     *
     * The decision to schedule a flush is dependent on three factors:
     *
     * -- an explicit request to flush the buffer
     *
     * -- the state of the event buffer and the amount of data not yet
     *    synchronized with the event consumers
     *
     * -- whether there is an already pending request flush request event.
     *
     * The explicit request to schedule a flush is passed via an input
     * parameter.
     *
     * The automatic flush is typically scheduled when the event buffers
     * contain enough data to merit starting a new offload.  Additional
     * triggers -- such as minimum and maximum time between offloads --
     * may also be taken into account depending on the offload strategy.
     *
     *
     * @param aFlushRequested A boolean value indicating whether the flush
     *                       should be scheduled regardless of internal
     *                       buffer management policy.
     *
     * @retval #CHIP_ERROR_INCORRECT_STATE EventManagement module was not initialized fully.
     * @retval #CHIP_NO_ERROR              On success.
     */
    CHIP_ERROR ScheduleFlushIfNeeded(bool aFlushRequested);

    /**
     * @brief
     *   Fetch the most recently vended Number for a particular priority level
     *
     * @param aPriority Priority level
     *
     * @return chip::EventNumber most recently vended event Number for that event priority
     */
    chip::EventNumber GetLastEventNumber(PriorityLevel aPriority);

    /**
     * @brief
     *   Fetch the first event Number currently stored for a particular priority level
     *
     * @param aPriority Priority level
     *
     * @return chip::EventNumber First currently stored event Number for that event priority
     */
    chip::EventNumber GetFirstEventNumber(PriorityLevel aPriority);

    /**
     * @brief Helper function for writing event header and data according to event
     *   logging protocol.
     *
     * @param[inout] apContext   EventLoadOutContext, initialized with stateful
     *                          information for the buffer. State is updated
     *                          and preserved by BlitEvent using this context.
     *
     * @param[in] apDelegate The EventLoggingDelegate to serialize the event data
     *
     * @param[in] apOptions     EventOptions describing timestamp and other tags
     *                          relevant to this event.
     *
     */
    CHIP_ERROR BlitEvent(EventLoadOutContext * apContext, EventLoggingDelegate * apDelegate, const EventOptions * apOptions);

    /**
     * @brief Helper function to skip writing an event corresponding to an allocated
     *   event id.
     *
     * @param[inout] apContext   EventLoadOutContext, initialized with stateful
     *                          information for the buffer. State is updated
     *                          and preserved by BlitEvent using this context.
     *
     */
    void SkipEvent(EventLoadOutContext * apContext);

private:
    // Internal function to log event
    CHIP_ERROR LogEventPrivate(EventLoggingDelegate * apDelegate, const EventOptions * apOptions, chip::EventNumber & aEventNumber);

    /**
     * @brief copy the event outright to next buffer with higher priority
     *
     * @param[in] apEventBuffer  CircularEventBuffer
     *
     */
    CHIP_ERROR CopyToNextBuffer(CircularEventBuffer * apEventBuffer);

    /**
     * @brief eusure current buffer has enough space, if not, when current buffer is final destination of last tail's event
     * priority, we need to drop event, otherwises, move the last event to the buffer with higher priority
     *
     * @param[in] aRequiredSpace  require space
     *
     */
    CHIP_ERROR EnsureSpace(size_t aRequiredSpace);

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
    static CHIP_ERROR CopyEventsSince(const chip::TLV::TLVReader & aReader, size_t aDepth, void * apContext);

    /**
     * @brief Internal iterator function used to scan and filter though event logs
     *
     * The function is used to scan through the event log to find events matching the spec in the supplied context.
     * Particularly, it would check against mStartingEventNumber, and skip fetched event.
     */
    static CHIP_ERROR EventIterator(const chip::TLV::TLVReader & aReader, size_t aDepth, void * apContext);

    /**
     * @brief Internal iterator function used to fetch event into EventEnvelopeContext, then EventIterator would filter event
     * based upon EventEnvelopeContext
     *
     */
    static CHIP_ERROR FetchEventParameters(const chip::TLV::TLVReader & aReader, size_t aDepth, void * apContext);

    /**
     * @brief Internal iterator function used to scan and filter though event logs
     * First event gets a timestamp, subsequent ones get a delta T
     * First event in the sequence gets a event number neatly packaged
     */
    static CHIP_ERROR CopyAndAdjustDeltaTime(const chip::TLV::TLVReader & aReader, size_t aDepth, void * apContext);

    /**
     * @brief checking if the tail's event can be moved to higher priority, if not, dropped, if yes, note how much space it
     * requires, and return.
     */
    static CHIP_ERROR EvictEvent(chip::TLV::CHIPCircularTLVBuffer & aBuffer, void * apAppData, chip::TLV::TLVReader & aReader);
    static CHIP_ERROR AlwaysFail(chip::TLV::CHIPCircularTLVBuffer & aBuffer, void * apAppData, chip::TLV::TLVReader & aReader)
    {
        return CHIP_ERROR_NO_MEMORY;
    };

    /**
     * @brief copy event from circular buffer to target buffer for report
     */
    static CHIP_ERROR CopyEvent(const chip::TLV::TLVReader & aReader, chip::TLV::TLVWriter & aWriter,
                                EventLoadOutContext * apContext);

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
    EventManagementStates mState               = EventManagementStates::Idle;
    uint32_t mBytesWritten                     = 0;
};
} // namespace app
} // namespace chip
