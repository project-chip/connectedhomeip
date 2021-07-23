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

#include <array>
#include <core/CHIPConfig.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <support/BitFlags.h>
#include <support/CodeUtils.h>

namespace chip {
namespace app {

/*
 * @brief
 *
 * Defines a standardized interface for encoding and decoding schema data to/from TLV.
 *
 * This interface is supported by the various interaction objects in the IM.
 */
class IEncodableElement
{
public:
    virtual CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) = 0;
    virtual CHIP_ERROR Decode(TLV::TLVReader & reader)               = 0;
    virtual ~IEncodableElement() {}
};

} // namespace app
} // namespace chip
