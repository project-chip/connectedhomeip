/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include <app/MessageDef/StatusIB.h>
#include <app/data-model/WrappedStructEncoder.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace InteractionModel {

/// Handles encoding of an invoke response for a specific invoke request.
///
/// This class handles a single request (i.e. a CommandDataIB within the
/// matter protocol) and is responsible for constructing its corresponding
/// response (i.e. a InvokeResponseIB within the matter protocol)
///
/// Invoke responses MUST contain exactly ONE of:
///   - response data (accessed via `ResponseEncoder`)
///   - A status, which may be success or failure, both of which may
///     contain a cluster-specific error code.
///
/// To encode a response, `Complete` MUST be called.
///
/// `Complete` requirements
///   - Complete with InteractionModel::Status::Success will respond with data
///     some response data was written.
///   - Any other case (including success with cluster specific codes) implies
///     no response data and a status will be encoded instead
///       - this includes the case when some response data was written already.
///         In that case, the response data will be rolled back and only the status
///         will be encoded.
///
/// Creating a response MAY be retried at most once, if and only if `Complete`
/// returns CHIP_ERROR_BUFFER_TOO_SMALL. Retry attempts MUST not exceed 1:
///   - FlushPendingResponses MUST be called to make as much buffer space as possible
///     available for encoding
///   - The response encoding (including `ResponseEncoder` usage and calling Complete)
///     MUST be retried once more. If the final Complete returns an error, the result
///     of the invoke will be an error status.
///
class InvokeResponder
{
public:
    virtual ~InvokeResponder() = default;

    // Copying not allowed since underlying requirement is that on deletion of this
    // object, a reply will be sent.
    InvokeResponder(const InvokeResponder &)             = delete;
    InvokeResponder & operator=(const InvokeResponder &) = delete;

    /// Flush any pending replies before encoding the current reply.
    ///
    /// MAY be called at most once.
    ///
    /// This function is intended to provided the ability to retry sending a reply
    /// if a reply encoding fails due to insufficient buffer.
    ///
    /// Call this if `Complete(...)` returns CHIP_ERROR_BUFFER_TOO_SMALL and try
    /// again. If reply data is needed, the complete ResponseEncoder + Complete
    /// call chain MUST be re-run.
    virtual CHIP_ERROR FlushPendingResponses() = 0;

    /// Reply with a data payload.
    ///
    /// MUST be called at most once per reply.
    /// Can be called a 2nd time after a `FlushPendingResponses()` call
    ///
    ///   - responseCommandId must correspond with the data encoded in the returned encoder
    ///   - Complete(CHIP_NO_ERROR) MUST be called to flush the reply
    ///
    /// If encoder returns CHIP_ERROR_BUFFER_TOO_SMALL, FlushPendingResponses should be
    /// used to attempt to free up buffer space then encoding should be tried again.
    virtual DataModel::WrappedStructEncoder & ResponseEncoder(CommandId responseCommandId) = 0;

    /// Signal completing of the reply.
    ///
    /// MUST be called exactly once to signal a response is to be recorded to be sent.
    /// The error code (and the data encoded by ResponseEncoder) may be buffered for
    /// sending among other batched responses.
    ///
    /// If this returns CHIP_ERROR_BUFFER_TOO_SMALL, this can be called a 2nd time after
    /// a FlushPendingResponses.
    ///
    /// Argument behavior:
    ///  - Commands can only be replied with ONE of the following (spec 8.9.4.4):
    ///      - command data (i.e. ResponseEncoder contents)
    ///      - A status (including success/error/cluster-specific-success-or-error )
    ///  - As a result there are two possible paths:
    ///      - IF a Status::Success is given (WITHOUT cluster specific status), then
    ///        the data in ResponseEncoder is sent as a reply. If no data was sent,
    ///        a invoke `Status::Success` with no cluster specific data is sent
    ///      - OTHERWISE any previously encoded data via ResponseEncoder is discarded
    ///        and the given reply (success with cluster status or failure) is sent
    ///        as a reply to the invoke.
    ///
    ///
    /// Returns success/failure state. One error code MUST be handled in particular:
    ///
    ///   - CHIP_ERROR_BUFFER_TOO_SMALL will return IF AND ONLY IF the responder was unable
    ///     to fully serialize the given reply/error data.
    ///
    ///     If such an error is returned, the caller MUST retry by calling FlushPendingResponses
    ///     first and then re-encoding the reply content (use ResponseEncoder if applicable and
    ///     call Complete again)
    ///
    ///   - Any other error (i.e. different from CHIP_NO_ERROR) mean that the invoke response
    ///     will contain an error and such an error is considered permanent.
    ///
    virtual CHIP_ERROR Complete(StatusIB error) = 0;
};

/// Enforces that once acquired, Complete will be called on the underlying writer
class AutoCompleteInvokeResponder
{
public:
    // non-copyable: once you have a handle, keep it
    AutoCompleteInvokeResponder(const AutoCompleteInvokeResponder &)             = delete;
    AutoCompleteInvokeResponder & operator=(const AutoCompleteInvokeResponder &) = delete;

    AutoCompleteInvokeResponder(InvokeResponder * writer) : mWriter(writer) {}
    ~AutoCompleteInvokeResponder()
    {
        if (mCompleteState != CompleteState::kComplete)
        {
            mWriter->Complete(StatusIB{Protocols::InteractionModel::Status::Failure});
        }
    }

    /// Direct access to reply encoding.
    ///
    /// Use this only in conjunction with the other Raw* calls
    DataModel::WrappedStructEncoder & RawResponseEncoder(CommandId replyCommandId)
    {
        return mWriter->ResponseEncoder(replyCommandId);
    }

    /// Direct access to flushing replies
    ///
    /// Use this only in conjunction with the other Raw* calls
    CHIP_ERROR RawFlushPendingReplies()
    {
        // allow a flush if we never called it (this may not be reasonable, however
        // we accept an early flush) or if flush is expected
        VerifyOrReturnError((mCompleteState == CompleteState::kNeverCalled) || (mCompleteState == CompleteState::kFlushExpected),
                            CHIP_ERROR_INCORRECT_STATE);
        mCompleteState = CompleteState::kFlushed;
        return mWriter->FlushPendingResponses();
    }

    /// Call "Complete" without the automatic retries.
    ///
    /// Use this in conjunction with the other Raw* calls
    CHIP_ERROR RawComplete(StatusIB status)
    {
        VerifyOrReturnError((mCompleteState == CompleteState::kNeverCalled) || (mCompleteState == CompleteState::kFlushed),
                            CHIP_ERROR_INCORRECT_STATE);
        CHIP_ERROR err = mWriter->Complete(status);
        if ((err == CHIP_ERROR_BUFFER_TOO_SMALL) && (mCompleteState == CompleteState::kNeverCalled))
        {
            mCompleteState = CompleteState::kFlushExpected;
        }
        else
        {
            mCompleteState = CompleteState::kComplete;
        }
        return err;
    }

    /// Complete the given command.
    ///
    /// Automatically handles retries for sending.
    /// Cannot be called after Raw* methods are used.
    ///
    /// Any error returned by this are final and not retriable
    /// as a retry for CHIP_ERROR_BUFFER_TOO_SMALL is already built in.
    CHIP_ERROR Complete(StatusIB status)
    {
        VerifyOrReturnError(mCompleteState == CompleteState::kNeverCalled, CHIP_ERROR_INCORRECT_STATE);
        // this is a final complete, including retry handling
        mCompleteState = CompleteState::kComplete;
        CHIP_ERROR err = mWriter->Complete(status);

        if (err != CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            return err;
        }

        // retry once. Failure to flush is permanent.
        ReturnErrorOnFailure(mWriter->FlushPendingResponses());
        return mWriter->Complete(status);
    }

    /// Sends the specified data structure as a response
    ///
    /// This version of the send has built-in RETRY and handles
    /// Flush/Complete automatically.
    /// Cannot be called after Raw* methods are used.
    ///
    /// Any error returned by this are final and not retriable
    /// as a retry for CHIP_ERROR_BUFFER_TOO_SMALL is already built in.
    template <typename ReplyData>
    CHIP_ERROR Send(const ReplyData & data)
    {
        VerifyOrReturnError(mCompleteState == CompleteState::kNeverCalled, CHIP_ERROR_INCORRECT_STATE);
        // this is a final complete, including retry handling
        mCompleteState = CompleteState::kComplete;
        CHIP_ERROR err = data.Encode(ResponseEncoder(ReplyData::GetCommandId()));
        if (err != CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            LogErrorOnFailure(err);
            err = mWriter->Complete(StatusIB(err));
        }
        if (err != CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            return err;
        }

        // retry once. Failure to flush is permanent.
        ReturnErrorOnFailure(mWriter->FlushPendingResponses());
        err = data.Encode(ResponseEncoder(ReplyData::GetCommandId()));

        // If encoding fails, we will end up sending an error back to the other side
        // the caller
        LogErrorOnFailure(err);
        if (err == CHIP_NO_ERROR)
        {
            err = mWriter->Complete(StatusIB(err));
        }
        else
        {
            // Error in "complete" is not something we can really forward anymore since
            // we already got an error in Encode ... just log this.
            LogErrorOnFailure(mWriter->Complete(StatusIB(err)));
        }

        return err;
    }

private:
    // Contract says that complete may only be called twice:
    //   - initial complete
    //   - again after a `Flush`
    // The states here expect we are in:
    //
    //    +----------------------------Flush---------|
    //    |                                          v
    //  NEVER --Complete--> F_EXPECTED --Flush--> FLUSHED --Complete--> COMPLETE
    //             |                                                    ^
    //             +-------------(success or permanent error)-----------|
    enum class CompleteState
    {
        kNeverCalled,
        kFlushExpected,
        kFlushed,
        kComplete,
    };

    InvokeResponder * mWriter;
    CompleteState mCompleteState = CompleteState::kNeverCalled;
};

enum ReplyAsyncFlags
{
    // Some commands that are expensive to process (e.g. crypto).
    // Implementations may choose to send an ack on the message right away to
    // avoid MRP retransmits.
    kSlowCommandHandling = 0x0001,
};

class InvokeReply
{
public:
    virtual ~InvokeReply() = default;

    // reply with no data
    CHIP_ERROR Reply(StatusIB status) { return this->Reply().Complete(status); }

    // Enqueue the content of the reply at this point in time (rather than Async sending it).
    //
    // Implementations will often batch several replies into one packet for batch commands,
    // so it will be implementation-specific on when the actual reply packet is
    // sent.
    virtual AutoCompleteInvokeResponder Reply() = 0;

    // Reply "later" to the command. This allows async processing. A reply will be forced
    // when the returned InvokeReply is destroyed.
    //
    // NOTE: Each InvokeReply is associated with a separate `CommandDataIB` within batch
    //       commands. When replying asynchronously, each InvokeReply will set the response
    //       data for the given commandpath/ref only.
    //
    // IF empty pointer is returned, insufficient memory to reply async is available and
    // this should be handled (e.g. by returning an error to the handler/replying with
    // an errorcode synchronously).
    virtual std::unique_ptr<InvokeReply> ReplyAsync(BitFlags<ReplyAsyncFlags> flags) = 0;
};

} // namespace InteractionModel
} // namespace app
} // namespace chip
