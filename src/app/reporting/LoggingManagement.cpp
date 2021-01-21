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
namespace app {
namespace reporting {

// Events are embedded in an anonymous structure: 1 for the control byte, 1 for end-of-container
#define EVENT_CONTAINER_OVERHEAD_TLV_SIZE 2
// Event importance element consumes 3 bytes: control byte, 1-byte tag, and 1 byte value
#define IMPORTANCE_TLV_SIZE 3
// Overhead of embedding something in a (short) byte string: 1 byte control, 1 byte tag, 1 byte length
#define EXTERNAL_EVENT_BYTE_STRING_TLV_SIZE 3

class CircularEventBuffer;

// Static instance: embedded platforms not always implement a proper
// C++ runtime; instead, the instance is initialized via placement new
// in CreateLoggingManagement.

static LoggingManagement sInstance;

LoggingManagement & LoggingManagement::GetInstance(void)
{
    return sInstance;
}

struct ReclaimEventCtx
{
    CircularEventBuffer * mpEventBuffer;
    size_t mSpaceNeededForEvent;
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
                // current required space in mAppData, we note the
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

/**
 * @brief Helper function for writing event header and data according to event
 *   logging protocol.
 *
 * @param[inout] apContext   EventLoadOutContext, initialized with stateful
 *                          information for the buffer. State is updated
 *                          and preserved by BlitEvent using this context.
 *
 * @param[in] aSchema      Schema defining importance, profile ID, and
 *                          structure type of this event.
 *
 * @param[in] aEventWriter The callback to invoke to serialize the event data.
 *
 * @param[in] apAppData     Application context for the callback.
 *
 * @param[in] apOptions     EventOptions describing timestamp and other tags
 *                          relevant to this event.
 *
 */
CHIP_ERROR LoggingManagement::BlitEvent(EventLoadOutContext * apContext, const EventSchema & aSchema, EventWriterFunct aEventWriter,
                                        void * apAppData, const EventOptions * apOptions)
{

    CHIP_ERROR err       = CHIP_NO_ERROR;
    TLVWriter checkpoint = apContext->mWriter;
    chip::TLV::TLVType dataContainerType;
    chip::app::EventDataElement::Builder eventDataElementBuilder;
    chip::app::EventPath::Builder eventPathBuilder;

    VerifyOrExit(apContext->mCurrentEventID >= apContext->mStartingEventID,
                 /* no-op: don't write event, but advance current event ID */);

    VerifyOrExit(apOptions != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(apOptions->timestampType != kTimestampType_Invalid, err = CHIP_ERROR_INVALID_ARGUMENT);

    eventDataElementBuilder.Init(&(apContext->mWriter));
    eventPathBuilder = eventDataElementBuilder.CreateEventPathBuilder();
    SuccessOrExit(eventPathBuilder.GetError());

    eventPathBuilder.NodeId(apOptions->eventSource->mNodeId)
        .EndpointId(apOptions->eventSource->mEndpointId)
        .ClusterId(aSchema.mClusterId)
        .EventId(((chip::EventId)(aSchema.mStructureType)))
        .EndOfEventPath();
    SuccessOrExit(eventPathBuilder.GetError());

    eventDataElementBuilder.PriorityLevel(apContext->mPriority).Number(apContext->mCurrentEventID);
    SuccessOrExit(eventDataElementBuilder.GetError());

    // if mFirst, record absolute time
    if (apContext->mFirst)
    {
        eventDataElementBuilder.SystemTimestamp(apOptions->timestamp.systemTimestamp);
    }
    // else record delta
    else
    {
        uint32_t deltatime = apOptions->timestamp.systemTimestamp - apContext->mCurrentTime;
        eventDataElementBuilder.DeltaSystemTimeStamp(deltatime);
    }

    SuccessOrExit(eventDataElementBuilder.GetError());

    err = apContext->mWriter.StartContainer(ContextTag(chip::app::EventDataElement::kCsTag_Data), chip::TLV::kTLVType_Structure,
                                            dataContainerType);
    SuccessOrExit(err);
    // Callback to write the EventData
    err = aEventWriter(apContext->mWriter, apAppData);
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
        apContext->mCurrentEventID++;
        {
            apContext->mCurrentTime = apOptions->timestamp.systemTimestamp;
        }
    }
    return err;
}

/**
 * @brief Helper function to skip writing an event corresponding to an allocated
 *   event id.
 *
 * @param[inout] apContext   EventLoadOutContext, initialized with stateful
 *                          information for the buffer. State is updated
 *                          and preserved by BlitEvent using this context.
 *
 */
void LoggingManagement::SkipEvent(EventLoadOutContext * apContext)
{
    apContext->mCurrentEventID++; // Advance the event id without writing anything
}

/**
 * @brief Create LoggingManagement object and initialize the logging management
 *   subsystem with provided resources.
 *
 * Initialize the LoggingManagement with an array of LogStorageResources.  The
 * array must provide a resource for each valid importance level, the elements
 * of the array must be in increasing numerical value of importance (and in
 * decreasing importance); the first element in the array corresponds to the
 * resources allocated for the most critical events, and the last element
 * corresponds to the least important events.
 *
 * @param[in] inMgr         ExchangeManager to be used with this logging subsystem
 *
 * @param[in] inNumBuffers  Number of elements in inLogStorageResources array
 *
 * @param[in] inLogStorageResources  An array of LogStorageResources for each importance level.
 *
 * @note This function must be called prior to the logging being used.
 */
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
    sInstance.mState        = kLoggingManagementState_Shutdown;
    sInstance.mpEventBuffer = NULL;
}

/**
 * @brief Set the ExchangeManager to be used with this logging subsystem.  On some
 *   platforms, this may need to happen separately from CreateLoggingManagement() above.
 *
 * @param[in] inMgr         ExchangeManager to be used with this logging subsystem
 */
CHIP_ERROR LoggingManagement::SetExchangeManager(Messaging::ExchangeManager * apMgr)
{
    mpExchangeMgr = apMgr;
    return CHIP_NO_ERROR;
}

/**
 * @brief
 *   LoggingManagement constructor
 *
 * Initialize the LoggingManagement with an array of LogStorageResources.  The
 * array must provide a resource for each valid priority level, the elements
 * of the array must be in increasing numerical value of priority (and in
 * increasing importance); the first element in the array corresponds to the
 * resources allocated for least important events, and the last element
 * corresponds to the most critical events.
 *
 * @param[in] apMgr         ExchangeManager to be used with this logging subsystem
 *
 * @param[in] aNumBuffers  Number of elements in inLogStorageResources array
 *
 * @param[in] apLogStorageResources  An array of LogStorageResources for each importance level.
 *
 */

LoggingManagement::LoggingManagement(Messaging::ExchangeManager * apMgr, size_t aNumBuffers,
                                     const LogStorageResources * const apLogStorageResources)
{
    CircularEventBuffer * current = NULL;
    CircularEventBuffer * prev    = NULL;
    CircularEventBuffer * next    = NULL;
    size_t i, j;

    VerifyOrDie(aNumBuffers > 0);

    mThrottled    = 0;
    mpExchangeMgr = apMgr;

    for (j = 0; j < aNumBuffers; j++)
    {
        i = aNumBuffers - 1 - j;

        next = (i > 0) ? static_cast<CircularEventBuffer *>(apLogStorageResources[i - 1].mpBuffer) : NULL;

        VerifyOrDie(apLogStorageResources[i].mBufferSize > sizeof(CircularEventBuffer));

        new (apLogStorageResources[i].mpBuffer)
            CircularEventBuffer(static_cast<uint8_t *>(apLogStorageResources[i].mpBuffer) + sizeof(CircularEventBuffer),
                                (uint32_t)(apLogStorageResources[i].mBufferSize - sizeof(CircularEventBuffer)), prev, next);

        current = prev                  = static_cast<CircularEventBuffer *>(apLogStorageResources[i].mpBuffer);
        current->mProcessEvictedElement = AlwaysFail;
        current->mAppData               = NULL;
        current->mPriority              = apLogStorageResources[i].mPriority;
        if ((apLogStorageResources[i].mCounterStorage != NULL) && (apLogStorageResources[i].mCounterKey != NULL) &&
            (apLogStorageResources[i].mCounterEpoch != 0))
        {

            // We have been provided storage for a counter for this importance level.
            new (apLogStorageResources[i].mCounterStorage) PersistedCounter();
            CHIP_ERROR err = apLogStorageResources[i].mCounterStorage->Init(*(apLogStorageResources[i].mCounterKey),
                                                                            apLogStorageResources[i].mCounterEpoch);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(EventLogging, "%s PersistedCounter[%d]->Init() failed with %d", __FUNCTION__, j, err);
            }
            current->mpEventIdCounter = apLogStorageResources[i].mCounterStorage;
        }
        else
        {
            // No counter has been provided, so we'll use our
            // "built-in" non-persisted counter.
            current->mNonPersistedCounter.Init(1);
            current->mpEventIdCounter = &(current->mNonPersistedCounter);
        }

        current->mFirstEventID = (chip::EventId) current->mpEventIdCounter->GetValue();
    }
    mpEventBuffer = static_cast<CircularEventBuffer *>(apLogStorageResources[kPriorityLevel_Last - kPriorityLevel_First].mpBuffer);

    mState             = kLoggingManagementState_Idle;
    mBytesWritten      = 0;
    mUploadRequested   = false;
    mMaxPriorityBuffer = kPriorityLevel_Last;
}

/**
 * @brief
 *   LoggingManagement default constructor. Provided primarily to make the compiler happy.
 *

 * @return LoggingManagement
 */
LoggingManagement::LoggingManagement(void) :
    mpEventBuffer(NULL), mpExchangeMgr(NULL), mState(kLoggingManagementState_Idle), mBytesWritten(0), mThrottled(0),
    mMaxPriorityBuffer(kPriorityLevel_Invalid), mUploadRequested(false)
{}

/**
 * @brief
 *   A function to get the current importance of a profile.
 *
 * The function returns the current priority of a ckuster as
 * currently configured in the #LoggingConfiguration cluster.  When
 * per-cluster priority is supported, it is used; otherwise only
 * global importance is supported.  When the log is throttled, we only
 * record the Critical events
 *
 * @param aClusterId Cluster against which the event is being logged
 *
 * @return Priority of the current Cluster based on the config
 */
PriorityLevel LoggingManagement::GetCurrentPriority(chip::ClusterId aClusterId)
{
    const LoggingConfiguration & config = LoggingConfiguration::GetInstance();
    PriorityLevel retval;

    if (mThrottled != 0)
    {
        retval = Info;
    }
    else if (config.SupportsPerClusterPriority())
    {
        retval = config.GetClusterPriority(aClusterId);
    }
    else
    {
        retval = config.mGlobalPriority;
    }
    return retval;
}

/**
 * @brief
 *   Get the max available importance of the system.
 *
 *  This function returns the max importance stored by logging management,
 *  as defined by both the global importance and number of buffers available
 *
 * @return PriorityLevel Max importance type currently stored.
 */
PriorityLevel LoggingManagement::GetMaxPriority(void)
{
    const LoggingConfiguration & config = LoggingConfiguration::GetInstance();
    return (config.mGlobalPriority < mMaxPriorityBuffer ? config.mGlobalPriority : mMaxPriorityBuffer);
}

/**
 * @brief
 *   Allocate a new event ID based on the event importance, and advance the counter
 *   if we have one.
 *
 * @return chip::EventId Event ID for this importance.
 */
chip::EventId CircularEventBuffer::VendEventID(void)
{
    chip::EventId retval = 0;
    CHIP_ERROR err       = CHIP_NO_ERROR;

    // Assign event ID to the buffer's counter's value.
    retval       = (chip::EventId)(mpEventIdCounter->GetValue());
    mLastEventID = static_cast<chip::EventId>(retval);

    // Now advance the counter.
    err = mpEventIdCounter->Advance();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "%s Advance() for importance %d failed with %d", __FUNCTION__, mPriority, err);
    }

    return retval;
}

/**
 * @brief
 *   Fetch the most recently vended ID for a particular importance level
 *
 * @param aPriority Priority level
 *
 * @return chip::EventId most recently vended event ID for that event importance
 */
chip::EventId LoggingManagement::GetLastEventID(PriorityLevel aPriority)
{
    return GetPriorityBuffer(aPriority)->mLastEventID;
}

/**
 * @brief
 *   Fetch the first event ID currently stored for a particular importance level
 *
 * @param aPriority Priority level
 *
 * @return chip::EventId First currently stored event ID for that event importance
 */
chip::EventId LoggingManagement::GetFirstEventID(PriorityLevel aPriority)
{
    return GetPriorityBuffer(aPriority)->mFirstEventID;
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

// Internal API used in copying an event out of the event buffers

CHIP_ERROR LoggingManagement::CopyAndAdjustDeltaTime(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err;
    CopyAndAdjustDeltaTimeContext * ctx = static_cast<CopyAndAdjustDeltaTimeContext *>(apContext);
    TLVReader reader(aReader);

    if (aReader.GetTag() == chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_DeltaSystemTimeStamp))
    {
        if (ctx->mContext->mFirst) // First event gets a timestamp, subsequent ones get a delta T
        {
            err = ctx->mWriter->Put(chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_DeltaSystemTimeStamp),
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

    return err;
}

/**
 * @brief
 *   Log an event via a callback, with options.
 *
 * The function logs an event represented as an ::EventWriterFunct and
 * an app-specific `appData` context.  The function writes the event
 * metadata and calls the `aEventWriter` with an chip::TLV::TLVWriter
 * reference and `apAppData` context so that the user code can emit
 * the event data directly into the event log.  This form of event
 * logging minimizes memory consumption, as event data is serialized
 * directly into the target buffer.  The event data MUST contain
 * context tags to be interpreted within the schema identified by
 * `inProfileID` and `inEventType`. The tag of the first element will be
 * ignored; the event logging system will replace it with the
 * eventData tag.
 *
 * The event is logged if the schema importance exceeds the logging
 * threshold specified in the LoggingConfiguration.  If the event's
 * importance does not meet the current threshold, it is dropped and
 * the function returns a `0` as the resulting event ID.
 *
 * This variant of the invocation permits the caller to set any
 * combination of `EventOptions`:
 * - timestamp, when 0 defaults to the current time at the point of
 *   the call,
 * - "root" section of the event source (event source and trait ID);
 *   if NULL, it defaults to the current device. the event is marked as
 *   relating to the device that is making the call,
 * - a related event ID for grouping event IDs; when the related event
 *   ID is 0, the event is marked as not relating to any other events,
 * - urgency; by default non-urgent.
 *
 * @param[in] aSchema     Schema defining importance, profile ID, and
 *                         structure type of this event.
 *
 * @param[in] aEventWriter The callback to invoke to actually
 *                         serialize the event data
 *
 * @param[in] apAppData    Application context for the callback.
 *
 * @param[in] apOptions    The options for the event metadata. May be NULL.
 *
 * @return chip::EventId      The event ID if the event was written to the
 *                         log, 0 otherwise.
 */
chip::EventId LoggingManagement::LogEvent(const EventSchema & aSchema, EventWriterFunct aEventWriter, void * apAppData,
                                          const EventOptions * apOptions)
{
    chip::EventId event_id = 0;

    // Make sure we're alive.
    VerifyOrExit(mState != kLoggingManagementState_Shutdown, /* no-op */);

    event_id = LogEventPrivate(aSchema, aEventWriter, apAppData, apOptions);

exit:
    return event_id;
}

// Note: the function below must be called with the critical section
// locked, and only when the logger is not shutting down

inline chip::EventId LoggingManagement::LogEventPrivate(const EventSchema & aSchema, EventWriterFunct aEventWriter,
                                                        void * apAppData, const EventOptions * apOptions)
{
    chip::EventId event_id = 0;
    CircularTLVWriter writer;
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    size_t requestSize             = CHIP_CONFIG_EVENT_SIZE_RESERVE;
    bool didWriteEvent             = false;
    CircularEventBuffer checkpoint = *mpEventBuffer;
    EventLoadOutContext ctxt = EventLoadOutContext(writer, aSchema.mPriority, GetPriorityBuffer(aSchema.mPriority)->mLastEventID);
    EventOptions opts        = EventOptions(static_cast<timestamp_t>(System::Timer::GetCurrentEpoch()));

    // check whether the entry is to be logged or discarded silently
    VerifyOrExit(aSchema.mPriority <= GetCurrentPriority(aSchema.mClusterId), /* no-op */);

    // Create all event specific data
    // Timestamp; encoded as a delta time
    if ((apOptions != NULL) && (apOptions->timestampType == kTimestampType_System))
    {
        opts.timestamp.systemTimestamp = apOptions->timestamp.systemTimestamp;
    }

    if (GetPriorityBuffer(aSchema.mPriority)->mFirstEventTimestamp == 0)
    {
        GetPriorityBuffer(aSchema.mPriority)->AddEvent(opts.timestamp.systemTimestamp);
    }

    if (apOptions != NULL)
    {
        opts.eventSource     = apOptions->eventSource;
        opts.relatedPriority = apOptions->relatedPriority;
    }

    ctxt.mFirst          = false;
    ctxt.mCurrentEventID = GetPriorityBuffer(aSchema.mPriority)->mLastEventID;
    ctxt.mCurrentTime    = GetPriorityBuffer(aSchema.mPriority)->mLastEventTimestamp;

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

        err = BlitEvent(&ctxt, aSchema, aEventWriter, apAppData, &opts);

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
        event_id = GetPriorityBuffer(aSchema.mPriority)->VendEventID();

        {
            GetPriorityBuffer(aSchema.mPriority)->AddEvent(opts.timestamp.systemTimestamp);

#if CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
            ChipLogDetail(EventLogging,
                          "LogEvent event id: %u importance: %u profile id: 0x%x structure id: 0x%x sys timestamp: 0x%" PRIx32,
                          event_id, aSchema.mPriority, aSchema.mClusterId, aSchema.mStructureType, opts.timestamp.systemTimestamp);
#endif // CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
        }

        ScheduleFlushIfNeeded(apOptions == NULL ? false : apOptions->urgent);
    }

    return event_id;
}

/**
 * @brief
 *   ThrottleLogger elevates the effective logging level to the Production level.
 *
 */
void LoggingManagement::ThrottleLogger(void)
{
    ChipLogProgress(EventLogging, "LogThrottle on");

    __sync_add_and_fetch(&mThrottled, 1);
}

/**
 * @brief
 *   UnthrottleLogger restores the effective logging level to the configured logging level.
 *
 */
void LoggingManagement::UnthrottleLogger(void)
{
    uint32_t throttled = __sync_sub_and_fetch(&mThrottled, 1);

    if (throttled == 0)
    {
        ChipLogProgress(EventLogging, "LogThrottle off");
    }
}

// internal API, used to copy events to external buffers
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

exit:
    return err;
}

/**
 * @brief Internal iterator function used to scan and filter though event logs
 *
 * The function is used to scan through the event log to find events matching the spec in the supplied context.
 */

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
        {
            loadOutContext->mCurrentTime += event.mDeltaTime;
            VerifyOrExit(loadOutContext->mCurrentEventID < loadOutContext->mStartingEventID, err = CHIP_EVENT_ID_FOUND);
            loadOutContext->mCurrentEventID++;
        }
    }

exit:
    return err;
}

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
CHIP_ERROR LoggingManagement::CopyEventsSince(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVWriter checkpoint;
    EventLoadOutContext * loadOutContext = static_cast<EventLoadOutContext *>(apContext);

    err = EventIterator(aReader, aDepth, apContext);
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
        loadOutContext->mCurrentEventID++;
    }

exit:
    return err;
}

/**
 * @brief
 *   A function to retrieve events of specified importance since a specified event ID.
 *
 * Given a chip::TLV::TLVWriter, an importance type, and an event ID, the
 * function will fetch events of specified importance since the
 * specified event.  The function will continue fetching events until
 * it runs out of space in the chip::TLV::TLVWriter or in the log. The function
 * will terminate the event writing on event boundary.
 *
 * @param[in] aWriter     The writer to use for event storage
 *
 * @param[in] aPriority The importance of events to be fetched
 *
 * @param[inout] aEventID On input, the ID of the event immediately
 *                         prior to the one we're fetching.  On
 *                         completion, the ID of the last event
 *                         fetched.
 *
 * @retval #CHIP_END_OF_TLV             The function has reached the end of the
 *                                       available log entries at the specified
 *                                       importance level
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
CHIP_ERROR LoggingManagement::FetchEventsSince(TLVWriter & aWriter, PriorityLevel aPriority, chip::EventId & aEventID)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    const bool recurse = false;
    TLVReader reader;

    EventLoadOutContext context(aWriter, aPriority, aEventID);

    CircularEventBuffer * buf = mpEventBuffer;

    while (!buf->IsFinalDestinationForPriority(aPriority))
    {
        buf = buf->mpNext;
    }

    context.mCurrentTime    = buf->mFirstEventTimestamp;
    context.mCurrentEventID = buf->mFirstEventID;
    err                     = GetEventReader(reader, aPriority);
    SuccessOrExit(err);

    err = chip::TLV::Utilities::Iterate(reader, CopyEventsSince, &context, recurse);

exit:
    aEventID = (chip::EventId)(context.mCurrentEventID);

    return err;
}

/**
 * @brief
 *   A helper method useful for examining the in-memory log buffers
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
 * @return                 #CHIP_NO_ERROR Unconditionally.
 */
CHIP_ERROR LoggingManagement::GetEventReader(TLVReader & aReader, PriorityLevel aPriority)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CircularEventBuffer * buffer;
    CircularEventReader reader;
    for (buffer = mpEventBuffer; buffer != NULL && !buffer->IsFinalDestinationForPriority(aPriority); buffer = buffer->mpNext)
        ;
    VerifyOrExit(buffer != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    reader.Init(buffer);

    aReader.Init(reader);
exit:
    return err;
}

// internal API
CHIP_ERROR LoggingManagement::FetchEventParameters(const TLVReader & aReader, size_t aDepth, void * apContext)
{
    CHIP_ERROR err                  = CHIP_NO_ERROR;
    EventEnvelopeContext * envelope = static_cast<EventEnvelopeContext *>(apContext);
    TLVReader reader;
    uint16_t extPriority; // Note: the type here matches the type case in LoggingManagement::LogEvent, importance section
    reader.Init(aReader);

    VerifyOrExit(envelope->mNumFieldsToRead > 0, err = CHIP_END_OF_TLV);

    if (reader.GetTag() == chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_PriorityLevel))
    {
        err = reader.Get(extPriority);
        SuccessOrExit(err);
        envelope->mPriority = static_cast<PriorityLevel>(extPriority);

        envelope->mNumFieldsToRead--;
    }

    if (reader.GetTag() == chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_DeltaSystemTimeStamp))
    {
        err = reader.Get(envelope->mDeltaTime);
        SuccessOrExit(err);

        envelope->mNumFieldsToRead--;
    }

exit:
    return err;
}

// internal API: determine priority of an event, and the space the event requires

CHIP_ERROR LoggingManagement::EvictEvent(CHIPCircularTLVBuffer & apBuffer, void * apAppData, TLVReader & inReader)
{
    ReclaimEventCtx * ctx             = static_cast<ReclaimEventCtx *>(apAppData);
    CircularEventBuffer * eventBuffer = ctx->mpEventBuffer;
    TLVType containerType;
    EventEnvelopeContext context;
    const bool recurse = false;
    CHIP_ERROR err;
    PriorityLevel imp = kPriorityLevel_Invalid;

    // pull out the delta time, pull out the importance
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
        ChipLogProgress(EventLogging, "Dropped events do to overflow: { priority_level: %d, count: %d };", imp, numEventsToDrop);
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

// Notes: called as a result of the timer expiration.  Main job:
// figure out whether trigger still applies, if it does, then kick off
// the upload.  If it does not, perform the appropriate backoff.

void LoggingManagement::LoggingFlushHandler(System::Layer * systemLayer, void * appState, INET_ERROR err)
{
    LoggingManagement * logger = static_cast<LoggingManagement *>(appState);
    logger->FlushHandler(systemLayer, err);
}

// FlushHandler is only called by the CHIP thread. As such, guard variables
// do not need to be atomically set or checked.
void LoggingManagement::FlushHandler(System::Layer * inSystemLayer, INET_ERROR inErr)
{
    switch (mState)
    {

    case kLoggingManagementState_Idle: {
        if (mpExchangeMgr != NULL)
        {
            chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine()->Run();
            mUploadRequested = false;
        }

        break;
    }

    case kLoggingManagementState_InProgress:
    case kLoggingManagementState_Shutdown: {
        // should never end in these states in this function
        break;
    }
    }
}

void LoggingManagement::SignalUploadDone(void) {}

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
 * The pending state of the event log is indicated by
 * `mUploadRequested` variable. Since this function can be called by
 * multiple threads, `mUploadRequested` must be atomically read and
 * set, to avoid scheduling a redundant `LoggingFlushHandler` before
 * the notification has been sent.
 *
 * @param inRequestFlush A boolean value indicating whether the flush
 *                       should be scheduled regardless of internal
 *                       buffer management policy.
 *
 * @retval #CHIP_ERROR_INCORRECT_STATE LoggingManagement module was not initialized fully.
 * @retval #CHIP_NO_ERROR              On success.
 */
CHIP_ERROR LoggingManagement::ScheduleFlushIfNeeded(bool inRequestFlush)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    return err;
}

CHIP_ERROR LoggingManagement::SetLoggingEndpoint(chip::EventId * apEventEndpoints, size_t aNumPriorityLevels,
                                                 size_t & aBytesOffloaded)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    CircularEventBuffer * eventBuffer = mpEventBuffer;

    aBytesOffloaded = mBytesWritten;

    while (eventBuffer != NULL && aNumPriorityLevels > 0)
    {
        if ((eventBuffer->mPriority >= kPriorityLevel_First) &&
            ((static_cast<size_t>(eventBuffer->mPriority - kPriorityLevel_First)) < aNumPriorityLevels))
        {
            apEventEndpoints[eventBuffer->mPriority - kPriorityLevel_First] = eventBuffer->mLastEventID;
        }
        eventBuffer = eventBuffer->mpNext;
    }

    return err;
}

/**
 * @brief
 *   Get the total number of bytes written (across all event
 *   importances) to this log since its instantiation
 *
 * @returns The number of bytes written to the log.
 */
uint32_t LoggingManagement::GetBytesWritten(void) const
{
    return mBytesWritten;
}

void LoggingManagement::NotifyEventsDelivered(PriorityLevel aPriority, chip::EventId inLastDeliveredEventID,
                                              uint64_t inRecipientNodeID)
{}

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
CircularEventBuffer::CircularEventBuffer(uint8_t * apBuffer, uint32_t aBufferLength, CircularEventBuffer * apPrev,
                                         CircularEventBuffer * apNext) :
    CHIPCircularTLVBuffer(apBuffer, aBufferLength),
    mpPrev(apPrev), mpNext(apNext), mPriority(kPriorityLevel_First), mFirstEventID(1), mLastEventID(0), mFirstEventTimestamp(0),
    mLastEventTimestamp(0), mpEventIdCounter(nullptr)
{
    // TODO: hook up the platform-specific persistent event ID.
}

/**
 * @brief
 *   A helper function that determines whether the event of
 *   specified priority is dropped from this buffer.
 *
 * @param[in]   aPriority   Priority of the event.
 *
 * @retval true  The event will be dropped from this buffer as
 *               a result of queue overflow.
 * @retval false The event will be bumped to the next queue.
 */
bool CircularEventBuffer::IsFinalDestinationForPriority(PriorityLevel aPriority) const
{
    return !((mpNext != NULL) && (mpNext->mPriority >= aPriority));
}

/**
 * @brief
 *   Given a timestamp of an event, compute the delta time to store in the log
 *
 * @param aEventTimestamp The event timestamp.
 *
 * @return int32_t         Time delta to encode for the event.
 */
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
    mFirstEventID = static_cast<chip::EventId>(mFirstEventID + aNumEvents);
}

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
void CircularEventReader::Init(chip::app::reporting::CircularEventBuffer * apBackStore)
{
    chip::TLV::CircularTLVReader reader;
    CircularEventBuffer * prev;

    reader.Init(*apBackStore);
    TLVReader::Init(reader);

    chip::TLV::TLVReader::Init(reader);

    for (prev = apBackStore->mpPrev; prev != NULL; prev = prev->mpPrev)
    {
        reader.Init(*prev);
        mMaxLen += reader.GetRemainingLength();
    }
}

CHIP_ERROR CircularEventBuffer::GetNextBuffer(TLVReader & aReader, const uint8_t *& aBufStart, uint32_t & aBufLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = CHIPCircularTLVBuffer::GetNextBuffer(aReader, aBufStart, aBufLen);
    SuccessOrExit(err);

    if ((aBufLen == 0) && (mpPrev != NULL))
    {
        err = mpPrev->GetNextBuffer(aReader, aBufStart, aBufLen);
        aReader.SetBackingStore(mpPrev);
    }

exit:
    return err;
}

CopyAndAdjustDeltaTimeContext::CopyAndAdjustDeltaTimeContext(TLVWriter * aWriter, EventLoadOutContext * inContext) :
    mWriter(aWriter), mContext(inContext)
{}

EventEnvelopeContext::EventEnvelopeContext(void) :
    mNumFieldsToRead(2), // read out importance and either system or utc delta time. events do not store both deltas.
    mDeltaTime(0), mPriority(kPriorityLevel_First)
{}

} // namespace reporting
} // namespace app
} // namespace chip
