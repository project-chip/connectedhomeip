/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

void ChunkedWriteCallback::OnResponse(const WriteClient * apWriteClient, const ConcreteDataAttributePath & aPath, StatusIB aStatus)
{
    // We may send a chunked list. To make the behavior consistent whether a list is being chunked or not,
    // we merge the write responses for a chunked list here and provide our consumer with a single status response.
    if (mProcessingAttributePath.HasValue())
    {
        // This is not the first write response.
        if (IsAppendingToLastItem(aPath))
        {
            // This is a response on the same path as what we already have stored. Report the first
            // failure status we encountered, and ignore subsequent ones.
            if (mAttributeStatus.IsSuccess())
            {
                mAttributeStatus = aStatus;
            }
            return;
        }

        // This is a response to another attribute write. Report the final result of last attribute write.
        callback->OnResponse(apWriteClient, mProcessingAttributePath.Value(), mAttributeStatus);
    }

    // This is the first report for a new attribute.  We assume it will never be a list item operation.
    if (aPath.IsListItemOperation())
    {
        aStatus = StatusIB(CHIP_ERROR_INCORRECT_STATE);
    }

    mProcessingAttributePath.SetValue(aPath);
    mAttributeStatus = aStatus;
    // For the last status in the response, we will call the application callback in OnDone()
}

void ChunkedWriteCallback::OnError(const WriteClient * apWriteClient, CHIP_ERROR aError)
{
    callback->OnError(apWriteClient, aError);
}

void ChunkedWriteCallback::OnDone(WriteClient * apWriteClient)
{
    if (mProcessingAttributePath.HasValue())
    {
        // We have a cached status that has yet to be reported to the application so report it now.
        // If we failed to receive the response, or we received a malformed response, OnResponse won't be called,
        // mProcessingAttributePath will be Missing() in this case.
        callback->OnResponse(apWriteClient, mProcessingAttributePath.Value(), mAttributeStatus);
    }

    mProcessingAttributePath = NullOptional;
    mAttributeStatus         = StatusIB();

    callback->OnDone(apWriteClient);
}

bool ChunkedWriteCallback::IsAppendingToLastItem(const ConcreteDataAttributePath & aPath)
{
    if (!aPath.IsListItemOperation())
    {
        return false;
    }
    if (!mProcessingAttributePath.HasValue() || !(mProcessingAttributePath.Value() == aPath))
    {
        return false;
    }
    return aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem;
}

} // namespace app
} // namespace chip
