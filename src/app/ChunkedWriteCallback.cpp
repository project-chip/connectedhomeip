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

#include <app/ChunkedWriteCallback.h>

namespace chip {
namespace app {

void ChunkedWriteCallback::OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteAttributePath & aPath,
                                      app::StatusIB status)
{
    // We may send a chunked list, to make the behavior consistent, we merge the write response here.
    if (mLastAttributePath.HasValue())
    {
        // This is not the first write response.
        if (!(mLastAttributePath.Value() == aPath))
        {
            // This is a response to another attribute write. Report the final result of last attribute write.
            callback->OnResponse(apWriteClient, mLastAttributePath.Value(), mAttributeStatus);
        }
        else if (mAttributeStatus.mStatus != Protocols::InteractionModel::Status::Success)
        {
            // We this is a response from previous write request, but it was failed before.
            // We only report the first failure, so skip the following errors.
            return;
        }
        /**
         * else The path is the same as the previous one
         */
    }
    /**
     * else: This is the first report message.
     */
    mLastAttributePath.SetValue(aPath);
    mAttributeStatus = status;

    // For the last status in the response, we will call the application callback in OnDone()
}

void ChunkedWriteCallback::OnError(const app::WriteClient * apWriteClient, const app::StatusIB & aStatus, CHIP_ERROR aError)
{
    callback->OnError(apWriteClient, aStatus, aError);
}

void ChunkedWriteCallback::OnDone(app::WriteClient * apWriteClient)
{
    if (mLastAttributePath.HasValue())
    {
        // We have one status not reported to the application, call it now.
        // If we failed to receive the response, or we received a malformed response, OnResponse won't be called,
        // mLastAttributePath will be Missing() in this case.
        callback->OnResponse(apWriteClient, mLastAttributePath.Value(), mAttributeStatus);
    }

    callback->OnDone(apWriteClient);
}

} // namespace app
} // namespace chip
