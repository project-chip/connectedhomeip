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
#include <inttypes.h>
#include <lib/core/CHIPEventLoggingConfig.h>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::TLV;

namespace chip {
namespace app {
static EventManagement sInstance;

/**
 * @brief
 *   A TLVReader backed by CircularEventBuffer
 */
class CircularEventReader : public TLV::TLVReader
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
    size_t mSpaceNeededForMovedEvent    = 0;
};

/**
 * @brief
 *  Internal structure for traversing event list.
 */
struct CopyAndAdjustDeltaTimeContext
{
    CopyAndAdjustDeltaTimeContext(TLVWriter * aWriter, EventLoadOutContext * inContext) : mpWriter(aWriter), mpContext(inContext) {}

    TLV::TLVWriter * mpWriter       = nullptr;
    EventLoadOutContext * mpContext = nullptr;
};

/**
 * @brief
 *  Internal structure for traversing events.
 */
struct EventEnvelopeContext
{
    EventEnvelopeContext() {}

    int mFieldsToRead = 0;
    /* PriorityLevel and DeltaTime are there if that is not first event when putting events in report*/
#if CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS & CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
    Timestamp mDeltaTime = Timestamp::System(System::Clock::kZero);
#else
    Timestamp mDeltaTime = Timestamp::Epoch(System::Clock::kZero);
#endif
    PriorityLevel mPriority = PriorityLevel::First;
    ClusterId mClusterId    = 0;
    EndpointId mEndpointId  = 0;
    EventId mEventId        = 0;
};

void EventManagement::Init(Messaging::ExchangeManager * apExchangeManager, uint32_t aNumBuffers,
                           CircularEventBuffer * apCircularEventBuffer, const LogStorageResources * const apLogStorageResources)
{
    CircularEventBuffer * current = nullptr;
    CircularEventBuffer * prev    = nullptr;
    CircularEventBuffer * next    = nullptr;

    if (aNumBuffers == 0)
    {
        ChipLogError(EventLogging, "Invalid aNumBuffers");
        return;
    }

    if (mState != EventManagementStates::Shutdown)
    {
        ChipLogError(EventLogging, "Invalid EventManagement State");
        return;
    }
    mpExchangeMgr = apExchangeManager;

    for (uint32_t bufferIndex = 0; bufferIndex < aNumBuffers; bufferIndex++)
    {
        next = (bufferIndex < aNumBuffers - 1) ? &apCircularEventBuffer[bufferIndex + 1] : nullptr;

        current = &apCircularEventBuffer[bufferIndex];
        current->Init(apLogStorageResources[bufferIndex].mpBuffer, apLogStorageResources[bufferIndex].mBufferSize, prev, next,
                      apLogStorageResources[bufferIndex].mPriority);

        prev = current;

        current->mProcessEvictedElement = AlwaysFail;
        current->mAppData               = nullptr;
        current->InitCounter(apLogStorageResources[bufferIndex].InitializeCounter());
    }

    mpEventBuffer = apCircularEventBuffer;
    mState        = EventManagementStates::Idle;
    mBytesWritten = 0;

#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    CHIP_ERROR err = chip::System::Mutex::Init(mAccessLock);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "mutex init fails with error %s", ErrorStr(err));
    }
#endif // !CHIP_SYSTEM_CONFIG_NO_LOCKING
}

CHIP_ERROR EventManagement::CopyToNextBuffer(CircularEventBuffer * apEventBuffer)
{
    CircularTLVWriter writer;
    CircularTLVReader reader;
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    CircularEventBuffer * nextBuffer = apEventBuffer->GetNextCircularEventBuffer();
    if (nextBuffer == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    CircularEventBuffer backup = *nextBuffer;

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

    ChipLogProgress(EventLogging, "Copy Event to next buffer with priority %u", static_cast<unsigned>(nextBuffer->GetPriority()));
exit:
    if (err != CHIP_NO_ERROR)
    {
        *nextBuffer = backup;
    }
    return err;
}

CHIP_ERROR EventManagement::EnsureSpaceInCircularBuffer(size_t aRequiredSpace)
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
        VerifyOrExit(requiredSpace <= eventBuffer->GetTotalDataLength(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

        if (requiredSpace > eventBuffer->AvailableDataLength())
        {
            ctx.mpEventBuffer             = eventBuffer;
            ctx.mSpaceNeededForMovedEvent = 0;

            eventBuffer->mProcessEvictedElement = EvictEvent;
            eventBuffer->mAppData               = &ctx;
            err                                 = eventBuffer->EvictHead();

            // one of two things happened: either the element was evicted immediately if the head's priority is same as current
            // buffer(final one), or we figured out how much space we need to evict it into the next buffer, the check happens in
            // EvictEvent function

            if (err != CHIP_NO_ERROR)
            {
                VerifyOrExit(ctx.mSpaceNeededForMovedEvent != 0, /* no-op, return err */);
                VerifyOrExit(eventBuffer->GetNextCircularEventBuffer() != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
                if (ctx.mSpaceNeededForMovedEvent <= eventBuffer->GetNextCircularEventBuffer()->AvailableDataLength())
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
                // current required space in mRequiredSpaceForEvicted, we note the
                // space requirements for the event in the current
                // buffer and make that space in the next buffer.
                eventBuffer->SetRequiredSpaceforEvicted(requiredSpace);
                eventBuffer = eventBuffer->GetNextCircularEventBuffer();

                // Sanity check: return error  here on null event buffer.  If
                // eventBuffer->mpNext were null, then the `EvictBuffer`
                // would have succeeded -- the event was
                // already in the final buffer.
                VerifyOrExit(eventBuffer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
                requiredSpace = ctx.mSpaceNeededForMovedEvent;
            }
        }
        else
        {
            // this branch is only taken when we go back in the buffer chain since we have free/spare enough space in next buffer,
            // and need to retry to copy event from current buffer to next buffer, and free space for current buffer
            if (eventBuffer == mpEventBuffer)
                break;
            eventBuffer   = eventBuffer->GetPreviousCircularEventBuffer();
            requiredSpace = eventBuffer->GetRequiredSpaceforEvicted();
            err           = CHIP_NO_ERROR;
        }
    }

    // On exit, configure the top-level s.t. it will always fail to evict an element
    mpEventBuffer->mProcessEvictedElement = AlwaysFail;
    mpEventBuffer->mAppData               = nullptr;

exit:
    return err;
}

CHIP_ERROR EventManagement::CalculateEventSize(EventLoggingDelegate * apDelegate, const EventOptions * apOptions,
                                               uint32_t & requiredSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    EventLoadOutContext ctxt =
        EventLoadOutContext(writer, apOptions->mPriority, GetPriorityBuffer(apOptions->mPriority)->GetLastEventNumber());
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(kMaxEventSizeReserve);
    if (buf.IsNull())
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    writer.Init(std::move(buf));

    ctxt.mCurrentEventNumber = GetPriorityBuffer(apOptions->mPriority)->GetLastEventNumber();
    ctxt.mCurrentTime.mValue = GetPriorityBuffer(apOptions->mPriority)->GetLastEventTimestamp();

    TLVWriter checkpoint = ctxt.mWriter;
    err                  = ConstructEvent(&ctxt, apDelegate, apOptions);
    if (err != CHIP_NO_ERROR)
    {
        ctxt.mWriter = checkpoint;
    }
    else
    {
        // update these variables since ConstructEvent can be used to track the
        // state of a set of events over multiple calls.
        ctxt.mCurrentEventNumber++;
        ctxt.mCurrentTime = apOptions->mTimestamp;
        requiredSize      = writer.GetLengthWritten();
    }
    return err;
}

CHIP_ERROR EventManagement::ConstructEvent(EventLoadOutContext * apContext, EventLoggingDelegate * apDelegate,
                                           const EventOptions * apOptions)
{
    TLV::TLVType dataContainerType;
    uint64_t deltatime = 0;

    VerifyOrReturnError(apContext->mCurrentEventNumber >= apContext->mStartingEventNumber, CHIP_NO_ERROR
                        /* no-op: don't write event, but advance current event Number */);

    VerifyOrReturnError(apOptions != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    EventReportIB::Builder eventReportBuilder;
    eventReportBuilder.Init(&(apContext->mWriter));
    // TODO: Update IsUrgent, issue 11386
    // TODO: Update statusIB, issue 11388
    EventDataIB::Builder & eventDataIBBuilder = eventReportBuilder.CreateEventData();
    ReturnErrorOnFailure(eventReportBuilder.GetError());
    EventPathIB::Builder & eventPathBuilder = eventDataIBBuilder.CreatePath();
    ReturnErrorOnFailure(eventDataIBBuilder.GetError());

    eventPathBuilder.Endpoint(apOptions->mPath.mEndpointId)
        .Cluster(apOptions->mPath.mClusterId)
        .Event(apOptions->mPath.mEventId)
        .IsUrgent(false)
        .EndOfEventPathIB();
    ReturnErrorOnFailure(eventPathBuilder.GetError());
    eventDataIBBuilder.Priority(chip::to_underlying(apContext->mPriority));
    ReturnErrorOnFailure(eventDataIBBuilder.GetError());

    deltatime = apOptions->mTimestamp.mValue - apContext->mCurrentTime.mValue;
    if (apOptions->mTimestamp.IsSystem())
    {
        eventDataIBBuilder.DeltaSystemTimestamp(deltatime);
    }
    else
    {
        eventDataIBBuilder.DeltaEpochTimestamp(deltatime);
    }

    ReturnErrorOnFailure(eventDataIBBuilder.GetError());

    ReturnErrorOnFailure(apContext->mWriter.StartContainer(ContextTag(chip::to_underlying(EventDataIB::Tag::kData)),
                                                           TLV::kTLVType_Structure, dataContainerType));
    // Callback to write the EventData
    ReturnErrorOnFailure(apDelegate->WriteEvent(apContext->mWriter));
    ReturnErrorOnFailure(apContext->mWriter.EndContainer(dataContainerType));
    eventDataIBBuilder.EndOfEventDataIB();
    ReturnErrorOnFailure(eventDataIBBuilder.GetError());
    eventReportBuilder.EndOfEventReportIB();
    ReturnErrorOnFailure(eventReportBuilder.GetError());
    ReturnErrorOnFailure(apContext->mWriter.Finalize());
    apContext->mFirst = false;
    return CHIP_NO_ERROR;
}

void EventManagement::CreateEventManagement(Messaging::ExchangeManager * apExchangeManager, uint32_t aNumBuffers,
                                            CircularEventBuffer * apCircularEventBuffer,
                                            const LogStorageResources * const apLogStorageResources)
{

    sInstance.Init(apExchangeManager, aNumBuffers, apCircularEventBuffer, apLogStorageResources);
}

/**
 * @brief Perform any actions we need to on shutdown.
 */
void EventManagement::DestroyEventManagement()
{
#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    ScopedLock lock(sInstance);
#endif // !CHIP_SYSTEM_CONFIG_NO_LOCKING
    sInstance.mState        = EventManagementStates::Shutdown;
    sInstance.mpEventBuffer = nullptr;
    sInstance.mpExchangeMgr = nullptr;
}

EventNumber CircularEventBuffer::VendEventNumber()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Assign event Number to the buffer's counter's value.
    mLastEventNumber = static_cast<EventNumber>(mpEventNumberCounter->GetValue());

    // Now advance the counter.
    err = mpEventNumberCounter->Advance();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "%s Advance() for priority %u failed with %" CHIP_ERROR_FORMAT, __FUNCTION__,
                     static_cast<unsigned>(mPriority), err.Format());
    }

    return mLastEventNumber;
}

EventNumber EventManagement::GetLastEventNumber(PriorityLevel aPriority)
{
    return GetPriorityBuffer(aPriority)->GetLastEventNumber();
}

EventNumber EventManagement::GetFirstEventNumber(PriorityLevel aPriority)
{
    return GetPriorityBuffer(aPriority)->GetFirstEventNumber();
}

CircularEventBuffer * EventManagement::GetPriorityBuffer(PriorityLevel aPriority) const
{
    CircularEventBuffer * buf = mpEventBuffer;
    while (!buf->IsFinalDestinationForPriority(aPriority))
    {
        buf = buf->GetNextCircularEventBuffer();
        assert(buf != nullptr);
        // code guarantees that every PriorityLevel has a buffer destination.
    }
    return buf;
}

CHIP_ERROR EventManagement::CopyAndAdjustDeltaTime(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err;
    CopyAndAdjustDeltaTimeContext * ctx = static_cast<CopyAndAdjustDeltaTimeContext *>(apContext);
    TLVReader reader(aReader);

    if (aReader.GetTag() == TLV::ContextTag(to_underlying(EventDataIB::Tag::kDeltaSystemTimestamp)) ||
        aReader.GetTag() == TLV::ContextTag(to_underlying(EventDataIB::Tag::kDeltaEpochTimestamp)))
    {
        if (ctx->mpContext->mFirst) // First event gets a timestamp, subsequent ones get a delta T
        {
            if (ctx->mpContext->mCurrentTime.IsSystem())
            {
                err = ctx->mpWriter->Put(TLV::ContextTag(to_underlying(EventDataIB::Tag::kSystemTimestamp)),
                                         ctx->mpContext->mCurrentTime.mValue);
            }
            else
            {
                err = ctx->mpWriter->Put(TLV::ContextTag(to_underlying(EventDataIB::Tag::kEpochTimestamp)),
                                         ctx->mpContext->mCurrentTime.mValue);
            }
        }
        else
        {
            if (ctx->mpContext->mCurrentTime.IsSystem())
            {
                err = ctx->mpWriter->Put(TLV::ContextTag(to_underlying(EventDataIB::Tag::kDeltaSystemTimestamp)),
                                         ctx->mpContext->mCurrentTime.mValue - ctx->mpContext->mPreviousTime.mValue);
            }
            else
            {
                err = ctx->mpWriter->Put(TLV::ContextTag(to_underlying(EventDataIB::Tag::kDeltaEpochTimestamp)),
                                         ctx->mpContext->mCurrentTime.mValue - ctx->mpContext->mPreviousTime.mValue);
            }
        }
    }
    else
    {
        err = ctx->mpWriter->CopyElement(reader);
    }

    if (aReader.GetTag() == TLV::ContextTag(to_underlying(EventDataIB::Tag::kPath)))
    {
        err =
            ctx->mpWriter->Put(TLV::ContextTag(to_underlying(EventDataIB::Tag::kEventNumber)), ctx->mpContext->mCurrentEventNumber);
    }
    return err;
}

CHIP_ERROR EventManagement::LogEvent(EventLoggingDelegate * apDelegate, EventOptions & aEventOptions, EventNumber & aEventNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    {
#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
        ScopedLock lock(sInstance);
#endif // !CHIP_SYSTEM_CONFIG_NO_LOCKING

        VerifyOrExit(mState != EventManagementStates::Shutdown, err = CHIP_ERROR_INCORRECT_STATE);
        err = LogEventPrivate(apDelegate, aEventOptions, aEventNumber);
    }
exit:
    return err;
}

CHIP_ERROR EventManagement::LogEventPrivate(EventLoggingDelegate * apDelegate, EventOptions & aEventOptions,
                                            EventNumber & aEventNumber)
{
    CircularTLVWriter writer;
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    uint32_t requestSize           = 0;
    aEventNumber                   = 0;
    CircularEventBuffer checkpoint = *mpEventBuffer;
    CircularEventBuffer * buffer   = nullptr;
    EventLoadOutContext ctxt =
        EventLoadOutContext(writer, aEventOptions.mPriority, GetPriorityBuffer(aEventOptions.mPriority)->GetLastEventNumber());
    EventOptions opts;
#if CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS & CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
    Timestamp timestamp;
    System::Clock::Timestamp utc_time;

    err = System::SystemClock().GetClock_RealTimeMS(utc_time);
    SuccessOrExit(err);
    timestamp = Timestamp::Epoch(utc_time);
#else
    Timestamp timestamp(System::SystemClock().GetMonotonicTimestamp());
#endif

    opts = EventOptions(timestamp);
    // Start the event container (anonymous structure) in the circular buffer
    writer.Init(*mpEventBuffer);

    // check whether the entry is to be logged or discarded silently
    VerifyOrExit(aEventOptions.mPriority >= CHIP_CONFIG_EVENT_GLOBAL_PRIORITY, /* no-op */);

    opts.mPriority = aEventOptions.mPriority;
    // Create all event specific data
    // Timestamp; encoded as a delta time

    opts.mTimestamp = aEventOptions.mTimestamp;

    if (GetPriorityBuffer(aEventOptions.mPriority)->GetFirstEventTimestamp() == 0)
    {
        GetPriorityBuffer(aEventOptions.mPriority)->UpdateFirstLastEventTime(opts.mTimestamp);
    }

    opts.mUrgent = aEventOptions.mUrgent;
    opts.mPath   = aEventOptions.mPath;

    ctxt.mCurrentEventNumber = GetPriorityBuffer(opts.mPriority)->GetLastEventNumber();
    ctxt.mCurrentTime.mValue = GetPriorityBuffer(opts.mPriority)->GetLastEventTimestamp();

    err = CalculateEventSize(apDelegate, &opts, requestSize);
    SuccessOrExit(err);

    // Ensure we have space in the in-memory logging queues
    err = EnsureSpaceInCircularBuffer(requestSize);
    SuccessOrExit(err);

    err = ConstructEvent(&ctxt, apDelegate, &opts);
    SuccessOrExit(err);

    // Check the number of bytes written.  If the event is too large
    // to be evicted from subsequent buffers, drop it now.
    buffer = mpEventBuffer;
    while (true)
    {
        VerifyOrExit(buffer->GetTotalDataLength() >= writer.GetLengthWritten(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
        if (buffer->IsFinalDestinationForPriority(opts.mPriority))
        {
            break;
        }
        else
        {
            buffer = buffer->GetNextCircularEventBuffer();
            assert(buffer != nullptr);
            // code guarantees that every PriorityLevel has a buffer destination.
        }
    }

    mBytesWritten += writer.GetLengthWritten();

exit:
    if (err != CHIP_NO_ERROR)
    {
        *mpEventBuffer = checkpoint;
    }
    else if (opts.mPriority >= CHIP_CONFIG_EVENT_GLOBAL_PRIORITY)
    {
        CircularEventBuffer * currentBuffer = GetPriorityBuffer(opts.mPriority);
        aEventNumber                        = currentBuffer->VendEventNumber();
        currentBuffer->UpdateFirstLastEventTime(opts.mTimestamp);

#if CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
        ChipLogDetail(EventLogging,
                      "LogEvent event number: 0x" ChipLogFormatX64 " schema priority: %u cluster id: " ChipLogFormatMEI
                      " event id: 0x%" PRIx32 " %s timestamp: 0x" ChipLogFormatX64,
                      ChipLogValueX64(aEventNumber), static_cast<unsigned>(opts.mPriority), ChipLogValueMEI(opts.mPath.mClusterId),
                      opts.mPath.mEventId, opts.mTimestamp.mType == Timestamp::Type::kSystem ? "Sys" : "Epoch",
                      ChipLogValueX64(opts.mTimestamp.mValue));
#endif // CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS

        if (opts.mUrgent == EventOptions::Type::kUrgent)
        {
            ConcreteEventPath path(opts.mPath.mEndpointId, opts.mPath.mClusterId, opts.mPath.mEventId);
            err = InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleUrgentEventDelivery(path);
        }
    }

    return err;
}

CHIP_ERROR EventManagement::CopyEvent(const TLVReader & aReader, TLVWriter & aWriter, EventLoadOutContext * apContext)
{
    TLVReader reader;
    TLVType containerType;
    TLVType containerType1;
    CopyAndAdjustDeltaTimeContext context(&aWriter, apContext);
    CHIP_ERROR err = CHIP_NO_ERROR;

    reader.Init(aReader);
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(aWriter.StartContainer(AnonymousTag, kTLVType_Structure, containerType));

    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.EnterContainer(containerType1));
    ReturnErrorOnFailure(
        aWriter.StartContainer(TLV::ContextTag(to_underlying(EventReportIB::Tag::kEventData)), kTLVType_Structure, containerType1));
    err = TLV::Utilities::Iterate(reader, CopyAndAdjustDeltaTime, &context, false /*recurse*/);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(aWriter.EndContainer(containerType1));
    ReturnErrorOnFailure(aWriter.EndContainer(containerType));
    ReturnErrorOnFailure(aWriter.Finalize());
    return CHIP_NO_ERROR;
}

static bool IsInterestedEventPaths(EventLoadOutContext * eventLoadOutContext, const EventEnvelopeContext & event)
{
    if (eventLoadOutContext->mCurrentEventNumber < eventLoadOutContext->mStartingEventNumber)
    {
        return false;
    }
    ConcreteEventPath path(event.mEndpointId, event.mClusterId, event.mEventId);
    for (auto * interestedPath = eventLoadOutContext->mpInterestedEventPaths; interestedPath != nullptr;
         interestedPath        = interestedPath->mpNext)
    {
        if (interestedPath->IsEventPathSupersetOf(path))
        {
            return true;
        }
    }
    return false;
}

CHIP_ERROR EventManagement::EventIterator(const TLVReader & aReader, size_t aDepth, EventLoadOutContext * apEventLoadOutContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVReader innerReader;
    TLVType tlvType;
    TLVType tlvType1;
    EventEnvelopeContext event;

    innerReader.Init(aReader);
    ReturnErrorOnFailure(innerReader.EnterContainer(tlvType));
    ReturnErrorOnFailure(innerReader.Next());

    ReturnErrorOnFailure(innerReader.EnterContainer(tlvType1));
    err = TLV::Utilities::Iterate(innerReader, FetchEventParameters, &event, false /*recurse*/);

    if (event.mFieldsToRead != kRequiredEventField)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    if (event.mPriority == apEventLoadOutContext->mPriority)
    {
        apEventLoadOutContext->mCurrentTime.mValue += event.mDeltaTime.mValue;
        if (IsInterestedEventPaths(apEventLoadOutContext, event))
        {
            return CHIP_EVENT_ID_FOUND;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EventManagement::CopyEventsSince(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    EventLoadOutContext * const loadOutContext = static_cast<EventLoadOutContext *>(apContext);
    CHIP_ERROR err                             = EventIterator(aReader, aDepth, loadOutContext);
    loadOutContext->mCurrentEventNumber++;
    if (err == CHIP_EVENT_ID_FOUND)
    {
        // checkpoint the writer
        TLV::TLVWriter checkpoint = loadOutContext->mWriter;

        err = CopyEvent(aReader, loadOutContext->mWriter, loadOutContext);

        // CHIP_NO_ERROR and CHIP_END_OF_TLV signify a
        // successful copy.  In all other cases, roll back the
        // writer state back to the checkpoint, i.e., the state
        // before we began the copy operation.
        if ((err != CHIP_NO_ERROR) && (err != CHIP_END_OF_TLV))
        {
            loadOutContext->mWriter = checkpoint;
            return err;
        }

        loadOutContext->mPreviousTime.mValue = loadOutContext->mCurrentTime.mValue;
        loadOutContext->mFirst               = false;
        loadOutContext->mEventCount++;
    }

    return err;
}

CHIP_ERROR EventManagement::FetchEventsSince(TLVWriter & aWriter, ClusterInfo * apClusterInfolist, PriorityLevel aPriority,
                                             EventNumber & aEventNumber, size_t & aEventCount)
{
    // TODO: Add particular set of event Paths in FetchEventsSince so that we can filter the interested paths
    CHIP_ERROR err     = CHIP_NO_ERROR;
    const bool recurse = false;
    TLVReader reader;
    CircularEventBufferWrapper bufWrapper;
    EventLoadOutContext context(aWriter, aPriority, aEventNumber);

    CircularEventBuffer * buf = mpEventBuffer;
#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    ScopedLock lock(sInstance);
#endif // !CHIP_SYSTEM_CONFIG_NO_LOCKING

    while (!buf->IsFinalDestinationForPriority(aPriority))
    {
        buf = buf->GetNextCircularEventBuffer();
    }

    context.mpInterestedEventPaths = apClusterInfolist;
    context.mCurrentTime.mValue    = buf->GetFirstEventTimestamp();
    context.mCurrentEventNumber    = buf->GetFirstEventNumber();
    err                            = GetEventReader(reader, aPriority, &bufWrapper);
    SuccessOrExit(err);

    err = TLV::Utilities::Iterate(reader, CopyEventsSince, &context, recurse);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    aEventNumber = context.mCurrentEventNumber;
    aEventCount += context.mEventCount;
    return err;
}

CHIP_ERROR EventManagement::GetEventReader(TLVReader & aReader, PriorityLevel aPriority, CircularEventBufferWrapper * apBufWrapper)
{
    CircularEventBuffer * buffer = GetPriorityBuffer(aPriority);
    VerifyOrReturnError(buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    apBufWrapper->mpCurrent = buffer;

    CircularEventReader reader;
    reader.Init(apBufWrapper);
    aReader.Init(reader);

    return CHIP_NO_ERROR;
}

CHIP_ERROR EventManagement::FetchEventParameters(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    EventEnvelopeContext * const envelope = static_cast<EventEnvelopeContext *>(apContext);
    TLVReader reader;
    reader.Init(aReader);

    if (reader.GetTag() == TLV::ContextTag(to_underlying(EventDataIB::Tag::kPath)))
    {
        EventPathIB::Parser path;
        ReturnErrorOnFailure(path.Init(aReader));
        ReturnErrorOnFailure(path.GetEndpoint(&(envelope->mEndpointId)));
        ReturnErrorOnFailure(path.GetCluster(&(envelope->mClusterId)));
        ReturnErrorOnFailure(path.GetEvent(&(envelope->mEventId)));
        envelope->mFieldsToRead |= 1 << to_underlying(EventDataIB::Tag::kPath);
    }

    if (reader.GetTag() == TLV::ContextTag(to_underlying(EventDataIB::Tag::kPriority)))
    {
        uint16_t extPriority; // Note: the type here matches the type case in EventManagement::LogEvent, priority section
        ReturnErrorOnFailure(reader.Get(extPriority));
        envelope->mPriority = static_cast<PriorityLevel>(extPriority);
        envelope->mFieldsToRead |= 1 << to_underlying(EventDataIB::Tag::kPriority);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EventManagement::EvictEvent(CHIPCircularTLVBuffer & apBuffer, void * apAppData, TLVReader & aReader)
{
    // pull out the delta time, pull out the priority
    ReturnErrorOnFailure(aReader.Next());

    TLVType containerType;
    TLVType containerType1;
    ReturnErrorOnFailure(aReader.EnterContainer(containerType));
    ReturnErrorOnFailure(aReader.Next());

    ReturnErrorOnFailure(aReader.EnterContainer(containerType1));
    EventEnvelopeContext context;
    constexpr bool recurse = false;
    CHIP_ERROR err         = TLV::Utilities::Iterate(aReader, FetchEventParameters, &context, recurse);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    ReturnErrorOnFailure(aReader.ExitContainer(containerType1));
    ReturnErrorOnFailure(aReader.ExitContainer(containerType));
    const PriorityLevel imp = static_cast<PriorityLevel>(context.mPriority);

    ReclaimEventCtx * const ctx             = static_cast<ReclaimEventCtx *>(apAppData);
    CircularEventBuffer * const eventBuffer = ctx->mpEventBuffer;
    if (eventBuffer->IsFinalDestinationForPriority(imp))
    {
        // event is getting dropped.  Increase the event number and first timestamp.
        EventNumber numEventsToDrop = 1;
        eventBuffer->RemoveEvent(numEventsToDrop);
        eventBuffer->SetFirstEventTimestamp(eventBuffer->GetFirstEventTimestamp() + context.mDeltaTime.mValue);
        ChipLogProgress(
            EventLogging,
            "Dropped events from buffer with priority %u due to overflow: { event priority_level: %u, count: 0x" ChipLogFormatX64
            " };",
            static_cast<unsigned>(eventBuffer->GetPriority()), static_cast<unsigned>(imp), ChipLogValueX64(numEventsToDrop));
        ctx->mSpaceNeededForMovedEvent = 0;
        return CHIP_NO_ERROR;
    }

    // event is not getting dropped. Note how much space it requires, and return.
    ctx->mSpaceNeededForMovedEvent = aReader.GetLengthRead();
    return CHIP_END_OF_TLV;
}

void EventManagement::SetScheduledEventEndpoint(EventNumber * apEventEndpoints)
{
    CircularEventBuffer * eventBuffer = mpEventBuffer;

#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    ScopedLock lock(sInstance);
#endif // !CHIP_SYSTEM_CONFIG_NO_LOCKING

    while (eventBuffer != nullptr)
    {
        if (eventBuffer->GetPriority() >= PriorityLevel::First && (eventBuffer->GetPriority() <= PriorityLevel::Last))
        {
            apEventEndpoints[static_cast<uint8_t>(eventBuffer->GetPriority())] = eventBuffer->GetLastEventNumber();
        }
        eventBuffer = eventBuffer->GetNextCircularEventBuffer();
    }
}

void CircularEventBuffer::Init(uint8_t * apBuffer, uint32_t aBufferLength, CircularEventBuffer * apPrev,
                               CircularEventBuffer * apNext, PriorityLevel aPriorityLevel)
{
    CHIPCircularTLVBuffer::Init(apBuffer, aBufferLength);
    mpPrev            = apPrev;
    mpNext            = apNext;
    mPriority         = aPriorityLevel;
    mFirstEventNumber = 1;
    mLastEventNumber  = 0;
#if CHIP_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS & CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
    mFirstEventTimestamp = Timestamp::Epoch(System::Clock::kZero);
    mLastEventTimestamp  = Timestamp::Epoch(System::Clock::kZero);
#else
    mFirstEventTimestamp = Timestamp::System(System::Clock::kZero);
    mLastEventTimestamp  = Timestamp::System(System::Clock::kZero);
#endif
    mpEventNumberCounter = nullptr;
}

bool CircularEventBuffer::IsFinalDestinationForPriority(PriorityLevel aPriority) const
{
    return !((mpNext != nullptr) && (mpNext->mPriority <= aPriority));
}

void CircularEventBuffer::UpdateFirstLastEventTime(Timestamp aEventTimestamp)
{
    if (mFirstEventTimestamp.mValue == 0)
    {
        mFirstEventTimestamp = aEventTimestamp;
        mLastEventTimestamp  = aEventTimestamp;
    }
    mLastEventTimestamp = aEventTimestamp;
}

void CircularEventBuffer::RemoveEvent(EventNumber aNumEvents)
{
    mFirstEventNumber = mFirstEventNumber + aNumEvents;
}

void CircularEventReader::Init(CircularEventBufferWrapper * apBufWrapper)
{
    CircularEventBuffer * prev;

    if (apBufWrapper->mpCurrent == nullptr)
        return;

    TLVReader::Init(*apBufWrapper, apBufWrapper->mpCurrent->DataLength());
    mMaxLen = apBufWrapper->mpCurrent->DataLength();
    for (prev = apBufWrapper->mpCurrent->GetPreviousCircularEventBuffer(); prev != nullptr;
         prev = prev->GetPreviousCircularEventBuffer())
    {
        CircularEventBufferWrapper bufWrapper;
        bufWrapper.mpCurrent = prev;
        mMaxLen += prev->DataLength();
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
} // namespace app
} // namespace chip
