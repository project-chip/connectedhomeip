/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/AttributeDataIB.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVWriter.h>

#include <vector>

namespace chip {
namespace Test {

struct DecodedAttributeData
{
    chip::DataVersion dataVersion;
    chip::app::ConcreteDataAttributePath attributePath;
    chip::TLV::TLVReader dataReader;

    CHIP_ERROR DecodeFrom(const chip::app::AttributeDataIB::Parser & parser);
};

CHIP_ERROR DecodeAttributeReportIBs(ByteSpan data, std::vector<DecodedAttributeData> & decoded_items);

/// Maintains an internal TLV buffer for data encoding and
/// decoding for ReportIBs.
///
/// Main use case is that explicit TLV layouts (structure and container starting) need to be
/// prepared to have a proper AttributeReportIBs::Builder/parser to exist.
class EncodedReportIBs
{
public:
    /// Initialize the report structures required to encode a
    CHIP_ERROR StartEncoding(app::AttributeReportIBs::Builder & builder);
    CHIP_ERROR FinishEncoding(app::AttributeReportIBs::Builder & builder);

    /// Decode the embedded attribute report IBs.
    /// The TLVReaders inside data have a lifetime tied to the current object (its readers point
    /// inside the current object)
    CHIP_ERROR Decode(std::vector<DecodedAttributeData> & decoded_items);

private:
    uint8_t mTlvDataBuffer[1024];
    TLV::TLVType mOuterStructureType;
    TLV::TLVWriter mEncodeWriter;
    ByteSpan mDecodeSpan;
};

} // namespace Test
} // namespace chip
