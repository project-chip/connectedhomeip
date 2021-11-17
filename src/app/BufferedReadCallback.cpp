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

#include "lib/core/CHIPTLV.h"
#include "lib/core/CHIPTLVTags.h"
#include "lib/core/CHIPTLVTypes.h"
#include "protocols/interaction_model/Constants.h"
#include "system/SystemPacketBuffer.h"
#include "system/TLVPacketBufferBackingStore.h"
#include <app/BufferedReadCallback.h>
#include <app/InteractionModelEngine.h>

namespace chip {
namespace app {

void BufferedReadCallback::OnReportBegin()
{
    mCallback.OnReportBegin();
}

void BufferedReadCallback::OnReportEnd()
{
    DispatchBufferedData(nullptr, mBufferedPath, StatusIB(), true);
    mCallback.OnReportEnd();
}

CHIP_ERROR BufferedReadCallback::GenerateListTLV(System::PacketBufferTLVReader & aReader)
{
    System::PacketBufferHandle handle = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSizeWithoutReserve, 0);
    System::PacketBufferTLVWriter writer;
    TLV::TLVType outerType;

    //
    // Initialize the writer, but with chaining enabled, since we'll very likely
    // exceed the size of a single packet buffer when reconstituing the entire list
    //
    writer.Init(std::move(handle), /* useChainedBuffers */ true);

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Array, outerType));

    for (auto & bufHandle : mBufferedList)
    {
        System::PacketBufferTLVReader reader;

        reader.Init(std::move(bufHandle));

        ReturnErrorOnFailure(reader.Next());
        ReturnErrorOnFailure(writer.CopyElement(TLV::AnonymousTag, reader));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    writer.Finalize(&handle);
    aReader.Init(std::move(handle));

    return CHIP_NO_ERROR;
}

CHIP_ERROR BufferedReadCallback::BufferData(const ConcreteAttributePath & aPath, TLV::TLVReader * apData)
{
    CHIP_ERROR err;

    if (aPath.mListOp == ConcreteAttributePath::ListOperation::ReplaceAll)
    {
        TLV::TLVType outerContainer;

        VerifyOrReturnError(apData->GetType() == TLV::kTLVType_Array, CHIP_ERROR_INVALID_TLV_ELEMENT);
        mBufferedList.clear();

        ReturnErrorOnFailure(apData->EnterContainer(outerContainer));

        while ((err = apData->Next()) == CHIP_NO_ERROR)
        {
            System::PacketBufferTLVWriter writer;
            System::PacketBufferHandle handle = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);

            writer.Init(std::move(handle), false);

            ReturnErrorOnFailure(writer.CopyElement(TLV::AnonymousTag, *apData));
            ReturnErrorOnFailure(writer.Finalize(&handle));

            // Compact the buffer down to a more reasonably sized packet buffer
            // if we can.
            //
            handle.RightSize();

            mBufferedList.push_back(std::move(handle));
        }

        ReturnErrorOnFailure(apData->ExitContainer(outerContainer));
    }
    else if (aPath.mListOp == ConcreteAttributePath::ListOperation::AppendItem)
    {
        System::PacketBufferTLVWriter writer;
        System::PacketBufferHandle handle = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);

        writer.Init(std::move(handle), false);

        ReturnErrorOnFailure(writer.CopyElement(TLV::AnonymousTag, *apData));
        ReturnErrorOnFailure(writer.Finalize(&handle));

        //
        // Compact the buffer down to a more reasonably sized packet buffer
        // if we can.
        //
        handle.RightSize();

        mBufferedList.push_back(std::move(handle));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BufferedReadCallback::DispatchBufferedData(const ReadClient * apReadClient, const ConcreteAttributePath & aPath,
                                                      const StatusIB & aStatusIB, bool endOfReport)
{
    if (aPath == mBufferedPath)
    {
        //
        // If we encountered the same list again and it's not the last DataIB, then
        // we need to continue to buffer up this list's data, so return immediately without dispatching
        // the existing buffered up contents.
        //
        if (!endOfReport)
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
    System::PacketBufferTLVReader reader;

    ReturnErrorOnFailure(GenerateListTLV(reader));

    //
    // Update the list operation to now reflect the delivery of the entire list
    // i.e a replace all operation.
    //
    mBufferedPath.mListOp = ConcreteAttributePath::ListOperation::ReplaceAll;

    //
    // Advance the reader forward to the list itself
    //
    reader.Next();

    mCallback.OnAttributeData(apReadClient, mBufferedPath, &reader, statusIB);

    //
    // Clear out our buffered contents to free up allocated buffers
    //
    mBufferedList.clear();

    return CHIP_NO_ERROR;
}

void BufferedReadCallback::OnAttributeData(const ReadClient * apReadClient, const ConcreteAttributePath & aPath,
                                           TLV::TLVReader * apData, const StatusIB & aStatus)
{
    //
    // First, let's dispatch to our registered callback any buffered up list data from previous calls.
    //
    DispatchBufferedData(apReadClient, aPath, aStatus);

    //
    // We buffer up list data (only if the status was successful)
    //
    if (aPath.IsListOperation() && aStatus.mStatus == Protocols::InteractionModel::Status::Success)
    {
        ReturnOnFailure(BufferData(aPath, apData));
    }
    else
    {
        mCallback.OnAttributeData(apReadClient, aPath, apData, aStatus);
    }

    //
    // Update our latched buffered path.
    //
    mBufferedPath = aPath;
}

} // namespace app
} // namespace chip
