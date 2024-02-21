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

#include <access/AccessControl.h>
#include <access/RequestPath.h>
#include <access/SubjectDescriptor.h>
#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/RequiredPrivilege.h>
#include <assert.h>
#include <inttypes.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CodeUtils.h>
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

EventManagement & EventManagement::GetInstance()
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

void EventManagement::Init(Messaging::ExchangeManager * apExchangeManager, uint32_t aNumBuffers,
                           CircularEventBuffer * apCircularEventBuffer, const LogStorageResources * const apLogStorageResources,
                           MonotonicallyIncreasingCounter<EventNumber> * apEventNumberCounter,
                           System::Clock::Milliseconds64 aMonotonicStartupTime)
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

        current->mProcessEvictedElement = nullptr;
        current->mAppData               = nullptr;
    }

    mpEventNumberCounter = apEventNumberCounter;
    mLastEventNumber     = mpEventNumberCounter->GetValue();

    mpEventBuffer = apCircularEventBuffer;
    mState        = EventManagementStates::Idle;
    mBytesWritten = 0;

    mMonotonicStartupTime = aMonotonicStartupTime;
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

    ChipLogDetail(EventLogging, "Copy Event to next buffer with priority %u", static_cast<unsigned>(nextBuffer->GetPriority()));
exit:
    if (err != CHIP_NO_ERROR)
    {
        *nextBuffer = backup;
    }
    return err;
}

CHIP_ERROR EventManagement::EnsureSpaceInCircularBuffer(size_t aRequiredSpace, PriorityLevel aPriority)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    size_t requiredSpace              = aRequiredSpace;
    CircularEventBuffer * eventBuffer = mpEventBuffer;
    ReclaimEventCtx ctx;

    // Check that we have this much space in all our event buffers that might
    // hold the event. If we do not, that will prevent the event from being
    // properly evicted into higher-priority buffers. We want to discover
    // this early, so that testing surfaces the need to make those buffers
    // larger.
    for (auto * currentBuffer = mpEventBuffer; currentBuffer; currentBuffer = currentBuffer->GetNextCircularEventBuffer())
    {
        VerifyOrExit(requiredSpace <= currentBuffer->GetTotalDataLength(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
        if (currentBuffer->IsFinalDestinationForPriority(aPriority))
        {
            break;
        }
    }

    VerifyOrExit(eventBuffer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // check whether we actually need to do anything, exit if we don't
    VerifyOrExit(requiredSpace > eventBuffer->AvailableDataLength(), err = CHIP_NO_ERROR);

    while (true)
    {
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

    mpEventBuffer->mProcessEvictedElement = nullptr;
    mpEventBuffer->mAppData               = nullptr;

exit:
    return err;
}

CHIP_ERROR EventManagement::CalculateEventSize(EventLoggingDelegate * apDelegate, const EventOptions * apOptions,
                                               uint32_t & requiredSize)
{
    System::PacketBufferTLVWriter writer;
    EventLoadOutContext ctxt       = EventLoadOutContext(writer, apOptions->mPriority, GetLastEventNumber());
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(kMaxEventSizeReserve);
    if (buf.IsNull())
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    writer.Init(std::move(buf));

    ctxt.mCurrentEventNumber = mLastEventNumber;
    ctxt.mCurrentTime        = mLastEventTimestamp;
    CHIP_ERROR err           = ConstructEvent(&ctxt, apDelegate, apOptions);
    if (err == CHIP_NO_ERROR)
    {
        requiredSize = writer.GetLengthWritten();
    }
    return err;
}

CHIP_ERROR EventManagement::ConstructEvent(EventLoadOutContext * apContext, EventLoggingDelegate * apDelegate,
                                           const EventOptions * apOptions)
{
    VerifyOrReturnError(apContext->mCurrentEventNumber >= apContext->mStartingEventNumber, CHIP_NO_ERROR
                        /* no-op: don't write event, but advance current event Number */);

    VerifyOrReturnError(apOptions != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    EventReportIB::Builder eventReportBuilder;
    ReturnErrorOnFailure(eventReportBuilder.Init(&(apContext->mWriter)));
    EventDataIB::Builder & eventDataIBBuilder = eventReportBuilder.CreateEventData();
    ReturnErrorOnFailure(eventReportBuilder.GetError());
    EventPathIB::Builder & eventPathBuilder = eventDataIBBuilder.CreatePath();
    ReturnErrorOnFailure(eventDataIBBuilder.GetError());

    CHIP_ERROR err = eventPathBuilder.Endpoint(apOptions->mPath.mEndpointId)
                         .Cluster(apOptions->mPath.mClusterId)
                         .Event(apOptions->mPath.mEventId)
                         .EndOfEventPathIB();
    ReturnErrorOnFailure(err);
    eventDataIBBuilder.EventNumber(apContext->mCurrentEventNumber).Priority(chip::to_underlying(apContext->mPriority));
    ReturnErrorOnFailure(eventDataIBBuilder.GetError());

    if (apOptions->mTimestamp.IsSystem())
    {
        eventDataIBBuilder.SystemTimestamp(apOptions->mTimestamp.mValue);
    }
    else
    {
        eventDataIBBuilder.EpochTimestamp(apOptions->mTimestamp.mValue);
    }

    ReturnErrorOnFailure(eventDataIBBuilder.GetError());

    // Callback to write the EventData
    ReturnErrorOnFailure(apDelegate->WriteEvent(apContext->mWriter));

    // The fabricIndex profile tag is internal use only for fabric filtering when retrieving event from circular event buffer,
    // and would not go on the wire.
    // Revisit FabricRemovedCB function should the encoding of fabricIndex change in the future.
    if (apOptions->mFabricIndex != kUndefinedFabricIndex)
    {
        apContext->mWriter.Put(TLV::ProfileTag(kEventManagementProfile, kFabricIndexTag), apOptions->mFabricIndex);
    }
    ReturnErrorOnFailure(eventDataIBBuilder.EndOfEventDataIB());
    ReturnErrorOnFailure(eventReportBuilder.EndOfEventReportIB());
    ReturnErrorOnFailure(apContext->mWriter.Finalize());
    apContext->mFirst = false;
    return CHIP_NO_ERROR;
}

void EventManagement::CreateEventManagement(Messaging::ExchangeManager * apExchangeManager, uint32_t aNumBuffers,
                                            CircularEventBuffer * apCircularEventBuffer,
                                            const LogStorageResources * const apLogStorageResources,
                                            MonotonicallyIncreasingCounter<EventNumber> * apEventNumberCounter,
                                            System::Clock::Milliseconds64 aMonotonicStartupTime)
{

    sInstance.Init(apExchangeManager, aNumBuffers, apCircularEventBuffer, apLogStorageResources, apEventNumberCounter,
                   aMonotonicStartupTime);
}

/**
 * @brief Perform any actions we need to on shutdown.
 */
void EventManagement::DestroyEventManagement()
{
    sInstance.mState        = EventManagementStates::Shutdown;
    sInstance.mpEventBuffer = nullptr;
    sInstance.mpExchangeMgr = nullptr;
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
    CopyAndAdjustDeltaTimeContext * ctx = static_cast<CopyAndAdjustDeltaTimeContext *>(apContext);
    TLVReader reader(aReader);

    if (aReader.GetTag() == TLV::ProfileTag(kEventManagementProfile, kFabricIndexTag))
    {
        // Does not go on the wire.
        return CHIP_NO_ERROR;
    }
    if ((aReader.GetTag() == TLV::ContextTag(EventDataIB::Tag::kSystemTimestamp)) && !(ctx->mpContext->mFirst) &&
        (ctx->mpContext->mCurrentTime.mType == ctx->mpContext->mPreviousTime.mType))
    {
        return ctx->mpWriter->Put(TLV::ContextTag(EventDataIB::Tag::kDeltaSystemTimestamp),
                                  ctx->mpContext->mCurrentTime.mValue - ctx->mpContext->mPreviousTime.mValue);
    }
    if ((aReader.GetTag() == TLV::ContextTag(EventDataIB::Tag::kEpochTimestamp)) && !(ctx->mpContext->mFirst) &&
        (ctx->mpContext->mCurrentTime.mType == ctx->mpContext->mPreviousTime.mType))
    {
        return ctx->mpWriter->Put(TLV::ContextTag(EventDataIB::Tag::kDeltaEpochTimestamp),
                                  ctx->mpContext->mCurrentTime.mValue - ctx->mpContext->mPreviousTime.mValue);
    }

    return ctx->mpWriter->CopyElement(reader);
}

void EventManagement::VendEventNumber()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Now advance the counter.
    err = mpEventNumberCounter->Advance();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "%s Advance() failed with %" CHIP_ERROR_FORMAT, __FUNCTION__, err.Format());
    }

    // Assign event Number to the buffer's counter's value.
    mLastEventNumber = mpEventNumberCounter->GetValue();
}

CHIP_ERROR EventManagement::LogEvent(EventLoggingDelegate * apDelegate, const EventOptions & aEventOptions,
                                     EventNumber & aEventNumber)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mState != EventManagementStates::Shutdown, CHIP_ERROR_INCORRECT_STATE);
    return LogEventPrivate(apDelegate, aEventOptions, aEventNumber);
}

CHIP_ERROR EventManagement::LogEventPrivate(EventLoggingDelegate * apDelegate, const EventOptions & aEventOptions,
                                            EventNumber & aEventNumber)
{
    CircularTLVWriter writer;
    CHIP_ERROR err               = CHIP_NO_ERROR;
    uint32_t requestSize         = 0;
    aEventNumber                 = 0;
    CircularTLVWriter checkpoint = writer;
    EventLoadOutContext ctxt     = EventLoadOutContext(writer, aEventOptions.mPriority, mLastEventNumber);
    EventOptions opts;

    Timestamp timestamp;
#if CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
    System::Clock::Milliseconds64 utc_time;
    err = System::SystemClock().GetClock_RealTimeMS(utc_time);
    if (err == CHIP_NO_ERROR)
    {
        timestamp = Timestamp::Epoch(utc_time);
    }
    else
#endif // CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS
    {
        auto systemTimeMs = System::SystemClock().GetMonotonicMilliseconds64() - mMonotonicStartupTime;
        timestamp         = Timestamp::System(systemTimeMs);
    }

    opts = EventOptions(timestamp);
    // Start the event container (anonymous structure) in the circular buffer
    writer.Init(*mpEventBuffer);

    opts.mPriority = aEventOptions.mPriority;
    // Create all event specific data
    // Timestamp; encoded as a delta time

    opts.mPath        = aEventOptions.mPath;
    opts.mFabricIndex = aEventOptions.mFabricIndex;

    ctxt.mCurrentEventNumber = mLastEventNumber;
    ctxt.mCurrentTime.mValue = mLastEventTimestamp.mValue;

    err = CalculateEventSize(apDelegate, &opts, requestSize);
    SuccessOrExit(err);

    // Ensure we have space in the in-memory logging queues
    err = EnsureSpaceInCircularBuffer(requestSize, aEventOptions.mPriority);
    SuccessOrExit(err);

    err = ConstructEvent(&ctxt, apDelegate, &opts);
    SuccessOrExit(err);

    mBytesWritten += writer.GetLengthWritten();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "Log event with error %" CHIP_ERROR_FORMAT, err.Format());
        writer = checkpoint;
    }
    else if (opts.mPriority >= CHIP_CONFIG_EVENT_GLOBAL_PRIORITY)
    {
        aEventNumber = mLastEventNumber;
        VendEventNumber();
        mLastEventTimestamp = timestamp;
#if CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
        ChipLogDetail(EventLogging,
                      "LogEvent event number: 0x" ChipLogFormatX64 " priority: %u, endpoint id:  0x%x"
                      " cluster id: " ChipLogFormatMEI " event id: 0x%" PRIx32 " %s timestamp: 0x" ChipLogFormatX64,
                      ChipLogValueX64(aEventNumber), static_cast<unsigned>(opts.mPriority), opts.mPath.mEndpointId,
                      ChipLogValueMEI(opts.mPath.mClusterId), opts.mPath.mEventId,
                      opts.mTimestamp.mType == Timestamp::Type::kSystem ? "Sys" : "Epoch", ChipLogValueX64(opts.mTimestamp.mValue));
#endif // CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS

        err = InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleEventDelivery(opts.mPath, mBytesWritten);
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
    ReturnErrorOnFailure(aWriter.StartContainer(AnonymousTag(), kTLVType_Structure, containerType));

    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.EnterContainer(containerType1));
    ReturnErrorOnFailure(
        aWriter.StartContainer(TLV::ContextTag(EventReportIB::Tag::kEventData), kTLVType_Structure, containerType1));
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

CHIP_ERROR EventManagement::CheckEventContext(EventLoadOutContext * eventLoadOutContext,
                                              const EventManagement::EventEnvelopeContext & event)
{
    if (eventLoadOutContext->mCurrentEventNumber < eventLoadOutContext->mStartingEventNumber)
    {
        return CHIP_ERROR_UNEXPECTED_EVENT;
    }

    if (event.mFabricIndex.HasValue() &&
        (event.mFabricIndex.Value() == kUndefinedFabricIndex ||
         eventLoadOutContext->mSubjectDescriptor.fabricIndex != event.mFabricIndex.Value()))
    {
        return CHIP_ERROR_UNEXPECTED_EVENT;
    }

    ConcreteEventPath path(event.mEndpointId, event.mClusterId, event.mEventId);
    CHIP_ERROR ret = CHIP_ERROR_UNEXPECTED_EVENT;

    for (auto * interestedPath = eventLoadOutContext->mpInterestedEventPaths; interestedPath != nullptr;
         interestedPath        = interestedPath->mpNext)
    {
        if (interestedPath->mValue.IsEventPathSupersetOf(path))
        {
            ret = CHIP_NO_ERROR;
            break;
        }
    }

    ReturnErrorOnFailure(ret);

    Access::RequestPath requestPath{ .cluster = event.mClusterId, .endpoint = event.mEndpointId };
    Access::Privilege requestPrivilege = RequiredPrivilege::ForReadEvent(path);
    CHIP_ERROR accessControlError =
        Access::GetAccessControl().Check(eventLoadOutContext->mSubjectDescriptor, requestPath, requestPrivilege);
    if (accessControlError != CHIP_NO_ERROR)
    {
        ReturnErrorCodeIf(accessControlError != CHIP_ERROR_ACCESS_DENIED, accessControlError);
        ret = CHIP_ERROR_UNEXPECTED_EVENT;
    }

    return ret;
}

CHIP_ERROR EventManagement::EventIterator(const TLVReader & aReader, size_t aDepth, EventLoadOutContext * apEventLoadOutContext,
                                          EventEnvelopeContext * event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVReader innerReader;
    TLVType tlvType;
    TLVType tlvType1;

    innerReader.Init(aReader);
    VerifyOrDie(event != nullptr);
    ReturnErrorOnFailure(innerReader.EnterContainer(tlvType));
    ReturnErrorOnFailure(innerReader.Next());

    ReturnErrorOnFailure(innerReader.EnterContainer(tlvType1));
    err = TLV::Utilities::Iterate(innerReader, FetchEventParameters, event, false /*recurse*/);

    if (event->mFieldsToRead != kRequiredEventField)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    apEventLoadOutContext->mCurrentTime        = event->mCurrentTime;
    apEventLoadOutContext->mCurrentEventNumber = event->mEventNumber;

    err = CheckEventContext(apEventLoadOutContext, *event);
    if (err == CHIP_NO_ERROR)
    {
        err = CHIP_EVENT_ID_FOUND;
    }
    else if (err == CHIP_ERROR_UNEXPECTED_EVENT)
    {
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR EventManagement::CopyEventsSince(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    EventLoadOutContext * const loadOutContext = static_cast<EventLoadOutContext *>(apContext);
    EventEnvelopeContext event;
    CHIP_ERROR err = EventIterator(aReader, aDepth, loadOutContext, &event);
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

CHIP_ERROR EventManagement::FetchEventsSince(TLVWriter & aWriter, const SingleLinkedListNode<EventPathParams> * apEventPathList,
                                             EventNumber & aEventMin, size_t & aEventCount,
                                             const Access::SubjectDescriptor & aSubjectDescriptor)
{
    // TODO: Add particular set of event Paths in FetchEventsSince so that we can filter the interested paths
    CHIP_ERROR err     = CHIP_NO_ERROR;
    const bool recurse = false;
    TLVReader reader;
    CircularEventBufferWrapper bufWrapper;
    EventLoadOutContext context(aWriter, PriorityLevel::Invalid, aEventMin);

    context.mSubjectDescriptor     = aSubjectDescriptor;
    context.mpInterestedEventPaths = apEventPathList;
    err                            = GetEventReader(reader, PriorityLevel::Critical, &bufWrapper);
    SuccessOrExit(err);

    err = TLV::Utilities::Iterate(reader, CopyEventsSince, &context, recurse);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    if (err == CHIP_ERROR_BUFFER_TOO_SMALL || err == CHIP_ERROR_NO_MEMORY)
    {
        // We failed to fetch the current event because the buffer is too small, we will start from this one the next time.
        aEventMin = context.mCurrentEventNumber;
    }
    else
    {
        // For all other cases, continue from the next event.
        aEventMin = context.mCurrentEventNumber + 1;
    }
    aEventCount += context.mEventCount;
    return err;
}

CHIP_ERROR EventManagement::FabricRemovedCB(const TLV::TLVReader & aReader, size_t aDepth, void * apContext)
{
    // the function does not actually remove the event, instead, it sets the fabric index to an invalid value.
    FabricIndex * invalidFabricIndex = static_cast<FabricIndex *>(apContext);

    TLVReader event;
    TLVType tlvType;
    TLVType tlvType1;
    event.Init(aReader);
    VerifyOrReturnError(event.EnterContainer(tlvType) == CHIP_NO_ERROR, CHIP_NO_ERROR);
    VerifyOrReturnError(event.Next(TLV::ContextTag(EventReportIB::Tag::kEventData)) == CHIP_NO_ERROR, CHIP_NO_ERROR);
    VerifyOrReturnError(event.EnterContainer(tlvType1) == CHIP_NO_ERROR, CHIP_NO_ERROR);

    while (CHIP_NO_ERROR == event.Next())
    {
        if (event.GetTag() == TLV::ProfileTag(kEventManagementProfile, kFabricIndexTag))
        {
            uint8_t fabricIndex = 0;
            VerifyOrReturnError(event.Get(fabricIndex) == CHIP_NO_ERROR, CHIP_NO_ERROR);
            if (fabricIndex == *invalidFabricIndex)
            {
                TLVCircularBuffer * readBuffer = static_cast<TLVCircularBuffer *>(event.GetBackingStore());
                // fabricIndex is encoded as an integer; the dataPtr will point to a location immediately after its encoding
                // shift the dataPtr to point to the encoding of the fabric index, accounting for wraparound in backing storage
                // we cannot get the actual encoding size from current container beginning to the fabric index because of several
                // optional parameters, so we are assuming minimal encoding is used and the fabric index is 1 byte.
                uint8_t * dataPtr;
                if (event.GetReadPoint() != readBuffer->GetQueue())
                {
                    dataPtr = readBuffer->GetQueue() + (event.GetReadPoint() - readBuffer->GetQueue() - 1);
                }
                else
                {
                    dataPtr = readBuffer->GetQueue() + readBuffer->GetTotalDataLength() - 1;
                }

                *dataPtr = kUndefinedFabricIndex;
            }
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EventManagement::FabricRemoved(FabricIndex aFabricIndex)
{
    const bool recurse = false;
    TLVReader reader;
    CircularEventBufferWrapper bufWrapper;

    ReturnErrorOnFailure(GetEventReader(reader, PriorityLevel::Critical, &bufWrapper));
    CHIP_ERROR err = TLV::Utilities::Iterate(reader, FabricRemovedCB, &aFabricIndex, recurse);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
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

CHIP_ERROR EventManagement::FetchEventParameters(const TLVReader & aReader, size_t, void * apContext)
{
    EventEnvelopeContext * const envelope = static_cast<EventEnvelopeContext *>(apContext);
    TLVReader reader;
    reader.Init(aReader);

    if (reader.GetTag() == TLV::ContextTag(EventDataIB::Tag::kPath))
    {
        EventPathIB::Parser path;
        ReturnErrorOnFailure(path.Init(aReader));
        ReturnErrorOnFailure(path.GetEndpoint(&(envelope->mEndpointId)));
        ReturnErrorOnFailure(path.GetCluster(&(envelope->mClusterId)));
        ReturnErrorOnFailure(path.GetEvent(&(envelope->mEventId)));
        envelope->mFieldsToRead |= 1 << to_underlying(EventDataIB::Tag::kPath);
    }

    if (reader.GetTag() == TLV::ContextTag(EventDataIB::Tag::kPriority))
    {
        uint16_t extPriority; // Note: the type here matches the type case in EventManagement::LogEvent, priority section
        ReturnErrorOnFailure(reader.Get(extPriority));
        envelope->mPriority = static_cast<PriorityLevel>(extPriority);
        envelope->mFieldsToRead |= 1 << to_underlying(EventDataIB::Tag::kPriority);
    }

    if (reader.GetTag() == TLV::ContextTag(EventDataIB::Tag::kEventNumber))
    {
        ReturnErrorOnFailure(reader.Get(envelope->mEventNumber));
    }

    if (reader.GetTag() == TLV::ContextTag(EventDataIB::Tag::kSystemTimestamp))
    {
        uint64_t systemTime;
        ReturnErrorOnFailure(reader.Get(systemTime));
        envelope->mCurrentTime.mType  = Timestamp::Type::kSystem;
        envelope->mCurrentTime.mValue = systemTime;
    }

    if (reader.GetTag() == TLV::ContextTag(EventDataIB::Tag::kEpochTimestamp))
    {
        uint64_t epochTime;
        ReturnErrorOnFailure(reader.Get(epochTime));
        envelope->mCurrentTime.mType  = Timestamp::Type::kEpoch;
        envelope->mCurrentTime.mValue = epochTime;
    }

    if (reader.GetTag() == TLV::ProfileTag(kEventManagementProfile, kFabricIndexTag))
    {
        uint8_t fabricIndex = kUndefinedFabricIndex;
        ReturnErrorOnFailure(reader.Get(fabricIndex));
        envelope->mFabricIndex.SetValue(fabricIndex);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EventManagement::EvictEvent(TLVCircularBuffer & apBuffer, void * apAppData, TLVReader & aReader)
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
        ChipLogProgress(EventLogging,
                        "Dropped 1 event from buffer with priority %u and event number  0x" ChipLogFormatX64
                        " due to overflow: event priority_level: %u",
                        static_cast<unsigned>(eventBuffer->GetPriority()), ChipLogValueX64(context.mEventNumber),
                        static_cast<unsigned>(imp));
        ctx->mSpaceNeededForMovedEvent = 0;
        return CHIP_NO_ERROR;
    }

    // event is not getting dropped. Note how much space it requires, and return.
    ctx->mSpaceNeededForMovedEvent = aReader.GetLengthRead();
    return CHIP_END_OF_TLV;
}

void EventManagement::SetScheduledEventInfo(EventNumber & aEventNumber, uint32_t & aInitialWrittenEventBytes) const
{
    aEventNumber              = mLastEventNumber;
    aInitialWrittenEventBytes = mBytesWritten;
}

void CircularEventBuffer::Init(uint8_t * apBuffer, uint32_t aBufferLength, CircularEventBuffer * apPrev,
                               CircularEventBuffer * apNext, PriorityLevel aPriorityLevel)
{
    TLVCircularBuffer::Init(apBuffer, aBufferLength);
    mpPrev    = apPrev;
    mpNext    = apNext;
    mPriority = aPriorityLevel;
}

bool CircularEventBuffer::IsFinalDestinationForPriority(PriorityLevel aPriority) const
{
    return !((mpNext != nullptr) && (mpNext->mPriority <= aPriority));
}

/**
 * @brief
 * TLVCircularBuffer::OnInit can modify the state of the buffer, but we don't want that behavior here.
 * We want to make sure we don't change our state, and just report the currently-available space.
 */
CHIP_ERROR CircularEventBuffer::OnInit(TLV::TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen)
{
    GetCurrentWritableBuffer(bufStart, bufLen);
    return CHIP_NO_ERROR;
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
