/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/TLV.h>

namespace {

const char kElementTypeInt[]    = "INT";
const char kElementTypeUInt[]   = "UINT";
const char kElementTypeBool[]   = "BOOL";
const char kElementTypeFloat[]  = "FLOAT";
const char kElementTypeDouble[] = "DOUBLE";
const char kElementTypeBytes[]  = "BYTES";
const char kElementTypeString[] = "STRING";
const char kElementTypeNull[]   = "NULL";
const char kElementTypeStruct[] = "STRUCT";
const char kElementTypeArray[]  = "ARRAY";
const char kElementTypeEmpty[]  = "?";

const char kFloatingPointPositiveInfinity[] = "Infinity";
const char kFloatingPointNegativeInfinity[] = "-Infinity";

struct ElementTypeContext
{
    chip::TLV::TLVType tlvType = chip::TLV::kTLVType_NotSpecified;
    bool isDouble              = false;
};

} // namespace
