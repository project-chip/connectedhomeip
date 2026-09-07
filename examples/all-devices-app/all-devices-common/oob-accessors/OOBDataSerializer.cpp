/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBDataSerializer.h>

namespace chip::app::OOBDataSerializer {

std::variant<CHIP_ERROR, AttributeRequest> ParseAttributeRequest(ByteSpan tlvBuffer)
{
    AttributeRequest request;
    chip::TLV::TLVReader rootReader;
    rootReader.Init(tlvBuffer);

    ReturnErrorAndLogOnFailure(rootReader.Next(), Support, "Failed to read first TLV element");
    if (rootReader.GetType() != chip::TLV::kTLVType_Structure)
    {
        return CHIP_ERROR_WRONG_TLV_TYPE;
    }

    chip::TLV::TLVType outerType;
    ReturnErrorAndLogOnFailure(rootReader.EnterContainer(outerType), Support, "Failed to enter outer TLV structure container");

    // Tag 1: EndpointId
    ReturnErrorAndLogOnFailure(rootReader.Next(), Support, "Failed to advance TLV reader to EndpointId element");
    if (rootReader.GetTag() != chip::TLV::ContextTag(kTagEndpointId))
    {
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    }
    ReturnErrorAndLogOnFailure(rootReader.Get(request.path.mEndpointId), Support, "Failed to read EndpointId value");

    // Tag 2: ClusterId
    ReturnErrorAndLogOnFailure(rootReader.Next(), Support, "Failed to advance TLV reader to ClusterId element");
    if (rootReader.GetTag() != chip::TLV::ContextTag(kTagClusterId))
    {
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    }
    ReturnErrorAndLogOnFailure(rootReader.Get(request.path.mClusterId), Support, "Failed to read ClusterId value");

    // Tag 3: AttributeId
    ReturnErrorAndLogOnFailure(rootReader.Next(), Support, "Failed to advance TLV reader to AttributeId element");
    if (rootReader.GetTag() != chip::TLV::ContextTag(kTagAttributeId))
    {
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    }
    ReturnErrorAndLogOnFailure(rootReader.Get(request.path.mAttributeId), Support, "Failed to read AttributeId value");

    // Tag 4: Arguments (args)
    ReturnErrorAndLogOnFailure(rootReader.Next(), Support, "Failed to advance TLV reader to AttributeValue element");
    if (rootReader.GetTag() != chip::TLV::ContextTag(kTagValue))
    {
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    }
    request.value = rootReader;

    ReturnErrorAndLogOnFailure(rootReader.ExitContainer(outerType), Support, "Failed to exit outer TLV structure container");

    return request;
}

std::variant<CHIP_ERROR, ReadOnlyBuffer<uint8_t>> BuildSetAttributeRequest(const ConcreteDataAttributePath & path,
                                                                           const chip::TLV::TLVReader & attributeValueReader)
{
    // Create a copy of the reader and skip the element to calculate its EXACT byte size
    chip::TLV::TLVReader readerHelper;
    readerHelper.Init(attributeValueReader);
    ReturnErrorAndLogOnFailure(readerHelper.Skip(), Support, "Failed to skip element to calculate size");
    size_t elementSize = readerHelper.GetLengthRead() - attributeValueReader.GetLengthRead();

    size_t additionalBytesForPath = 256; // Enough for initial padding of EndpointId, clusterId and attributeId
    size_t totalBufferLen         = elementSize + additionalBytesForPath;

    Platform::ScopedMemoryBuffer<uint8_t> tlvRequest;
    if (!tlvRequest.Alloc(totalBufferLen))
    {
        ChipLogError(Support, "Failed to allocate memory for SetAttribute OOB request buffer");
        return CHIP_ERROR_NO_MEMORY;
    }

    chip::TLV::ScopedBufferTLVWriter tlvWriter(std::move(tlvRequest), totalBufferLen);
    chip::TLV::TLVType outerType;

    ReturnErrorAndLogOnFailure(tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerType),
                               Support, "Failed to start TLV structure container");

    // Tag 1: EndpointId
    ReturnErrorAndLogOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kTagEndpointId), path.mEndpointId), Support,
                               "Failed to write EndpointId tag");

    // Tag 2: ClusterId
    ReturnErrorAndLogOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kTagClusterId), path.mClusterId), Support,
                               "Failed to write ClusterId tag");

    // Tag 3: AttributeId
    ReturnErrorAndLogOnFailure(tlvWriter.Put(chip::TLV::ContextTag(kTagAttributeId), path.mAttributeId), Support,
                               "Failed to write AttributeId tag");

    // Tag 4: Attribute Value
    readerHelper.Init(attributeValueReader);
    ReturnErrorAndLogOnFailure(tlvWriter.CopyElement(chip::TLV::ContextTag(kTagValue), readerHelper), Support,
                               "Failed to copy attribute value TLV element");

    ReturnErrorAndLogOnFailure(tlvWriter.EndContainer(outerType), Support, "Failed to end TLV structure container");

    // Get TLV length and move ownership back to tlvRequest.
    size_t tlvLen = tlvWriter.GetLengthWritten();
    ReturnErrorAndLogOnFailure(tlvWriter.Finalize(tlvRequest), Support, "Failed to finalize TLV buffer creator");

    uint8_t * rawPtr = tlvRequest.Release();
    return ReadOnlyBuffer<uint8_t>(rawPtr, tlvLen, true /* allocated */);
}

} // namespace chip::app::OOBDataSerializer
