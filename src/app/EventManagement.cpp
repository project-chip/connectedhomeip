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

#include <app/EventManagement.h>
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
static EventManagement sInstance;

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
    void Init(CircularEventBufferWrapper * apBuf);

    virtual ~CircularEventReader() = default;
};

EventManagement & EventManagement::GetInstance(void)
{
    return sInstance;
}

struct ReclaimEventCtx
{
    CircularEventBuffer * mpEventBuffer = nullptr;
    size_t mSpaceNeededForEvent         = 0;
};

/**
 * @brief
 *  Internal structure for traversing event list.
 */
struct CopyAndAdjustDeltaTimeContext
{
    CopyAndAdjustDeltaTimeContext(TLVWriter * aWriter, EventLoadOutContext * inContext) : mpWriter(aWriter), mpContext(inContext) {}

    chip::TLV::TLVWriter * mpWriter = nullptr;
    EventLoadOutContext * mpContext = nullptr;
};

/**
 * @brief
 *  Internal structure for traversing events.
 */
struct EventEnvelopeContext
{
    EventEnvelopeContext() {}

    size_t mNumFieldsToRead =
        2 /* PriorityLevel and DeltaSystemTimestamp are there if that is not first event when putting events in report*/;
    uint32_t mDeltaTime     = 0;
    int64_t mDeltaUtc       = 0;
    PriorityLevel mPriority = PriorityLevel::First;
};

EventManagement::EventManagement(Messaging::ExchangeManager * apExchangeMgr, int aNumBuffers,
                                 CircularEventBuffer * apCircularEventBuffer,
                                 const LogStorageResources * const apLogStorageResources)
{
    Init(apExchangeMgr, aNumBuffers, apCircularEventBuffer, apLogStorageResources);
}

void EventManagement::Init(Messaging::ExchangeManager * apExchangeManager, int aNumBuffers,
                           CircularEventBuffer * apCircularEventBuffer, const LogStorageResources * const apLogStorageResources)
{
    VerifyOrDie(aNumBuffers > 0);

    mpExchangeMgr = apExchangeManager;

    for (int bufferIndex = 0; bufferIndex < aNumBuffers; bufferIndex++)
    {
        static CircularEventBuffer * current = nullptr;
        static CircularEventBuffer * prev    = nullptr;
        static CircularEventBuffer * next    = nullptr;

        next = (bufferIndex < aNumBuffers - 1) ? &apCircularEventBuffer[bufferIndex + 1] : nullptr;

        current = &apCircularEventBuffer[bufferIndex];
        current->Init(static_cast<uint8_t *>(apLogStorageResources[bufferIndex].mpBuffer),
                      (uint32_t)(apLogStorageResources[bufferIndex].mBufferSize), prev, next);

        prev = current;

        current->mProcessEvictedElement = AlwaysFail;
        current->mAppData               = nullptr;
        current->mPriority              = apLogStorageResources[bufferIndex].mPriority;
        if ((apLogStorageResources[bufferIndex].mpCounterStorage != nullptr) &&
            (apLogStorageResources[bufferIndex].mCounterKey != nullptr) && (apLogStorageResources[bufferIndex].mCounterEpoch != 0))
        {
            // We have been provided storage for a counter for this priority level.
            CHIP_ERROR err = apLogStorageResources[bufferIndex].mpCounterStorage->Init(
                *(apLogStorageResources[bufferIndex].mCounterKey), apLogStorageResources[bufferIndex].mCounterEpoch);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(EventLogging, "PersistedCounter[%d]->Init() failed with %d", bufferIndex, err);
            }
            current->mpEventNumberCounter = apLogStorageResources[bufferIndex].mpCounterStorage;
        }
        else
        {
            // No counter has been provided, so we'll use our
            // "built-in" non-persisted counter.
            current->mNonPersistedCounter.Init(1);
            current->mpEventNumberCounter = &(current->mNonPersistedCounter);
        }

        current->mFirstEventNumber = static_cast<chip::EventNumber>(current->mpEventNumberCounter->GetValue());
    }

    mpEventBuffer = &apCircularEventBuffer[0];
    mState        = EventManagementStates::Idle;
    mBytesWritten = 0;
}

CHIP_ERROR EventManagement::CopyToNextBuffer(CircularEventBuffer * apEventBuffer)
{
    CircularTLVWriter writer;
    CircularTLVReader reader;
    CircularEventBuffer checkpoint   = *(apEventBuffer->mpNext);
    CircularEventBuffer * nextBuffer = apEventBuffer->mpNext;
    CHIP_ERROR err                   = CHIP_NO_ERROR;

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

    ChipLogProgress(EventLogging, "Copy Event to next buffer with priority %d", nextBuffer->mPriority);
exit:
    if (err != CHIP_NO_ERROR)
    {
        *(apEventBuffer->mpNext) = checkpoint;
    }
    return err;
}

CHIP_ERROR EventManagement::EnsureSpace(size_t aRequiredSpace)
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

            // one of two things happened: either the element was evicted immediately if the head's priority is same as current
            // buffer(final one), or we figured out how much space we need to evict it into the next buffer, the check happens in
            // EvictEvent function

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
                    eventBuffer->mProcessEvictedElement = nullptr;
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
                VerifyOrDie(eventBuffer != nullptr);

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
    mpEventBuffer->mAppData               = nullptr;

exit:
    return err;
}

CHIP_ERROR EventManagement::BlitEvent(EventLoadOutContext * apContext, EventLoggingDelegate * apDelegate,
                                      const EventOptions * apOptions)
{

    CHIP_ERROR err       = CHIP_NO_ERROR;
    TLVWriter checkpoint = apContext->mWriter;
    chip::TLV::TLVType dataContainerType;
    chip::app::EventDataElement::Builder eventDataElementBuilder;
    chip::app::EventPath::Builder eventPathBuilder;

    VerifyOrExit(apContext->mCurrentEventNumber >= apContext->mStartingEventNumber,
                 /* no-op: don't write event, but advance current event Number */);

    VerifyOrExit(apOptions != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(apOptions->mTimestamp.mType != Timestamp::Type::kInvalid, err = CHIP_ERROR_INVALID_ARGUMENT);

    eventDataElementBuilder.Init(&(apContext->mWriter));
    eventPathBuilder = eventDataElementBuilder.CreateEventPathBuilder();
    SuccessOrExit(eventPathBuilder.GetError());

    // TODO: Revisit NodeId since the the encoding spec and the IM seem to disagree on how this stuff works
    eventPathBuilder.NodeId(apOptions->mpEventSchema->mNodeId)
        .EndpointId(apOptions->mpEventSchema->mEndpointId)
        .ClusterId(apOptions->mpEventSchema->mClusterId)
        .EventId(apOptions->mpEventSchema->mEventId)
        .EndOfEventPath();
    SuccessOrExit(eventPathBuilder.GetError());

    eventDataElementBuilder.PriorityLevel(static_cast<uint8_t>(apContext->mPriority));

    // if mFirst, record absolute time
    if (apContext->mFirst)
    {
        if (apOptions->mTimestamp.mType == Timestamp::Type::kSystem)
        {
            eventDataElementBuilder.Number(apContext->mCurrentEventNumber);
            eventDataElementBuilder.SystemTimestamp(apOptions->mTimestamp.mValue);
        }
    }
    // else record delta
    else
    {
        if (apOptions->mTimestamp.mType == Timestamp::Type::kSystem)
        {
            uint64_t deltatime = apOptions->mTimestamp.mValue - apContext->mCurrentSystemTime.mValue;
            eventDataElementBuilder.DeltaSystemTimestamp(deltatime);
        }
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
        if (apContext->mCurrentSystemTime.mType == Timestamp::Type::kSystem)
        {
            apContext->mCurrentSystemTime = apOptions->mTimestamp;
        }
    }
    return err;
}

void EventManagement::SkipEvent(EventLoadOutContext * apContext)
{
    apContext->mCurrentEventNumber++; // Advance the event id without writing anything
}

void EventManagement::CreateEventManagement(Messaging::ExchangeManager * apExchangeManager, int aNumBuffers,
                                            CircularEventBuffer * apCircularEventBuffer,
                                            const LogStorageResources * const apLogStorageResources)
{

    sInstance.Init(apExchangeManager, aNumBuffers, apCircularEventBuffer, apLogStorageResources);
    static_assert(std::is_trivially_destructible<EventManagement>::value, "EventManagement must be trivially destructible");
}

/**
 * @brief Perform any actions we need to on shutdown.
 */
void EventManagement::DestroyEventManagement()
{
    Platform::CriticalSectionEnter();
    sInstance.mState        = EventManagementStates::Shutdown;
    sInstance.mpEventBuffer = nullptr;
    sInstance.mpExchangeMgr = nullptr;
    Platform::CriticalSectionExit();
}

chip::EventNumber CircularEventBuffer::VendEventNumber()
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

chip::EventNumber EventManagement::GetLastEventNumber(PriorityLevel aPriority)
{
    return GetPriorityBuffer(aPriority)->mLastEventNumber;
}

chip::EventNumber EventManagement::GetFirstEventNumber(PriorityLevel aPriority)
{
    return GetPriorityBuffer(aPriority)->mFirstEventNumber;
}

CircularEventBuffer * EventManagement::GetPriorityBuffer(PriorityLevel aPriority) const
{
    CircularEventBuffer * buf = mpEventBuffer;
    while (!buf->IsFinalDestinationForPriority(aPriority))
    {
        buf = buf->mpNext;
    }
    return buf;
}

CHIP_ERROR EventManagement::CopyAndAdjustDeltaTime(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err;
    CopyAndAdjustDeltaTimeContext * ctx = static_cast<CopyAndAdjustDeltaTimeContext *>(apContext);
    TLVReader reader(aReader);

    // TODO: Add UTC timestamp support
    if (aReader.GetTag() == chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_DeltaSystemTimestamp))
    {
        if (ctx->mpContext->mFirst) // First event gets a timestamp, subsequent ones get a delta T
        {
            err = ctx->mpWriter->Put(chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_SystemTimestamp),
                                     ctx->mpContext->mCurrentSystemTime.mValue);
        }
        else
        {
            err = ctx->mpWriter->CopyElement(reader);
        }
    }
    else
    {
        err = ctx->mpWriter->CopyElement(reader);
    }

    // First event in the sequence gets a event number neatly packaged
    // right after the priority to keep tags ordered
    if (aReader.GetTag() == chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_PriorityLevel))
    {
        if (ctx->mpContext->mFirst)
        {
            err = ctx->mpWriter->Put(TLV::ContextTag(chip::app::EventDataElement::kCsTag_Number),
                                     ctx->mpContext->mCurrentEventNumber);
        }
    }

    return err;
}

CHIP_ERROR EventManagement::LogEvent(EventLoggingDelegate * apDelegate, const EventOptions * apOptions,
                                     chip::EventNumber & aEventNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Platform::CriticalSectionEnter();

    VerifyOrExit(mState != EventManagementStates::Shutdown, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(apOptions != nullptr && apOptions->mpEventSchema != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    err = LogEventPrivate(apDelegate, apOptions, aEventNumber);

exit:
    Platform::CriticalSectionExit();
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR EventManagement::LogEventPrivate(EventLoggingDelegate * apDelegate, const EventOptions * apOptions,
                                            chip::EventNumber & aEventNumber)
{
    CircularTLVWriter writer;
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    size_t requestSize             = kEventSizeRezerve;
    bool didWriteEvent             = false;
    aEventNumber                   = 0;
    CircularEventBuffer checkpoint = *mpEventBuffer;

    EventLoadOutContext ctxt = EventLoadOutContext(writer, apOptions->mpEventSchema->mPriority,
                                                   GetPriorityBuffer(apOptions->mpEventSchema->mPriority)->mLastEventNumber);
    Timestamp timestamp(Timestamp::Type::kSystem, System::Timer::GetCurrentEpoch());
    EventOptions opts = EventOptions(timestamp);

    // check whether the entry is to be logged or discarded silently
    VerifyOrExit(apOptions->mpEventSchema->mPriority >= CHIP_CONFIG_EVENT_GLOBAL_PRIORITY, /* no-op */);

    // Create all event specific data
    // Timestamp; encoded as a delta time
    if ((apOptions->mTimestamp.mType == Timestamp::Type::kSystem))
    {
        opts.mTimestamp = apOptions->mTimestamp;
    }

    if (GetPriorityBuffer(apOptions->mpEventSchema->mPriority)->mFirstEventSystemTimestamp.mValue == 0)
    {
        GetPriorityBuffer(apOptions->mpEventSchema->mPriority)->AddEvent(opts.mTimestamp);
    }

    opts.mUrgent       = apOptions->mUrgent;
    opts.mpEventSchema = apOptions->mpEventSchema;

    ctxt.mFirst              = false;
    ctxt.mCurrentEventNumber = GetPriorityBuffer(opts.mpEventSchema->mPriority)->mLastEventNumber;
    ctxt.mCurrentSystemTime  = GetPriorityBuffer(opts.mpEventSchema->mPriority)->mLastEventSystemTimestamp;

    // Begin writing
    while (!didWriteEvent)
    {
        // Ensure we have space in the in-memory logging queues
        err = EnsureSpace(requestSize);
        // If we fail to ensure the initial reserve size, then the
        // logging subsystem will never be able to make progress.  In
        // that case, it is best to assert
        if ((requestSize == kEventSizeRezerve) && (err != CHIP_NO_ERROR))
            chipDie();
        SuccessOrExit(err);

        // save a checkpoint for the underlying buffer.  Note that with
        // the current event buffering scheme, only the mpEventBuffer will
        // be affected by the writes to the `writer` below, and thus
        // that's the only thing we need to checkpoint.
        checkpoint = *mpEventBuffer;

        // Start the event container (anonymous structure) in the circular buffer
        writer.Init(*mpEventBuffer);

        err = BlitEvent(&ctxt, apDelegate, &opts);

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
        // Check the number of bytes written.  If the event is too large
        // to be evicted from subsequent buffers, drop it now.
        CircularEventBuffer * buffer = mpEventBuffer;
        do
        {
            VerifyOrExit(buffer->GetQueueSize() >= writer.GetLengthWritten(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
            if (buffer->IsFinalDestinationForPriority(opts.mpEventSchema->mPriority))
            {
                break;
            }
            else
            {
                buffer = buffer->mpNext;
            }
        } while (true);
    }

    mBytesWritten += writer.GetLengthWritten();

exit:
    ChipLogFunctError(err);
    if (err != CHIP_NO_ERROR)
    {
        *mpEventBuffer = checkpoint;
    }
    else if (opts.mpEventSchema->mPriority >= CHIP_CONFIG_EVENT_GLOBAL_PRIORITY)
    {
        CircularEventBuffer * currentBuffer = GetPriorityBuffer(opts.mpEventSchema->mPriority);
        aEventNumber                        = currentBuffer->VendEventNumber();
        currentBuffer->AddEvent(opts.mTimestamp);

#if CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
        ChipLogDetail(EventLogging,
                      "LogEvent event number: %u schema priority: %u cluster id: 0x%x event id: 0x%x sys timestamp: 0x%" PRIx64,
                      aEventNumber, opts.mpEventSchema->mPriority, opts.mpEventSchema->mClusterId, opts.mpEventSchema->mEventId,
                      opts.mTimestamp.mValue);
#endif // CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS

        ScheduleFlushIfNeeded(apOptions == nullptr ? false : opts.mUrgent);
    }

    return err;
}

CHIP_ERROR EventManagement::CopyEvent(const TLVReader & aReader, TLVWriter & aWriter, EventLoadOutContext * apContext)
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

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR EventManagement::EventIterator(const TLVReader & aReader, size_t aDepth, void * apContext)
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
        loadOutContext->mCurrentSystemTime.mValue += event.mDeltaTime;
        VerifyOrExit(loadOutContext->mCurrentEventNumber < loadOutContext->mStartingEventNumber, err = CHIP_EVENT_ID_FOUND);
        loadOutContext->mCurrentEventNumber++;
    }

exit:
    return err;
}

CHIP_ERROR EventManagement::CopyEventsSince(const TLVReader & aReader, size_t aDepth, void * apContext)
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

        loadOutContext->mCurrentSystemTime.mValue = 0;
        loadOutContext->mFirst                    = false;
        loadOutContext->mCurrentEventNumber++;
    }

exit:
    return err;
}

CHIP_ERROR EventManagement::FetchEventsSince(TLVWriter & aWriter, PriorityLevel aPriority, chip::EventNumber & aEventNumber)
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

    context.mCurrentSystemTime  = buf->mFirstEventSystemTimestamp;
    context.mCurrentEventNumber = buf->mFirstEventNumber;
    err                         = GetEventReader(reader, aPriority, &bufWrapper);
    SuccessOrExit(err);

    err = chip::TLV::Utilities::Iterate(reader, CopyEventsSince, &context, recurse);

exit:
    aEventNumber = context.mCurrentEventNumber;
    Platform::CriticalSectionExit();

    return err;
}

CHIP_ERROR EventManagement::GetEventReader(TLVReader & aReader, PriorityLevel aPriority,
                                           chip::app::CircularEventBufferWrapper * apBufWrapper)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CircularEventReader reader;
    CircularEventBuffer * buffer = GetPriorityBuffer(aPriority);
    VerifyOrExit(buffer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    apBufWrapper->mpCurrent = buffer;
    reader.Init(apBufWrapper);
    aReader.Init(reader);
exit:
    return err;
}

CHIP_ERROR EventManagement::FetchEventParameters(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err                  = CHIP_NO_ERROR;
    EventEnvelopeContext * envelope = static_cast<EventEnvelopeContext *>(apContext);
    TLVReader reader;
    uint16_t extPriority; // Note: the type here matches the type case in EventManagement::LogEvent, priority section
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

CHIP_ERROR EventManagement::EvictEvent(CHIPCircularTLVBuffer & apBuffer, void * apAppData, TLVReader & inReader)
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
        eventBuffer->mFirstEventSystemTimestamp.mValue += context.mDeltaTime;
        ChipLogProgress(EventLogging,
                        "Dropped events from buffer with priority %d due to overflow: { event priority_level: %d, count: %d };",
                        eventBuffer->mPriority, imp, numEventsToDrop);
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

CHIP_ERROR EventManagement::ScheduleFlushIfNeeded(bool inRequestFlush)
{
    // TODO: Implement ScheduleFlushIfNeeded
    return CHIP_NO_ERROR;
}

CircularEventBuffer::CircularEventBuffer() :
    CHIPCircularTLVBuffer(nullptr, 0), mpPrev(nullptr), mpNext(nullptr), mPriority(PriorityLevel::First), mFirstEventNumber(1),
    mLastEventNumber(0), mFirstEventSystemTimestamp(Timestamp::Type::kSystem, 0),
    mLastEventSystemTimestamp(Timestamp::Type::kSystem, 0), mpEventNumberCounter(nullptr)
{}

void CircularEventBuffer::Init(uint8_t * apBuffer, uint32_t aBufferLength, CircularEventBuffer * apPrev,
                               CircularEventBuffer * apNext)
{
    CHIPCircularTLVBuffer::Init(apBuffer, aBufferLength);
    mpPrev            = apPrev;
    mpNext            = apNext;
    mPriority         = PriorityLevel::First;
    mFirstEventNumber = 1;
    mLastEventNumber  = 0;
    mFirstEventSystemTimestamp.Init(Timestamp::Type::kSystem, 0);
    mLastEventSystemTimestamp.Init(Timestamp::Type::kSystem, 0);
    mpEventNumberCounter = nullptr;
}

bool CircularEventBuffer::IsFinalDestinationForPriority(PriorityLevel aPriority) const
{
    return !((mpNext != nullptr) && (mpNext->mPriority <= aPriority));
}

void CircularEventBuffer::AddEvent(Timestamp aEventTimestamp)
{
    if (mFirstEventSystemTimestamp.mValue == 0)
    {
        mFirstEventSystemTimestamp = aEventTimestamp;
        mLastEventSystemTimestamp  = aEventTimestamp;
    }
    mLastEventSystemTimestamp = aEventTimestamp;
}

void CircularEventBuffer::RemoveEvent(uint16_t aNumEvents)
{
    mFirstEventNumber = static_cast<chip::EventNumber>(mFirstEventNumber + aNumEvents);
}

void CircularEventReader::Init(CircularEventBufferWrapper * apBufWrapper)
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
        CircularEventBufferWrapper bufWrapper;
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
    ChipLogFunctError(err);
    return err;
}
} // namespace app
} // namespace chip
