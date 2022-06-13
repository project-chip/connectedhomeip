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

#pragma once

#include <app/WriteClient.h>

namespace chip {
namespace app {

/*
 * This is an adapter that intercepts calls that deliver status codes from the WriteClient and
 * selectively "merge"s the status codes for a chunked list write as follows:
 * - If the whole list was successfully written, callback->OnResponse will be called with success.
 * - If any element in the list was not successfully written, callback->OnResponse will be called with the first error received.
 * - callback->OnResponse will always have NotList as mListOp since we have merged the chunked responses.
 * The merge logic assumes all list operations are part of list chunking.
 */
class ChunkedWriteCallback : public WriteClient::Callback
{
public:
    ChunkedWriteCallback(WriteClient::Callback * apCallback) : callback(apCallback) {}

    void OnResponse(const WriteClient * apWriteClient, const ConcreteDataAttributePath & aPath, StatusIB status) override;
    void OnError(const WriteClient * apWriteClient, CHIP_ERROR aError) override;
    void OnDone(WriteClient * apWriteClient) override;

private:
    bool IsAppendingToLastItem(const ConcreteDataAttributePath & aPath);

    // We are using the casts between ConcreteAttributePath and ConcreteDataAttributePath, then all paths passed to upper
    // applications will always have NotList as mListOp.
    Optional<ConcreteAttributePath> mProcessingAttributePath;
    StatusIB mAttributeStatus;

    WriteClient::Callback * callback;
};

} // namespace app
} // namespace chip
