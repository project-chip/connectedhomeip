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

/// Aids in writing invoke replies:
///   - Has access to the raw data encoder for response data content (via `ResponseEncoder`)
///   - `Complete` handling:
///      - MUST be called exactly once
///      - called with an error, makes TLV data written to be discarded (Invokes may only either
///        succeed or fully fail)
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
    ///     If such an error is returned, the caller MUST retry by calling
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
        if (!mCompleted)
        {
            mWriter->Complete(Protocols::InteractionModel::Status::Failure);
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
        mCompleted = false;
        return mWriter->FlushPendingResponses();
    }

    /// Call "Complete" without the automatic retries.
    ///
    /// Use this in conjunction with the other Raw* calls
    CHIP_ERROR RawComplete(StatusIB status)
    {
        VerifyOrReturnError(!mCompleted, CHIP_ERROR_INCORRECT_STATE);
        mCompleted = true;
        return mWriter->Complete(status);
    }

    /// Complete the given command.
    ///
    /// Automatically handles retries for sending.
    ///
    /// Any error returned by this are final and not retriable
    /// as a retry for CHIP_ERROR_BUFFER_TOO_SMALL is already built in.
    CHIP_ERROR Complete(StatusIB status)
    {
        VerifyOrReturnError(!mCompleted, CHIP_ERROR_INCORRECT_STATE);
        mCompleted     = true;
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
    ///
    /// Any error returned by this are final and not retriable
    /// as a retry for CHIP_ERROR_BUFFER_TOO_SMALL is already built in.
    template <typename ReplyData>
    CHIP_ERROR Send(const ReplyData & data)
    {
        VerifyOrReturnError(!mCompleted, CHIP_ERROR_INCORRECT_STATE);
        mCompleted     = true;
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
    InvokeResponder * mWriter;
    bool mCompleted = false;
};

enum ReplyAsyncFlags
{
    // Some commmands that are expensive to process (e.g. crypto).
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

    // Send a reply "NOW" to the given invoke
    virtual AutoCompleteInvokeResponder Reply() = 0;

    // Reply "later" to the command. This allows async processing. A reply will be forced
    // when the returned InvokeReply is destroyed.
    virtual std::unique_ptr<InvokeReply> ReplyAsync(BitFlags<ReplyAsyncFlags> flags) = 0;
};

} // namespace InteractionModel
} // namespace app
} // namespace chip
