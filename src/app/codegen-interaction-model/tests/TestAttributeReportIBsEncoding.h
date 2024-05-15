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
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>

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

} // namespace Test
} // namespace chip
