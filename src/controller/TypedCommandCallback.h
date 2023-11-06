/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "protocols/interaction_model/Constants.h"
#include <app/CommandSender.h>
#include <app/data-model/Decode.h>
#include <app/data-model/NullObject.h>
#include <functional>

namespace chip {
namespace Controller {

/*
 * This provides an adapter class that implements CommandSender::Callback and provides two additional features:
 *  1. The ability to pass in std::function closures to permit more flexible programming scenarios than are provided by the strict
 *     delegate interface stipulated by CommandSender::Callback
 *
 *  2. Automatic decoding of command response data provided in the TLVReader by the CommandSender callback into a decoded cluster
 * object.
 */
template <typename CommandResponseObjectT>
class TypedCommandCallback final : public app::CommandSender::Callback
{
public:
    using OnSuccessCallbackType =
        std::function<void(const app::ConcreteCommandPath &, const app::StatusIB &, const CommandResponseObjectT &)>;
    using OnErrorCallbackType = std::function<void(CHIP_ERROR aError)>;
    using OnDoneCallbackType  = std::function<void(app::CommandSender * commandSender)>;

    /*
     * Constructor that takes in success, failure and onDone callbacks.
     *
     * The latter can be provided later through the SetOnDoneCallback below in cases where the
     * TypedCommandCallback object needs to be created first before it can be passed in as a closure
     * into a hypothetical OnDoneCallback function.
     */
    TypedCommandCallback(OnSuccessCallbackType aOnSuccess, OnErrorCallbackType aOnError, OnDoneCallbackType aOnDone = {}) :
        mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone)
    {}

    void SetOnDoneCallback(OnDoneCallbackType callback) { mOnDone = callback; }

private:
    void OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aCommandPath,
                    const app::StatusIB & aStatus, TLV::TLVReader * aReader) override;

    void OnError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override
    {
        if (mCalledCallback)
        {
            return;
        }
        mCalledCallback = true;

        mOnError(aError);
    }

    void OnDone(app::CommandSender * apCommandSender) override
    {
        if (!mCalledCallback)
        {
            // This can happen if the server sends a response with an empty
            // InvokeResponses list.  Since we are not sending wildcard command
            // paths, that's not a valid response and we should treat it as an
            // error.  Use the error we would have gotten if we in fact expected
            // a nonempty list.
            OnError(apCommandSender, CHIP_END_OF_TLV);
        }

        mOnDone(apCommandSender);
    }

    OnSuccessCallbackType mOnSuccess;
    OnErrorCallbackType mOnError;
    OnDoneCallbackType mOnDone;

    bool mCalledCallback = false;
};

/*
 * Decodes the data provided by the TLVReader into the templated cluster object that denotes the command response.
 *
 * This function specifically decodes command responses that have actual data payloads.
 */
template <typename CommandResponseObjectT>
void TypedCommandCallback<CommandResponseObjectT>::OnResponse(app::CommandSender * apCommandSender,
                                                              const app::ConcreteCommandPath & aCommandPath,
                                                              const app::StatusIB & aStatus, TLV::TLVReader * aReader)
{
    if (mCalledCallback)
    {
        return;
    }
    mCalledCallback = true;

    CommandResponseObjectT response;
    CHIP_ERROR err = CHIP_NO_ERROR;

    //
    // We're expecting response data in this variant of OnResponse. Consequently, aReader should always be
    // non-null. If it is, it means we received a success status code instead, which is not what was expected.
    //
    VerifyOrExit(aReader != nullptr, err = CHIP_ERROR_SCHEMA_MISMATCH);

    //
    // Validate that the data response we received matches what we expect in terms of its cluster and command IDs.
    //
    VerifyOrExit(aCommandPath.mClusterId == CommandResponseObjectT::GetClusterId() &&
                     aCommandPath.mCommandId == CommandResponseObjectT::GetCommandId(),
                 err = CHIP_ERROR_SCHEMA_MISMATCH);

    err = app::DataModel::Decode(*aReader, response);
    SuccessOrExit(err);

    mOnSuccess(aCommandPath, aStatus, response);

exit:
    if (err != CHIP_NO_ERROR)
    {
        mOnError(err);
    }
}

/*
 * Decodes the data provided by the TLVReader into the templated cluster object that denotes the command response.
 *
 * This function specifically decodes command responses that do not have actual data payloads and where the passed in TLVReader
 * should be null.
 */
template <>
inline void TypedCommandCallback<app::DataModel::NullObjectType>::OnResponse(app::CommandSender * apCommandSender,
                                                                             const app::ConcreteCommandPath & aCommandPath,
                                                                             const app::StatusIB & aStatus,
                                                                             TLV::TLVReader * aReader)
{
    if (mCalledCallback)
    {
        return;
    }
    mCalledCallback = true;

    //
    // If we got a valid reader, it means we received response data that we were not expecting to receive.
    //
    if (aReader != nullptr)
    {
        mOnError(CHIP_ERROR_SCHEMA_MISMATCH);
        return;
    }

    app::DataModel::NullObjectType nullResp;
    mOnSuccess(aCommandPath, aStatus, nullResp);
}

} // namespace Controller
} // namespace chip
