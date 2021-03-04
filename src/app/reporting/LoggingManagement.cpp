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

#include "LoggingManagement.h"
#include "LoggingConfiguration.h"
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventDataElement.h>
#include <core/CHIPEventLoggingConfig.h>
#include <core/CHIPTLVUtilities.hpp>
#include <inttypes.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemTimer.h>

using namespace chip::TLV;

namespace chip {
namespace Platform {
// TODO: Implement CriticalSectionEnter per platform
void CriticalSectionEnter() {}

void CriticalSectionExit() {}
} // namespace Platform
} // namespace chip

namespace chip {
namespace app {
namespace reporting {

static LoggingManagement sInstance;

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
    CircularEventBuffer(uint8_t * apBuffer, uint32_t aBufferLength, CircularEventBuffer * apPrev, CircularEventBuffer * apNext);

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
     *   Allocate a new event ID based on the event priority, and advance the counter
     *   if we have one.
     *
     * @return chip::EventNumber Event Number for this priority.
     */
    chip::EventNumber VendEventNumber(void);

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
     * @return int32_t         Time delta to encode for the event.
     */
    void AddEvent(timestamp_t aEventTimestamp);

    PriorityLevel GetPriorityLevel() { return mPriority; }

    CircularEventBuffer * GetPreviousCircularEventBuffer() { return mpPrev; }
    CircularEventBuffer * GetNextCircularEventBuffer() { return mpNext; }

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
public:
    /**
     * @brief
     *   Initializes a TLVReader object backed by CircularEventBuffer
     *
     * Reading begins in the CircularTLVBuffer belonging to this
     * CircularEventBuffer.  When the reader runs out of data, it begins
     * to read from the previous CircularEventBuffer.
     *
     * @param[in] apBuf A pointer to a fully initialized CircularEventBuffer
     *
     */
    void Init(chip::app::reporting::CircularEventBufferWrapper * apBuf);

    virtual ~CircularEventReader() = default;
};

LoggingManagement & LoggingManagement::GetInstance(void)
{
    return sInstance;
}

struct ReclaimEventCtx
{
    CircularEventBuffer * mpEventBuffer;
    size_t mSpaceNeededForEvent;
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

CHIP_ERROR LoggingManagement::AlwaysFail(chip::TLV::CHIPCircularTLVBuffer & apBuffer, void * apAppData,
                                         chip::TLV::TLVReader & inReader)
{
    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR LoggingManagement::CopyToNextBuffer(CircularEventBuffer * apEventBuffer)
{
    CircularTLVWriter writer;
    CircularTLVReader reader;
    CircularEventBuffer checkpoint   = *(apEventBuffer->mpNext);
    CircularEventBuffer * nextBuffer = apEventBuffer->mpNext;
    CHIP_ERROR err;

    // Set up the next buffer s.t. it fails if needs to evict an element
    nextBuffer->mProcessEvictedElement = AlwaysFail;

    writer.Init(*nextBuffer);

    // Set up the reader s.t. it is positioned to read the head event
    reader.Init(*apEventBuffer);

    err = reader.Next();
    SuccessOrExit(err);

    err = writer.CopyElement(reader);
    SuccessOrExit(err);

    err = writer.Finalize();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        *(apEventBuffer->mpNext) = checkpoint;
    }
    return err;
}

CHIP_ERROR LoggingManagement::EnsureSpace(size_t aRequiredSpace)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    size_t requiredSpace              = aRequiredSpace;
    CircularEventBuffer * eventBuffer = mpEventBuffer;
    ReclaimEventCtx ctx;

    // check whether we actually need to do anything, exit if we don't
    VerifyOrExit(requiredSpace > eventBuffer->AvailableDataLength(), err = CHIP_NO_ERROR);

    while (true)
    {
        // check that the request can ultimately be satisfied.
        VerifyOrExit(requiredSpace <= eventBuffer->GetQueueSize(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

        if (requiredSpace > eventBuffer->AvailableDataLength())
        {
            ctx.mpEventBuffer        = eventBuffer;
            ctx.mSpaceNeededForEvent = 0;

            eventBuffer->mProcessEvictedElement = EvictEvent;
            eventBuffer->mAppData               = &ctx;
            err                                 = eventBuffer->EvictHead();

            // one of two things happened: either the element was evicted,
            // or we figured out how much space we need to evict it into
            // the next buffer

            if (err != CHIP_NO_ERROR)
            {
                VerifyOrExit(ctx.mSpaceNeededForEvent != 0, /* no-op, return err */);
                if (ctx.mSpaceNeededForEvent <= eventBuffer->mpNext->AvailableDataLength())
                {
                    // we can copy the event outright.  copy event and
                    // subsequently evict head s.t. evicting the head
                    // element always succeeds.
                    // Since we're calling CopyElement and we've checked
                    // that there is space in the next buffer, we don't expect
                    // this to fail.
                    err = CopyToNextBuffer(eventBuffer);
                    SuccessOrExit(err);
                    // success; evict head unconditionally
                    eventBuffer->mProcessEvictedElement = NULL;
                    err                                 = eventBuffer->EvictHead();
                    // if unconditional eviction failed, this
                    // means that we have no way of further
                    // clearing the buffer.  fail out and let the
                    // caller know that we could not honor the
                    // request
                    SuccessOrExit(err);
                    continue;
                }
                // we cannot copy event outright. We remember the
                // current required space in mpAppData, we note the
                // space requirements for the event in the current
                // buffer and make that space in the next buffer.
                eventBuffer->mAppData = reinterpret_cast<void *>(requiredSpace);
                eventBuffer           = eventBuffer->mpNext;

                // Sanity check: Die here on null event buffer.  If
                // eventBuffer->mpNext were null, then the `EvictBuffer`
                // in line 130 would have succeeded -- the event was
                // already in the final buffer.
                VerifyOrDie(eventBuffer != NULL);

                requiredSpace = ctx.mSpaceNeededForEvent;
            }
        }
        else
        {
            if (eventBuffer == mpEventBuffer)
                break;
            eventBuffer   = eventBuffer->mpPrev;
            requiredSpace = reinterpret_cast<size_t>(eventBuffer->mAppData);
            err           = CHIP_NO_ERROR;
        }
    }

    // On exit, configure the top-level s.t. it will always fail to evict an element
    mpEventBuffer->mProcessEvictedElement = AlwaysFail;
    mpEventBuffer->mAppData               = NULL;

exit:
    return err;
}

CHIP_ERROR LoggingManagement::BlitEvent(EventLoadOutContext * apContext, const EventSchema & aSchema,
                                        EventLoggingDelegate * apDelegate, const EventOptions * apOptions)
{

    CHIP_ERROR err       = CHIP_NO_ERROR;
    TLVWriter checkpoint = apContext->mWriter;
    chip::TLV::TLVType dataContainerType;
    chip::app::EventDataElement::Builder eventDataElementBuilder;
    chip::app::EventPath::Builder eventPathBuilder;

    VerifyOrExit(apContext->mCurrentEventNumber >= apContext->mStartingEventNumber,
                 /* no-op: don't write event, but advance current event Number */);

    VerifyOrExit(apOptions != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(apOptions->timestampType != TimestampType::Invalid, err = CHIP_ERROR_INVALID_ARGUMENT);

    eventDataElementBuilder.Init(&(apContext->mWriter));
    eventPathBuilder = eventDataElementBuilder.CreateEventPathBuilder();
    SuccessOrExit(eventPathBuilder.GetError());

    // TODO: Revisit NodeId since the the encoding spec and the IM seem to disagree on how this stuff works
    eventPathBuilder.NodeId(apOptions->eventSource->mNodeId)
        .EndpointId(apOptions->eventSource->mEndpointId)
        .ClusterId(aSchema.mClusterId)
        .EventId(aSchema.mEventId)
        .EndOfEventPath();
    SuccessOrExit(eventPathBuilder.GetError());

    eventDataElementBuilder.PriorityLevel(static_cast<uint8_t>(apContext->mPriority));

    // if mFirst, record absolute time
    if (apContext->mFirst)
    {
        eventDataElementBuilder.Number(apContext->mCurrentEventNumber);
        eventDataElementBuilder.SystemTimestamp(apOptions->timestamp.systemTimestamp);
    }
    // else record delta
    else
    {
        uint32_t deltatime = apOptions->timestamp.systemTimestamp - apContext->mCurrentTime;
        eventDataElementBuilder.DeltaSystemTimestamp(deltatime);
    }

    SuccessOrExit(eventDataElementBuilder.GetError());

    err = apContext->mWriter.StartContainer(ContextTag(chip::app::EventDataElement::kCsTag_Data), chip::TLV::kTLVType_Structure,
                                            dataContainerType);
    SuccessOrExit(err);
    // Callback to write the EventData
    err = apDelegate->WriteEvent(apContext->mWriter);
    SuccessOrExit(err);

    err = apContext->mWriter.EndContainer(dataContainerType);
    SuccessOrExit(err);

    eventDataElementBuilder.EndOfEventDataElement();
    SuccessOrExit(eventDataElementBuilder.GetError());

    err = apContext->mWriter.Finalize();
    SuccessOrExit(err);

    // only update mFirst if an event was successfully written.
    if (apContext->mFirst)
    {
        apContext->mFirst = false;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        apContext->mWriter = checkpoint;
    }
    else
    {
        // update these variables since BlitEvent can be used to track the
        // state of a set of events over multiple calls.
        apContext->mCurrentEventNumber++;
        {
            apContext->mCurrentTime = apOptions->timestamp.systemTimestamp;
        }
    }
    return err;
}

void LoggingManagement::SkipEvent(EventLoadOutContext * apContext)
{
    apContext->mCurrentEventNumber++; // Advance the event id without writing anything
}

void LoggingManagement::CreateLoggingManagement(Messaging::ExchangeManager * apMgr, size_t aNumBuffers,
                                                const LogStorageResources * const apLogStorageResources)
{
    new (&sInstance) LoggingManagement(apMgr, aNumBuffers, apLogStorageResources);
}

/**
 * @brief Perform any actions we need to on shutdown.
 */
void LoggingManagement::DestroyLoggingManagement(void)
{
    Platform::CriticalSectionEnter();
    sInstance.mState        = LoggingManagementStates::Shutdown;
    sInstance.mpEventBuffer = NULL;
    Platform::CriticalSectionExit();
}

LoggingManagement::LoggingManagement(Messaging::ExchangeManager * apMgr, size_t aNumBuffers,
                                     const LogStorageResources * const apLogStorageResources)
{
    CircularEventBuffer * current = nullptr;
    CircularEventBuffer * prev    = nullptr;
    CircularEventBuffer * next    = nullptr;
    size_t bufferIndex, j;

    VerifyOrDie(aNumBuffers > 0);

    mpExchangeMgr = apMgr;

    for (j = 0; j < aNumBuffers; j++)
    {
        bufferIndex = aNumBuffers - 1 - j;

        next = (bufferIndex > 0) ? static_cast<CircularEventBuffer *>(apLogStorageResources[bufferIndex - 1].mpBuffer) : NULL;

        VerifyOrDie(apLogStorageResources[bufferIndex].mBufferSize > sizeof(CircularEventBuffer));

        new (apLogStorageResources[bufferIndex].mpBuffer) CircularEventBuffer(
            static_cast<uint8_t *>(apLogStorageResources[bufferIndex].mpBuffer) + sizeof(CircularEventBuffer),
            (uint32_t)(apLogStorageResources[bufferIndex].mBufferSize - sizeof(CircularEventBuffer)), prev, next);

        current = prev                  = static_cast<CircularEventBuffer *>(apLogStorageResources[bufferIndex].mpBuffer);
        current->mProcessEvictedElement = AlwaysFail;
        current->mAppData               = NULL;
        current->mPriority              = apLogStorageResources[bufferIndex].mPriority;
        if ((apLogStorageResources[bufferIndex].mCounterStorage != NULL) &&
            (apLogStorageResources[bufferIndex].mCounterKey != NULL) && (apLogStorageResources[bufferIndex].mCounterEpoch != 0))
        {
            // We have been provided storage for a counter for this priority level.
            new (apLogStorageResources[bufferIndex].mCounterStorage) PersistedCounter();
            CHIP_ERROR err = apLogStorageResources[bufferIndex].mCounterStorage->Init(
                *(apLogStorageResources[bufferIndex].mCounterKey), apLogStorageResources[bufferIndex].mCounterEpoch);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(EventLogging, "%s PersistedCounter[%d]->Init() failed with %d", __FUNCTION__, j, err);
            }
            current->mpEventNumberCounter = apLogStorageResources[bufferIndex].mCounterStorage;
        }
        else
        {
            // No counter has been provided, so we'll use our
            // "built-in" non-persisted counter.
            current->mNonPersistedCounter.Init(1);
            current->mpEventNumberCounter = &(current->mNonPersistedCounter);
        }

        current->mFirstEventNumber = (chip::EventNumber) current->mpEventNumberCounter->GetValue();
    }
    mpEventBuffer = static_cast<CircularEventBuffer *>(apLogStorageResources[CHIP_NUM_PRIORITY_LEVEL-1].mpBuffer);

    mState        = LoggingManagementStates::Idle;
    mBytesWritten = 0;
}

LoggingManagement::LoggingManagement(void) :
    mpEventBuffer(NULL), mpExchangeMgr(NULL), mState(LoggingManagementStates::Idle), mBytesWritten(0)
{}

PriorityLevel LoggingManagement::GetCurrentPriority(chip::ClusterId aClusterId)
{
    const LoggingConfiguration & config = LoggingConfiguration::GetInstance();
    PriorityLevel retval;

    if (config.SupportsPerClusterPriority())
    {
        retval = config.GetClusterPriority(aClusterId);
    }
    else
    {
        retval = config.GetGlobalPriorityLevel();
    }
    return retval;
}

chip::EventNumber CircularEventBuffer::VendEventNumber(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Assign event Number to the buffer's counter's value.
    mLastEventNumber = static_cast<chip::EventNumber>(mpEventNumberCounter->GetValue());

    // Now advance the counter.
    err = mpEventNumberCounter->Advance();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "%s Advance() for priority %d failed with %d", __FUNCTION__, mPriority, err);
    }

    return mLastEventNumber;
}

chip::EventNumber LoggingManagement::GetLastEventNumber(PriorityLevel aPriority)
{
    return GetPriorityBuffer(aPriority)->mLastEventNumber;
}

chip::EventNumber LoggingManagement::GetFirstEventNumber(PriorityLevel aPriority)
{
    return GetPriorityBuffer(aPriority)->mFirstEventNumber;
}

CircularEventBuffer * LoggingManagement::GetPriorityBuffer(PriorityLevel aPriority) const
{
    CircularEventBuffer * buf = mpEventBuffer;
    while (!buf->IsFinalDestinationForPriority(aPriority))
    {
        buf = buf->mpNext;
    }
    return buf;
}

CHIP_ERROR LoggingManagement::CopyAndAdjustDeltaTime(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err;
    CopyAndAdjustDeltaTimeContext * ctx = static_cast<CopyAndAdjustDeltaTimeContext *>(apContext);
    TLVReader reader(aReader);

    // TODO: Add UTC timestamp support
    if (aReader.GetTag() == chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_DeltaSystemTimestamp))
    {
        if (ctx->mContext->mFirst) // First event gets a timestamp, subsequent ones get a delta T
        {
            err = ctx->mWriter->Put(chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_SystemTimestamp),
                                    ctx->mContext->mCurrentTime);
        }
        else
        {
            err = ctx->mWriter->CopyElement(reader);
        }
    }
    else
    {
        err = ctx->mWriter->CopyElement(reader);
    }

    // First event in the sequence gets a event number neatly packaged
    // right after the priority to keep tags ordered
    if (aReader.GetTag() == chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_PriorityLevel))
    {
        if (ctx->mContext->mFirst)
        {
            err =
                ctx->mWriter->Put(TLV::ContextTag(chip::app::EventDataElement::kCsTag_Number), ctx->mContext->mCurrentEventNumber);
        }
    }

    return err;
}

chip::EventNumber LoggingManagement::LogEvent(const EventSchema & aSchema, EventLoggingDelegate * apDelegate,
                                              const EventOptions * apOptions)
{
    chip::EventNumber event_number = 0;
    Platform::CriticalSectionEnter();
    // Make sure we're alive.
    VerifyOrExit(mState != LoggingManagementStates::Shutdown, /* no-op */);

    event_number = LogEventPrivate(aSchema, apDelegate, apOptions);

exit:
    Platform::CriticalSectionExit();
    return event_number;
}

inline chip::EventNumber LoggingManagement::LogEventPrivate(const EventSchema & aSchema, EventLoggingDelegate * apDelegate,
                                                            const EventOptions * apOptions)
{
    chip::EventNumber event_number = 0;
    CircularTLVWriter writer;
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    size_t requestSize             = CHIP_CONFIG_EVENT_SIZE_RESERVE;
    bool didWriteEvent             = false;
    CircularEventBuffer checkpoint = *mpEventBuffer;
    // GetPriorityBuffer(aSchema.mPriority)
    EventLoadOutContext ctxt =
        EventLoadOutContext(writer, aSchema.mPriority, GetPriorityBuffer(aSchema.mPriority)->mLastEventNumber);
    EventOptions opts = EventOptions(static_cast<timestamp_t>(System::Timer::GetCurrentEpoch()));

    // check whether the entry is to be logged or discarded silently
    VerifyOrExit(aSchema.mPriority <= GetCurrentPriority(aSchema.mClusterId), /* no-op */);

    // Create all event specific data
    // Timestamp; encoded as a delta time
    if ((apOptions != NULL) && (apOptions->timestampType == TimestampType::System))
    {
        opts.timestamp.systemTimestamp = apOptions->timestamp.systemTimestamp;
    }

    if (GetPriorityBuffer(aSchema.mPriority)->mFirstEventTimestamp == 0)
    {
        GetPriorityBuffer(aSchema.mPriority)->AddEvent(opts.timestamp.systemTimestamp);
    }

    if (apOptions != NULL)
    {
        opts.eventSource = apOptions->eventSource;
    }

    ctxt.mFirst              = false;
    ctxt.mCurrentEventNumber = GetPriorityBuffer(aSchema.mPriority)->mLastEventNumber;
    ctxt.mCurrentTime        = GetPriorityBuffer(aSchema.mPriority)->mLastEventTimestamp;

    // Begin writing
    while (!didWriteEvent)
    {
        // Ensure we have space in the in-memory logging queues
        err = EnsureSpace(requestSize);
        // If we fail to ensure the initial reserve size, then the
        // logging subsystem will never be able to make progress.  In
        // that case, it is best to assert
        if ((requestSize == CHIP_CONFIG_EVENT_SIZE_RESERVE) && (err != CHIP_NO_ERROR))
            chipDie();
        SuccessOrExit(err);

        // save a checkpoint for the underlying buffer.  Note that with
        // the current event buffering scheme, only the mpEventBuffer will
        // be affected by the writes to the `writer` below, and thus
        // that's the only thing we need to checkpoint.
        checkpoint = *mpEventBuffer;

        // Start the event container (anonymous structure) in the circular buffer
        writer.Init(*mpEventBuffer);

        err = BlitEvent(&ctxt, aSchema, apDelegate, &opts);

        if (err == CHIP_ERROR_NO_MEMORY)
        {
            // try again
            err = CHIP_NO_ERROR;
            requestSize += CHIP_CONFIG_EVENT_SIZE_INCREMENT;
            *mpEventBuffer = checkpoint;
            continue;
        }

        didWriteEvent = true;
    }

    {
        // Check the number of bytes written.  If the event is loo large
        // to be evicted from subsequent buffers, drop it now.
        CircularEventBuffer * buffer = mpEventBuffer;
        do
        {
            VerifyOrExit(buffer->GetQueueSize() >= writer.GetLengthWritten(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
            if (buffer->IsFinalDestinationForPriority(aSchema.mPriority))
                break;
            else
                buffer = buffer->mpNext;
        } while (true);
    }

    mBytesWritten += writer.GetLengthWritten();

exit:

    if (err != CHIP_NO_ERROR)
    {
        *mpEventBuffer = checkpoint;
    }
    else if (aSchema.mPriority <= GetCurrentPriority(aSchema.mClusterId))
    {
        CircularEventBuffer * currentBuffer = GetPriorityBuffer(aSchema.mPriority);
        event_number                        = currentBuffer->VendEventNumber();
        currentBuffer->AddEvent(opts.timestamp.systemTimestamp);

#if CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
        ChipLogDetail(EventLogging,
                      "LogEvent event number: %u schema priority: %u cluster id: 0x%x event id: 0x%x sys timestamp: 0x%" PRIx32,
                      event_number, aSchema.mPriority, aSchema.mClusterId, aSchema.mEventId, opts.timestamp.systemTimestamp);
#endif // CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS

        ScheduleFlushIfNeeded(apOptions == NULL ? false : apOptions->urgent);
    }

    return event_number;
}

CHIP_ERROR LoggingManagement::CopyEvent(const TLVReader & aReader, TLVWriter & aWriter, EventLoadOutContext * apContext)
{
    TLVReader reader;
    TLVType containerType;
    CopyAndAdjustDeltaTimeContext context(&aWriter, apContext);
    const bool recurse = false;
    CHIP_ERROR err     = CHIP_NO_ERROR;

    reader.Init(aReader);
    err = reader.EnterContainer(containerType);
    SuccessOrExit(err);

    err = aWriter.StartContainer(AnonymousTag, kTLVType_Structure, containerType);
    SuccessOrExit(err);

    err = chip::TLV::Utilities::Iterate(reader, CopyAndAdjustDeltaTime, &context, recurse);
    VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV, );

    err = aWriter.EndContainer(containerType);
    SuccessOrExit(err);

    err = aWriter.Finalize();
    SuccessOrExit(err);

    VerifyOrExit(kMinEventBoundarySize <= aWriter.GetRemainingFreeLength(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
exit:
    return err;
}

CHIP_ERROR LoggingManagement::EventIterator(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    const bool recurse = false;
    TLVReader innerReader;
    TLVType tlvType;
    EventEnvelopeContext event;
    EventLoadOutContext * loadOutContext = static_cast<EventLoadOutContext *>(apContext);

    innerReader.Init(aReader);
    err = innerReader.EnterContainer(tlvType);
    SuccessOrExit(err);

    err = chip::TLV::Utilities::Iterate(innerReader, FetchEventParameters, &event, recurse);
    VerifyOrExit(event.mNumFieldsToRead == 0, err = CHIP_NO_ERROR);

    err = CHIP_NO_ERROR;

    if (event.mPriority == loadOutContext->mPriority)
    {
        loadOutContext->mCurrentTime += event.mDeltaTime;
        VerifyOrExit(loadOutContext->mCurrentEventNumber < loadOutContext->mStartingEventNumber, err = CHIP_EVENT_ID_FOUND);
        loadOutContext->mCurrentEventNumber++;
    }

exit:
    return err;
}

CHIP_ERROR LoggingManagement::CopyEventsSince(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter checkpoint;
    EventLoadOutContext * loadOutContext = static_cast<EventLoadOutContext *>(apContext);
    err                                  = EventIterator(aReader, aDepth, apContext);
    if (err == CHIP_EVENT_ID_FOUND)
    {
        // checkpoint the writer
        checkpoint = loadOutContext->mWriter;

        err = CopyEvent(aReader, loadOutContext->mWriter, loadOutContext);

        // CHIP_NO_ERROR and CHIP_END_OF_TLV signify a
        // successful copy.  In all other cases, roll back the
        // writer state back to the checkpoint, i.e., the state
        // before we began the copy operation.
        VerifyOrExit((err == CHIP_NO_ERROR) || (err == CHIP_END_OF_TLV), loadOutContext->mWriter = checkpoint);

        loadOutContext->mCurrentTime = 0;
        loadOutContext->mFirst       = false;
        loadOutContext->mCurrentEventNumber++;
    }

exit:
    return err;
}

CHIP_ERROR LoggingManagement::FetchEventsSince(TLVWriter & aWriter, PriorityLevel aPriority, chip::EventNumber & aEventNumber)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    const bool recurse = false;
    TLVReader reader;
    CircularEventBufferWrapper bufWrapper;
    EventLoadOutContext context(aWriter, aPriority, aEventNumber);

    CircularEventBuffer * buf = mpEventBuffer;
    Platform::CriticalSectionEnter();

    while (!buf->IsFinalDestinationForPriority(aPriority))
    {
        buf = buf->mpNext;
    }

    context.mCurrentTime        = buf->mFirstEventTimestamp;
    context.mCurrentEventNumber = buf->mFirstEventNumber;
    err                         = GetEventReader(reader, aPriority, &bufWrapper);
    SuccessOrExit(err);

    err = chip::TLV::Utilities::Iterate(reader, CopyEventsSince, &context, recurse);

exit:
    aEventNumber = context.mCurrentEventNumber;
    Platform::CriticalSectionExit();

    return err;
}

CHIP_ERROR LoggingManagement::GetEventReader(TLVReader & aReader, PriorityLevel aPriority,
                                             chip::app::reporting::CircularEventBufferWrapper * apBufWrapper)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CircularEventReader reader;
    CircularEventBuffer * buffer = GetPriorityBuffer(aPriority);
    VerifyOrExit(buffer != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
    apBufWrapper->mpCurrent = buffer;
    reader.Init(apBufWrapper);
    aReader.Init(reader);
exit:
    return err;
}

CHIP_ERROR LoggingManagement::FetchEventParameters(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err                  = CHIP_NO_ERROR;
    EventEnvelopeContext * envelope = static_cast<EventEnvelopeContext *>(apContext);
    TLVReader reader;
    uint16_t extPriority; // Note: the type here matches the type case in LoggingManagement::LogEvent, priority section
    reader.Init(aReader);

    VerifyOrExit(envelope->mNumFieldsToRead > 0, err = CHIP_END_OF_TLV);

    if (reader.GetTag() == chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_PriorityLevel))
    {
        err = reader.Get(extPriority);
        SuccessOrExit(err);
        envelope->mPriority = static_cast<PriorityLevel>(extPriority);

        envelope->mNumFieldsToRead--;
    }

    if (reader.GetTag() == chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_DeltaSystemTimestamp))
    {
        err = reader.Get(envelope->mDeltaTime);
        SuccessOrExit(err);

        envelope->mNumFieldsToRead--;
    }

exit:
    return err;
}

CHIP_ERROR LoggingManagement::EvictEvent(CHIPCircularTLVBuffer & apBuffer, void * apAppData, TLVReader & inReader)
{
    ReclaimEventCtx * ctx             = static_cast<ReclaimEventCtx *>(apAppData);
    CircularEventBuffer * eventBuffer = ctx->mpEventBuffer;
    TLVType containerType;
    EventEnvelopeContext context;
    const bool recurse = false;
    CHIP_ERROR err;
    PriorityLevel imp = PriorityLevel::Invalid;

    // pull out the delta time, pull out the priority
    err = inReader.Next();
    SuccessOrExit(err);

    err = inReader.EnterContainer(containerType);
    SuccessOrExit(err);

    chip::TLV::Utilities::Iterate(inReader, FetchEventParameters, &context, recurse);

    err = inReader.ExitContainer(containerType);

    SuccessOrExit(err);

    imp = static_cast<PriorityLevel>(context.mPriority);

    if (eventBuffer->IsFinalDestinationForPriority(imp))
    {
        // event is getting dropped.  Increase the eventid and first timestamp.
        uint16_t numEventsToDrop = 1;

        eventBuffer->RemoveEvent(numEventsToDrop);
        eventBuffer->mFirstEventTimestamp += context.mDeltaTime;
        ChipLogProgress(EventLogging, "Dropped events due to overflow: { priority_level: %d, count: %d };", imp, numEventsToDrop);
        ctx->mSpaceNeededForEvent = 0;
    }
    else
    {
        // event is not getting dropped. Note how much space it requires, and return.
        ctx->mSpaceNeededForEvent = inReader.GetLengthRead();
        err                       = CHIP_END_OF_TLV;
    }

exit:
    return err;
}

void LoggingManagement::FlushHandler(System::Layer * inSystemLayer, INET_ERROR inErr)
{
    switch (mState)
    {

    case LoggingManagementStates::Idle: {
        if (mpExchangeMgr != NULL)
        {
            chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine()->Run();
        }

        break;
    }

    case LoggingManagementStates::InProgress:
    case LoggingManagementStates::Shutdown: {
        // should never end in these states in this function
        break;
    }
    }
}

CHIP_ERROR LoggingManagement::ScheduleFlushIfNeeded(bool inRequestFlush)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // TODO: Implement ScheduleFlushIfNeeded
    return err;
}

CHIP_ERROR LoggingManagement::SetLoggingEndpoint(chip::EventNumber * apEventEndpoints, size_t aNumPriorityLevels)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    CircularEventBuffer * eventBuffer = mpEventBuffer;

    Platform::CriticalSectionEnter();

    while (eventBuffer != NULL && aNumPriorityLevels > 0)
    {
        if ((eventBuffer->mPriority >= PriorityLevel::First) && (static_cast<size_t>(eventBuffer->mPriority) - static_cast<size_t>(PriorityLevel::First) < aNumPriorityLevels))
        {
            apEventEndpoints[static_cast<int>(eventBuffer->mPriority) - static_cast<int>(PriorityLevel::First)] = eventBuffer->mLastEventNumber;
        }
        eventBuffer = eventBuffer->mpNext;
    }

    Platform::CriticalSectionExit();

    return err;
}

uint32_t LoggingManagement::GetBytesWritten(void) const
{
    return mBytesWritten;
}

void LoggingManagement::NotifyEventsDelivered(PriorityLevel aPriority, chip::EventNumber inLastDeliveredEventNumber,
                                              uint64_t inRecipientNodeID)
{}

CircularEventBuffer::CircularEventBuffer(uint8_t * apBuffer, uint32_t aBufferLength, CircularEventBuffer * apPrev,
                                         CircularEventBuffer * apNext) :
    CHIPCircularTLVBuffer(apBuffer, aBufferLength),
    mpPrev(apPrev), mpNext(apNext), mPriority(PriorityLevel::First), mFirstEventNumber(1), mLastEventNumber(0),
    mFirstEventTimestamp(0), mLastEventTimestamp(0), mpEventNumberCounter(nullptr)
{}

bool CircularEventBuffer::IsFinalDestinationForPriority(PriorityLevel aPriority) const
{
    return !((mpNext != NULL) && (mpNext->mPriority >= aPriority));
}

void CircularEventBuffer::AddEvent(chip::app::reporting::timestamp_t aEventTimestamp)
{
    if (mFirstEventTimestamp == 0)
    {
        mFirstEventTimestamp = aEventTimestamp;
        mLastEventTimestamp  = aEventTimestamp;
    }
    mLastEventTimestamp = aEventTimestamp;
}

void CircularEventBuffer::RemoveEvent(uint16_t aNumEvents)
{
    mFirstEventNumber = static_cast<chip::EventNumber>(mFirstEventNumber + aNumEvents);
}

void CircularEventReader::Init(chip::app::reporting::CircularEventBufferWrapper * apBufWrapper)
{
    chip::TLV::TLVReader reader;
    CircularEventBuffer * prev;

    if (apBufWrapper->mpCurrent == nullptr)
        return;
    reader.Init(*apBufWrapper, apBufWrapper->mpCurrent->DataLength());
    TLVReader::Init(reader);
    mMaxLen = apBufWrapper->mpCurrent->DataLength();
    for (prev = apBufWrapper->mpCurrent->GetPreviousCircularEventBuffer(); prev != nullptr; prev = prev->mpPrev)
    {
        chip::app::reporting::CircularEventBufferWrapper bufWrapper;
        bufWrapper.mpCurrent = prev;
        reader.Init(bufWrapper, prev->DataLength());
        mMaxLen += reader.GetRemainingLength();
    }
}

CHIP_ERROR CircularEventBufferWrapper::GetNextBuffer(TLVReader & aReader, const uint8_t *& aBufStart, uint32_t & aBufLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mpCurrent->GetNextBuffer(aReader, aBufStart, aBufLen);
    SuccessOrExit(err);

    if ((aBufLen == 0) && (mpCurrent->GetPreviousCircularEventBuffer() != nullptr))
    {
        mpCurrent = mpCurrent->GetPreviousCircularEventBuffer();
        aBufStart = nullptr;
        err       = GetNextBuffer(aReader, aBufStart, aBufLen);
    }

exit:
    return err;
}

CopyAndAdjustDeltaTimeContext::CopyAndAdjustDeltaTimeContext(TLVWriter * aWriter, EventLoadOutContext * inContext) :
    mWriter(aWriter), mContext(inContext)
{}

EventEnvelopeContext::EventEnvelopeContext(void) : mNumFieldsToRead(2), mDeltaTime(0), mPriority(PriorityLevel::First) {}

} // namespace reporting
} // namespace app
} // namespace chip
