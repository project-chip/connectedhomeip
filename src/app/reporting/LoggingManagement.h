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

#include "EventLoggingTypes.h"
#include <core/CHIPCircularTLVBuffer.h>
#include <messaging/ExchangeMgr.h>
#include <support/PersistedCounter.h>
#include <util/basic-types.h>

#define CHIP_CONFIG_EVENT_SIZE_RESERVE 196

namespace chip {
namespace app {
namespace reporting {

/**
 * @brief
 *   Internal event buffer, built around the chip::TLV::CHIPCircularTLVBuffer
 */

class CircularEventBuffer : public TLV::CHIPCircularTLVBuffer
{
public:
    // for doxygen, see the CPP file
    CircularEventBuffer(uint8_t * apBuffer, uint32_t aBufferLength, CircularEventBuffer * apPrev, CircularEventBuffer * apNext);

    CHIP_ERROR GetNextBuffer(chip::TLV::TLVReader & aReader, const uint8_t *& aBufStart, uint32_t & aBufLen) override;

    // for doxygen, see the CPP file
    bool IsFinalDestinationForPriority(PriorityLevel aPriority) const;

    chip::EventNumber VendEventNumber(void);
    void RemoveEvent(uint16_t aNumEvents);

    // for doxygen, see the CPP file
    void AddEvent(timestamp_t aEventTimestamp);

private:
    friend class LoggingManagement;
    friend class CircularEventReader;

    CircularEventBuffer * mpPrev; //< A pointer CircularEventBuffer storing events less important events
    CircularEventBuffer * mpNext; //< A pointer CircularEventBuffer storing events more important events

    PriorityLevel mPriority; //< The buffer is the final bucket for events of this priority.  Events of lesser priority are
                             //< dropped when they get bumped out of this buffer

    chip::EventNumber mFirstEventNumber; //< First event Number stored in the logging subsystem for this priority
    chip::EventNumber mLastEventNumber;  //< Last event Number vended for this priority

    timestamp_t mFirstEventTimestamp; //< The timestamp of the first event in this buffer
    timestamp_t mLastEventTimestamp;  //< The timestamp of the last event in this buffer

    // The counter we're going to actually use.
    chip::MonotonicallyIncreasingCounter * mpEventNumberCounter;

    // The backup counter to use if no counter is provided for us.
    chip::MonotonicallyIncreasingCounter mNonPersistedCounter;
};

/**
 * @brief
 *   A TLVReader backed by CircularEventBuffer
 */
class CircularEventReader : public chip::TLV::TLVReader
{
    friend class CircularEventBuffer;

public:
    void Init(chip::app::reporting::CircularEventBuffer * apBuf);

    virtual ~CircularEventReader() = default;
};

/**
 * @brief
 *  Internal structure for traversing event list.
 */
struct CopyAndAdjustDeltaTimeContext
{
    CopyAndAdjustDeltaTimeContext(chip::TLV::TLVWriter * aWriter, EventLoadOutContext * inContext);

    chip::TLV::TLVWriter * mWriter;
    EventLoadOutContext * mContext;
};

/**
 * @brief
 *  Internal structure for traversing events.
 */
struct EventEnvelopeContext
{
    EventEnvelopeContext(void);

    size_t mNumFieldsToRead;
    uint32_t mDeltaTime;
    int64_t mDeltaUtc;
    PriorityLevel mPriority;
};

enum LoggingManagementStates
{
    kLoggingManagementState_Idle       = 1, //< No log offload in progress, log offload can begin without any constraints
    kLoggingManagementState_InProgress = 2, //< Log offload in progress
    kLoggingManagementState_Shutdown   = 3  //< Not capable of performing any logging operation
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
    void * mpBuffer;    //< Buffer to be used as a storage at the particular priority level and shared with more important events.
                        //< Must not be NULL.  Must be large enough to accommodate the largest event emitted by the system.
    size_t mBufferSize; //< The size, in bytes, of the `mBuffer`.
    chip::Platform::PersistedStorage::Key *
        mCounterKey;        //< Name of the key naming persistent counter for events of this priority.  When NULL, the persistent
                            //< counters will not be used for this priority level.
    uint32_t mCounterEpoch; //< The interval used in incrementing persistent counters.  When 0, the persistent counters will not be
                            //< used for this priority level.
    chip::PersistedCounter * mCounterStorage; //< Application-provided storage for persistent counter for this priority level.
                                              //< When NULL, persistent counters will not be used for this priority level.
    PriorityLevel mPriority;                  //< Log priority level associated with the resources provided in this structure.
};

/**
 * @brief
 *   A class for managing the in memory event logs.
 */

class LoggingManagement
{
public:
    LoggingManagement(Messaging::ExchangeManager * apMgr, size_t aNumBuffers,
                      const LogStorageResources * const apLogStorageResources);
    LoggingManagement(void);

    static LoggingManagement & GetInstance(void);

    static void CreateLoggingManagement(Messaging::ExchangeManager * apMgr, size_t aNumBuffers,
                                        const LogStorageResources * const apLogStorageResources);

    static void DestroyLoggingManagement(void);

    CHIP_ERROR SetExchangeManager(Messaging::ExchangeManager * apMgr);

    chip::EventNumber LogEvent(const EventSchema & aSchema, EventWriterFunct aEventWriter, void * apAppData,
                           const EventOptions * apOptions);

    CHIP_ERROR GetEventReader(chip::TLV::TLVReader & aReader, PriorityLevel aPriority);

    CHIP_ERROR FetchEventsSince(chip::TLV::TLVWriter & aWriter, PriorityLevel aPriority, chip::EventNumber & aEventNumber);

    CHIP_ERROR ScheduleFlushIfNeeded(bool aFlushRequested);

    CHIP_ERROR SetLoggingEndpoint(chip::EventNumber * aEventEndpoints, size_t aNumPriorityLevels, size_t & aLoggingPosition);

    uint32_t GetBytesWritten(void) const;

    void NotifyEventsDelivered(PriorityLevel aPriority, chip::EventNumber aLastDeliveredEventNumber, uint64_t aRecipientNodeID);

    /**
     * @brief
     *   IsValid returns whether the LoggingManagement instance is valid
     *
     * @retval true  The instance is valid (initialized with the appropriate backing store)
     * @retval false Otherwise
     */
    bool IsValid(void) { return (mpEventBuffer != NULL); };

    chip::EventNumber GetLastEventNumber(PriorityLevel aPriority);
    chip::EventNumber GetFirstEventNumber(PriorityLevel aPriority);

    CHIP_ERROR BlitEvent(EventLoadOutContext * apContext, const EventSchema & aSchema, EventWriterFunct aEventWriter,
                         void * apAppData, const EventOptions * apOptions);
    void SkipEvent(EventLoadOutContext * apContext);

private:
    chip::EventNumber LogEventPrivate(const EventSchema & aSchema, EventWriterFunct aEventWriter, void * apAppData,
                                  const EventOptions * apOptions);

    void FlushHandler(System::Layer * inSystemLayer, INET_ERROR inErr);
    void SignalUploadDone(void);
    CHIP_ERROR CopyToNextBuffer(CircularEventBuffer * apEventBuffer);
    CHIP_ERROR EnsureSpace(size_t aRequiredSpace);

    static CHIP_ERROR CopyEventsSince(const chip::TLV::TLVReader & aReader, size_t aDepth, void * apContext);
    static CHIP_ERROR EventIterator(const chip::TLV::TLVReader & aReader, size_t aDepth, void * apContext);
    static CHIP_ERROR FetchEventParameters(const chip::TLV::TLVReader & aReader, size_t aDepth, void * apContext);
    static CHIP_ERROR CopyAndAdjustDeltaTime(const chip::TLV::TLVReader & aReader, size_t aDepth, void * apContext);
    static CHIP_ERROR EvictEvent(chip::TLV::CHIPCircularTLVBuffer & aBuffer, void * apAppData, chip::TLV::TLVReader & aReader);
    static CHIP_ERROR AlwaysFail(chip::TLV::CHIPCircularTLVBuffer & aBuffer, void * apAppData, chip::TLV::TLVReader & aReader);
    static CHIP_ERROR CopyEvent(const chip::TLV::TLVReader & aReader, chip::TLV::TLVWriter & aWriter,
                                EventLoadOutContext * apContext);

    static void LoggingFlushHandler(System::Layer * aSystemLayer, void * apAppState, INET_ERROR err);

    PriorityLevel GetMaxPriority(void);
    PriorityLevel GetCurrentPriority(chip::ClusterId aClusterId);

private:
    CircularEventBuffer * GetPriorityBuffer(PriorityLevel aPriority) const;
    CircularEventBuffer * mpEventBuffer;
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    LoggingManagementStates mState;
    uint32_t mBytesWritten;
    PriorityLevel mMaxPriorityBuffer;
    bool mUploadRequested;
};
} // namespace reporting
} // namespace app
} // namespace chip
