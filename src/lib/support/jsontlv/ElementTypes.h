/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
