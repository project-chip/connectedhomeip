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

#include <app/data-model/WrappedStructEncoder.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace InteractionModel {

/// Aids in writing invoke replies:
///   - Has access to the raw data encoder for response data content (via `ReplyEncoder`)
///   - `Complete` handling:
///      - MUST be called exactly once
///      - called with an error, makes TLV data written to be discarded (Invokes may only either
///        succeed or fully fail)
class InvokeResponder
{
public:
    virtual ~InvokeResponder()                           = default;
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
    /// again. If reply data is needed, the complete ReplyEncoder + Complete
    /// call chain MUST be re-run.
    virtual CHIP_ERROR FlushPendingReplies() = 0;

    /// Reply with a data payload.
    ///
    /// MUST be called at most once per reply.
    /// Can be called a 2nd time after a `FlushPendingReplies()` call
    ///
    ///   - replyCommandId must correspond with the data encoded in the returned encoder
    ///   - Complete(CHIP_NO_ERROR) MUST be called to flush the reply
    virtual DataModel::WrappedStructEncoder & ReplyEncoder(CommandId replyCommandId) = 0;

    /// Signal completing of the reply.
    ///
    /// MUST be called exactly once to signal a reply to be sent.
    /// If this returns CHIP_ERROR_BUFFER_TOO_SMALL, this can be called a 2nd time after
    /// a FlushPendingReplies.
    ///
    /// Argument behavior:
    ///  - If an ERROR is given (i.e. NOT CHIP_NO_ERROR) an error response will be given to the
    ///    command.
    ///  - If CHIP_NO_ERROR is given:
    ///    - if a ReplyEncoder() was called, a data reply will be sent
    ///    - if no ReplyEncoder() was called, a Success reply will be sent
    ///
    /// Returns success/failure state. One error code MUST be handled in particular:
    ///
    ///   - CHIP_ERROR_BUFFER_TOO_SMALL will return IF AND ONLY IF the responder was unable
    ///     to fully serialize the given reply/error data.
    ///
    ///     If such an error is returned, the caller MUST retry by calling
    ///
    virtual CHIP_ERROR Complete(CHIP_ERROR error) = 0;
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
            mWriter->Complete(CHIP_IM_GLOBAL_STATUS(Failure));
        }
    }

    /// Direct access to reply encoding.
    ///
    /// Use this only in conjunction with the other Raw* calls
    DataModel::WrappedStructEncoder & RawReplyEncoder(CommandId replyCommandId) { return mWriter->ReplyEncoder(replyCommandId); }

    /// Direct access to flushing replies
    ///
    /// Use this only in conjunction with the other Raw* calls
    CHIP_ERROR RawFlushPendingReplies() { return mWriter->FlushPendingReplies(); }

    /// Call "Complete" without the automatic retries.
    ///
    /// Use this in conjunction with the other Raw* calls
    CHIP_ERROR RawComplete(CHIP_ERROR error) { return mWriter->Complete(error); }

    /// Complete the given command.
    ///
    /// Automatically handles retries for sending.
    CHIP_ERROR Complete(CHIP_ERROR error)
    {
        mCompleted     = true;
        CHIP_ERROR err = mWriter->Complete(error);

        if (err != CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            return err;
        }

        // retry once. Failure to flush is permanent.
        ReturnErrorOnFailure(mWriter->FlushPendingReplies());
        return mWriter->Complete(error);

        return err;
    }

    /// Sends the specified data structure as a response
    ///
    /// This version of the send has built-in RETRY and handles
    /// Flush/Complete automatically.
    ///
    template <typename ReplyData>
    CHIP_ERROR Send(const ReplyData & data)
    {
        CHIP_ERROR err = data.Encode(ReplyEncoder(ReplyData::GetCommandId()));
        LogErrorOnFailure(err);
        err = mWriter->Complete(err);
        if (err != CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            return err;
        }

        // retry once. Failure to flush is permanent.
        ReturnErrorOnFailure(mWriter->FlushPendingReplies());
        err = data.Encode(ReplyEncoder(ReplyData::GetCommandId()));
        LogErrorOnFailure(err);
        return mWriter->Complete(err);
    }

private:
    InvokeResponder * mWriter;
    bool mCompleted = false;
};

enum ReplyAsyncFlags
{
    // Some commmands that are expensive to process (e.g. crypto)
    // should ack right away to improve reliability and reduce
    // needless retries.
    kFlushAckRightAway = 0x0001,
};

class InvokeReply
{
public:
    virtual ~InvokeReply() = default;

    // reply with no data
    CHIP_ERROR Reply(CHIP_ERROR e) { return this->Reply().Complete(e); }

    // Send a reply "NOW" to the given invoke
    virtual AutoCompleteInvokeResponder Reply() = 0;

    // Reply "later" to the command. This allows async processing. A reply will be forced
    // when the returned InvokeReply is destroyed.
    virtual std::unique_ptr<InvokeReply> ReplyAsync(BitFlags<ReplyAsyncFlags> flags) = 0;
};

} // namespace InteractionModel
} // namespace app
} // namespace chip
