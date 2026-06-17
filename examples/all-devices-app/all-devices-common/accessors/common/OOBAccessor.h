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

#pragma once

#include <optional>

#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>

namespace chip::app {

class OOBAccessor : public chip::IntrusiveListNodeBase<chip::IntrusiveMode::AutoUnlink>
{
public:
    virtual ~OOBAccessor() = default;

    static constexpr CharSpan kActionSetAttribute = chip::literals::operator""_span("SetAttribute", 12);

    /**
     * @brief A utility class for parsing a unified TLV buffer containing an out-of-band "SetAttribute" request.
     *
     * The incoming TLV buffer is expected to be an anonymous structure containing four sequential elements
     * in the following predefined format:
     *
     * Structure
     * ├── Context Tag 1: EndpointId (uint16_t)
     * ├── Context Tag 2: ClusterId (uint32_t)
     * ├── Context Tag 3: AttributeId (uint32_t)
     * └── Context Tag 4: Raw, unwrapped Attribute Value.
     *
     * To use this parser:
     * @code
        OOBAccessor::SetAttributeRequestParser parser;
        ReturnErrorAndLogOnFailure(parser.Init(tlvBuffer), Support, "Failed to parse attribute path and value from tlvBuffer");

        if (parser.path.mEndpointId != mDevice->GetEndpointId())
        {
            return std::nullopt; // Not for our endpoint
        }

        Access::SubjectDescriptor subjectDescriptor{ .authMode = chip::Access::AuthMode::kInternalDeviceAccess };
        AttributeValueDecoder decoder(parser.attrValueReader, subjectDescriptor);

        return SetAttribute(parser.path, decoder);
     * @endcode
     */
    struct SetAttributeRequestParser
    {
        static constexpr uint8_t kTagEndpointId  = 1;
        static constexpr uint8_t kTagClusterId   = 2;
        static constexpr uint8_t kTagAttributeId = 3;
        static constexpr uint8_t kTagValue       = 4;

        ConcreteDataAttributePath path;
        chip::TLV::TLVReader attrValueReader;

        CHIP_ERROR Init(ByteSpan tlvBuffer)
        {
            chip::TLV::TLVReader rootReader;
            rootReader.Init(tlvBuffer);

            ReturnErrorOnFailure(rootReader.Next());
            if (rootReader.GetType() != chip::TLV::kTLVType_Structure)
            {
                return CHIP_ERROR_WRONG_TLV_TYPE;
            }

            chip::TLV::TLVType outerType;
            ReturnErrorOnFailure(rootReader.EnterContainer(outerType));

            // Tag 1: EndpointId
            ReturnErrorOnFailure(rootReader.Next());
            if (rootReader.GetTag() != chip::TLV::ContextTag(kTagEndpointId))
            {
                return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
            }
            ReturnErrorOnFailure(rootReader.Get(path.mEndpointId));

            // Tag 2: ClusterId
            ReturnErrorOnFailure(rootReader.Next());
            if (rootReader.GetTag() != chip::TLV::ContextTag(kTagClusterId))
            {
                return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
            }
            ReturnErrorOnFailure(rootReader.Get(path.mClusterId));

            // Tag 3: AttributeId
            ReturnErrorOnFailure(rootReader.Next());
            if (rootReader.GetTag() != chip::TLV::ContextTag(kTagAttributeId))
            {
                return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
            }
            ReturnErrorOnFailure(rootReader.Get(path.mAttributeId));

            // Tag 4: Arguments (args)
            ReturnErrorOnFailure(rootReader.Next());
            if (rootReader.GetTag() != chip::TLV::ContextTag(kTagValue))
            {
                return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
            }
            attrValueReader = rootReader;

            ReturnErrorOnFailure(rootReader.ExitContainer(outerType));

            return CHIP_NO_ERROR;
        }
    };

    /**
     * @brief Dynamically allocates and builds a unified, out-of-band "SetAttribute" request TLV buffer.
     *
     * This helper handles dynamic memory allocation based on the actual size of the attribute value element,
     * serializes the metadata, copies the value, and stores the completed package in the provided ScopedMemoryBuffer.
     *
     * The built request will have the following serialized TLV format:
     * Structure
     * ├── Context Tag 1: EndpointId (uint16_t)
     * ├── Context Tag 2: ClusterId (uint32_t)
     * ├── Context Tag 3: AttributeId (uint32_t)
     * └── Context Tag 4: Attribute Value (any valid TLV element/container representing the new value)
     *
     * @param[in]     path                  The concrete data attribute path containing Endpoint, Cluster, and Attribute IDs.
     * @param[in]     attributeValueReader  A TLVReader that MUST be positioned exactly on the data element of the attribute's value
     *                                      to be copied (e.g., a primitive element or a container header). This reader is not
     *                                      advanced or modified by the function.
     * @param[in,out] tlvLen                On input, this value is ignored. On successful return, it is updated with the exact
     *                                      total byte size of the fully serialized, finalized merged request.
     * @param[out]    tlvRequest            A ScopedMemoryBuffer that will take ownership of the newly allocated heap memory
     *                                      containing the completed request TLV buffer on success. It is cleared at the start
     *                                      and remains cleared upon failure.
     *
     * @return        #CHIP_NO_ERROR on successful buffer generation.
     *                #CHIP_ERROR_NO_MEMORY if the heap allocation fails.
     *                Other #CHIP_ERROR codes if serialization or element copying fails.
     */
    static CHIP_ERROR BuildSetAttributeRequest(const ConcreteDataAttributePath & path,
                                               const chip::TLV::TLVReader & attributeValueReader, size_t & tlvLen,
                                               Platform::ScopedMemoryBuffer<uint8_t> & tlvRequest)
    {
        tlvRequest.Free();

        // Create a copy of the reader and skip the element to calculate its EXACT byte size
        chip::TLV::TLVReader readerHelper;
        readerHelper.Init(attributeValueReader);
        ReturnErrorAndLogOnFailure(readerHelper.Skip(), Support, "Failed to skip element to calculate size");
        size_t elementSize = readerHelper.GetLengthRead() - attributeValueReader.GetLengthRead();

        size_t additionalBytesForPath = 256; // Enough for initial padding of EndpointId, clusterId and attributeId
        size_t totalBufferLen         = elementSize + additionalBytesForPath;

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
        ReturnErrorAndLogOnFailure(
            tlvWriter.Put(chip::TLV::ContextTag(SetAttributeRequestParser::kTagEndpointId), path.mEndpointId), Support,
            "Failed to write EndpointId tag");

        // Tag 2: ClusterId
        ReturnErrorAndLogOnFailure(tlvWriter.Put(chip::TLV::ContextTag(SetAttributeRequestParser::kTagClusterId), path.mClusterId),
                                   Support, "Failed to write ClusterId tag");

        // Tag 3: AttributeId
        ReturnErrorAndLogOnFailure(
            tlvWriter.Put(chip::TLV::ContextTag(SetAttributeRequestParser::kTagAttributeId), path.mAttributeId), Support,
            "Failed to write AttributeId tag");

        // Tag 4: Attribute Value
        readerHelper = attributeValueReader;
        ReturnErrorAndLogOnFailure(tlvWriter.CopyElement(chip::TLV::ContextTag(SetAttributeRequestParser::kTagValue), readerHelper),
                                   Support, "Failed to copy attribute value TLV element");

        ReturnErrorAndLogOnFailure(tlvWriter.EndContainer(outerType), Support, "Failed to end TLV structure container");

        ReturnErrorAndLogOnFailure(tlvWriter.Finalize(tlvRequest), Support, "Failed to finalize TLV buffer creator");

        tlvLen = tlvWriter.GetLengthWritten();

        return CHIP_NO_ERROR;
    }

    /**
     * @brief Handles a generic out-of-band action.
     *
     * @param actionName The name of the action to invoke.
     * @param tlvBuffer Buffer containing TLV data for the action request.
     * @return std::nullopt if the action is not handled/supported by this accessor.
     *         A non-null optional containing:
     *           - CHIP_NO_ERROR on success.
     *           - Other CHIP_ERROR codes on failure.
     */
    virtual std::optional<CHIP_ERROR> HandleAction(CharSpan actionName, ByteSpan tlvBuffer) = 0;
};

} // namespace chip::app
