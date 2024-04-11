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

    virtual DataModel::WrappedStructEncoder & ReplyEncoder() = 0; // write the invoke reply
    virtual CHIP_ERROR Complete(CHIP_ERROR error) = 0; // Complete the response (e.g. CHIP_NO_ERROR or CHIP_IM_GLOBAL_STATUS(...))
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

    DataModel::WrappedStructEncoder & ReplyEncoder() { return mWriter->ReplyEncoder(); }

    CHIP_ERROR Complete(CHIP_ERROR error)
    {
        mCompleted = true;
        return mWriter->Complete(error);
    }

    /// Sends the specified data structure as a response
    template <class ReplyData>
    CHIP_ERROR Send(const ReplyData & data)
    {
        CHIP_ERROR err = data.Encode(ReplyEncoder());
        LogErrorOnFailure(err);
        return Complete(err);
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
