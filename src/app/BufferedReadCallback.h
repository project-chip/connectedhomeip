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

#include "system/SystemPacketBuffer.h"
#include "system/TLVPacketBufferBackingStore.h"
#include <app/AttributePathParams.h>
#include <app/ReadClient.h>
#include <vector>

namespace chip {
namespace app {

/*
 * This is an adapter that intercepts calls that deliver data from the ReadClient,
 * selectively buffers up list chunks in TLV and reconstitutes them into a singular, contiguous TLV array
 * upon completion of delivery of all chunks. This is then delivered to a compliant ReadClient::Callback
 * without any awareness on their part that chunking happened.
 *
 */
class BufferedReadCallback : public ReadClient::Callback
{
public:
    BufferedReadCallback(Callback & callback) : mCallback(callback) {}

private:
    /*
     * Generates the reconsistuted TLV array from the stored individual list elements
     */
    CHIP_ERROR GenerateListTLV(System::PacketBufferTLVReader & reader);

    /*
     * Dispatch any buffered list data if we need to
     */
    CHIP_ERROR DispatchBufferedData(const ReadClient * apReadClient, const ConcreteAttributePath & aPath, const StatusIB & aStatus,
                                    bool endOfReport = false);

    /*
     * Buffer up list data as they arrive.
     */
    CHIP_ERROR BufferData(const ConcreteAttributePath & aPath, TLV::TLVReader * apReader);

private:
    //
    // ReadClient::Callback
    //
    void OnReportBegin(const ReadClient * apReadClient) override;
    void OnReportEnd(const ReadClient * apReadClient) override;
    void OnAttributeData(const ReadClient * apReadClient, const ConcreteAttributePath & aPath, TLV::TLVReader * apData,
                         const StatusIB & aStatus) override;
    void OnError(const ReadClient * apReadClient, CHIP_ERROR aError) override { return mCallback.OnError(apReadClient, aError); }
    void OnEventData(const ReadClient * apReadClient, TLV::TLVReader & aEventReports) override
    {
        return mCallback.OnEventData(apReadClient, aEventReports);
    }
    void OnDone(ReadClient * apReadClient) override { return mCallback.OnDone(apReadClient); }
    void OnSubscriptionEstablished(const ReadClient * apReadClient) override { mCallback.OnSubscriptionEstablished(apReadClient); }

    ConcreteAttributePath mBufferedPath;
    std::vector<System::PacketBufferHandle> mBufferedList;
    Callback & mCallback;
};

} // namespace app
} // namespace chip
