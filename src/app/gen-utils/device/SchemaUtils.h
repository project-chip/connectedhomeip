/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *
 */

#pragma once

#include "SchemaTypes.h"
#include <array>
#include <basic-types.h>
#include <core/CHIPConfig.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <support/BitFlags.h>
#include <support/CodeUtils.h>

namespace chip {
namespace app {

CHIP_ERROR EncodeSchemaElement(chip::Span<const CompactFieldDescriptor> pDescriptor, void * buf, uint64_t tag,
                               TLV::TLVWriter & writer, bool inArray = false);
CHIP_ERROR DecodeSchemaElement(chip::Span<const CompactFieldDescriptor> pDescriptor, void * buf, TLV::TLVReader & reader,
                               bool inArray = false);

template <typename GenType_t>
CHIP_ERROR EncodeSchemaElement(GenType_t & v, TLV::TLVWriter & writer, uint64_t tag)
{
    CHIP_ERROR err = EncodeSchemaElement({ v.mDescriptor.FieldList.data(), v.mDescriptor.FieldList.size() }, &v, tag, writer);
    SuccessOrExit(err);

    err = writer.Finalize();

exit:
    return err;
}

template <typename GenType_t>
CHIP_ERROR DecodeSchemaElement(GenType_t & v, TLV::TLVReader & reader)
{
    CHIP_ERROR err = DecodeSchemaElement({ v.mDescriptor.FieldList.data(), v.mDescriptor.FieldList.size() }, &v, reader);
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace app
} // namespace chip
