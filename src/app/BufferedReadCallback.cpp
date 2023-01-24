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

#include "lib/core/TLV.h"
#include "lib/core/TLVTags.h"
#include "lib/core/TLVTypes.h"
#include "protocols/interaction_model/Constants.h"
#include "system/SystemPacketBuffer.h"
#include "system/TLVPacketBufferBackingStore.h"
#include <app/BufferedReadCallback.h>
#include <app/InteractionModelEngine.h>
#include <lib/support/ScopedBuffer.h>

namespace chip {
namespace app {

void BufferedReadCallback::OnReportBegin()
{
    mCallback.OnReportBegin();
}

void BufferedReadCallback::OnReportEnd()
{
    CHIP_ERROR err = DispatchBufferedData(mBufferedPath, StatusIB(), true);
    if (err != CHIP_NO_ERROR)
    {
        mCallback.OnError(err);
    }

    mCallback.OnReportEnd();
}

CHIP_ERROR BufferedReadCallback::GenerateListTLV(TLV::ScopedBufferTLVReader & aReader)
{
    TLV::TLVType outerType;
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;

    //
    // To generate the final reconstituted list, we need to allocate a contiguous
    // buffer than can hold the entirety of its contents. To do so, we need to figure out
    // how big a buffer to allocate. This requires walking the buffered list items and computing their TLV sizes,
    // summing them all up and adding a bit of slop to account for the TLV array the list elements will go into.
    //
    // The alternative was to use a PacketBufferTLVWriter backed by chained packet buffers to
    // write out the list - this would have removed the need for this first pass. However,
    // we cannot actually back a TLVReader with a chained buffer since that violates the ability
    // for us to create readers off-of readers. Each reader would assume exclusive ownership of the chained
    // buffer and mutate the state within TLVPacketBufferBackingStore, preventing shared use.
    //
    // To avoid that, a single contiguous buffer is the best likely approach for now.
    //
    uint32_t totalBufSize = 0;
    for (const auto & packetBuffer : mBufferedList)
    {
        totalBufSize += packetBuffer->TotalLength();
    }

    //
    // Size of the start container and end container are just 1 byte each, but, let's just be safe.
    //
    totalBufSize += 4;

    backingBuffer.Calloc(totalBufSize);
    VerifyOrReturnError(backingBuffer.Get() != nullptr, CHIP_ERROR_NO_MEMORY);

    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), totalBufSize);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

    for (auto & bufHandle : mBufferedList)
    {
        System::PacketBufferTLVReader reader;

        reader.Init(std::move(bufHandle));

        ReturnErrorOnFailure(reader.Next());
        ReturnErrorOnFailure(writer.CopyElement(TLV::AnonymousTag(), reader));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    writer.Finalize(backingBuffer);

    aReader.Init(std::move(backingBuffer), totalBufSize);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BufferedReadCallback::BufferListItem(TLV::TLVReader & reader)
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle handle;

    //
    // We conservatively allocate a packet buffer as big as an IPv6 MTU (since we're buffering
    // data received over the wire, which should always fit within that).
    //
    // We could have snapshotted the reader at its current position, advanced it past the current element
    // and computed the delta in its read point to figure out the size of the element before allocating
    // our target buffer. However, the reader's current position is already set past the control octet
    // and the tag. Consequently, the computed size is always going to omit the sizes of these two parts of the
    // TLV element. Since the tag can vary in size, for now, let's just do the safe thing. In the future, if this is a problem,
    // we can improve this.
    //
    handle = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    VerifyOrReturnError(!handle.IsNull(), CHIP_ERROR_NO_MEMORY);

    writer.Init(std::move(handle), false);

    ReturnErrorOnFailure(writer.CopyElement(TLV::AnonymousTag(), reader));
    ReturnErrorOnFailure(writer.Finalize(&handle));

    // Compact the buffer down to a more reasonably sized packet buffer
    // if we can.
    //
    handle.RightSize();

    mBufferedList.push_back(std::move(handle));

    return CHIP_NO_ERROR;
}

CHIP_ERROR BufferedReadCallback::BufferData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData)
{

    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
    {
        TLV::TLVType outerContainer;

        VerifyOrReturnError(apData->GetType() == TLV::kTLVType_Array, CHIP_ERROR_INVALID_TLV_ELEMENT);
        mBufferedList.clear();

        ReturnErrorOnFailure(apData->EnterContainer(outerContainer));

        CHIP_ERROR err;

        while ((err = apData->Next()) == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(BufferListItem(*apData));
        }

        if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(err);
        ReturnErrorOnFailure(apData->ExitContainer(outerContainer));
    }
    else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        ReturnErrorOnFailure(BufferListItem(*apData));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BufferedReadCallback::DispatchBufferedData(const ConcreteAttributePath & aPath, const StatusIB & aStatusIB,
                                                      bool aEndOfReport)
{
    if (aPath == mBufferedPath)
    {
        //
        // If we encountered the same list again and it's not the last DataIB, then
        // we need to continue to buffer up this list's data, so return immediately without dispatching
        // the existing buffered up contents.
        //
        if (!aEndOfReport)
        {
            return CHIP_NO_ERROR;
        }

        //
        // If we had previously buffered up data for this list and now we have encountered
        // an error for this list, that error takes precedence and the buffered data is now
        // rendered invalid. Return immediately without dispatching the existing buffered up contents.
        //
        if (aStatusIB.mStatus != Protocols::InteractionModel::Status::Success)
        {
            return CHIP_NO_ERROR;
        }
    }

    if (!mBufferedPath.IsListOperation())
    {
        return CHIP_NO_ERROR;
    }

    StatusIB statusIB;
    TLV::ScopedBufferTLVReader reader;

    ReturnErrorOnFailure(GenerateListTLV(reader));

    //
    // Update the list operation to now reflect the delivery of the entire list
    // i.e a replace all operation.
    //
    mBufferedPath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;

    //
    // Advance the reader forward to the list itself
    //
    ReturnErrorOnFailure(reader.Next());

    mCallback.OnAttributeData(mBufferedPath, &reader, statusIB);

    //
    // Clear out our buffered contents to free up allocated buffers, and reset the buffered path.
    //
    mBufferedList.clear();
    mBufferedPath = ConcreteDataAttributePath();
    return CHIP_NO_ERROR;
}

void BufferedReadCallback::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                           const StatusIB & aStatus)
{
    CHIP_ERROR err;

    //
    // First, let's dispatch to our registered callback any buffered up list data from previous calls.
    //
    err = DispatchBufferedData(aPath, aStatus);
    SuccessOrExit(err);

    //
    // We buffer up list data (only if the status was successful)
    //
    if (aPath.IsListOperation() && aStatus.mStatus == Protocols::InteractionModel::Status::Success)
    {
        err = BufferData(aPath, apData);
        SuccessOrExit(err);
    }
    else
    {
        mCallback.OnAttributeData(aPath, apData, aStatus);
    }

    //
    // Update our latched buffered path.
    //
    mBufferedPath = aPath;

exit:
    if (err != CHIP_NO_ERROR)
    {
        mCallback.OnError(err);
    }
}

} // namespace app
} // namespace chip
