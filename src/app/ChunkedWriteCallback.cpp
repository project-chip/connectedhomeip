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

void ChunkedWriteCallback::OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteDataAttributePath & aPath,
                                      app::StatusIB status)
{
    // We may send a chunked list, to make the behavior consistent, we merge the write response here.
    if (mLastAttributePath.HasValue())
    {
        // This is not the first write response.
        if (!IsAppendingToLastItem(aPath))
        {
            // This is a response to another attribute write. Report the final result of last attribute write.
            callback->OnResponse(apWriteClient, mLastAttributePath.Value(), mAttributeStatus);
        }
        else if (mAttributeStatus.mStatus != Protocols::InteractionModel::Status::Success)
        {
            // This is a response on the same path as the last stored on. We only latch in the first encountered failure on that
            // path, and ignore subsequent ones.
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

void ChunkedWriteCallback::OnError(const app::WriteClient * apWriteClient, CHIP_ERROR aError)
{
    callback->OnError(apWriteClient, aError);
}

void ChunkedWriteCallback::OnDone(app::WriteClient * apWriteClient)
{
    if (mLastAttributePath.HasValue())
    {
        // We have a cached status that has yet to be reported to the application so call it now.
        // If we failed to receive the response, or we received a malformed response, OnResponse won't be called,
        // mLastAttributePath will be Missing() in this case.
        callback->OnResponse(apWriteClient, mLastAttributePath.Value(), mAttributeStatus);
    }

    callback->OnDone(apWriteClient);
}

bool ChunkedWriteCallback::IsAppendingToLastItem(const app::ConcreteDataAttributePath & path)
{
    if (!path.IsListItemOperation())
    {
        return false;
    }
    if (!mLastAttributePath.HasValue() ||
        !(ConcreteAttributePath(path.mEndpointId, path.mClusterId, path.mAttributeId) == mLastAttributePath.Value()))
    {
        return false;
    }
    return path.mListOp == app::ConcreteDataAttributePath::ListOperation::AppendItem;
}

} // namespace app
} // namespace chip
