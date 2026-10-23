/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/core/TLVReader.h>

#include <cstdint>
#include <variant>

namespace chip {
namespace app {
namespace Clusters {
namespace detail {

// INTERNAL class used for structure decoding within individual
// `DecodableType::Decode` calls for TLV-encoded data.
//
// This class provides the ability to iterate over all context tags
// of a TLV-encoded structure. Example:
//
//    // reader is assumed to be positioned AT THE START of a kTLVType_Structure.
//    TLV::TLVReader reader;
//
//    detail::StructDecodeIterator iterator(reader);
//
//    while (true) {
//       uint8_t context_tag;
//       CHIP_ERROR err = iterator.Next(context_tag);
//
//       if (err == CHIP_ERROR_END_OF_TLV) {
//          break; // processing finished
//       }
//       ReturnErrorOnFailure(err); // CHIP_NO_ERROR means tag is valid
//
//       // process context_tag ... like using to_underlying(Fields::k....)
//    }
//
class StructDecodeIterator
{
public:
    StructDecodeIterator(TLV::TLVReader & reader) : mReader(reader) {}

    /// Iterate through structure elements.
    /// Reader MUST be positioned on a kTLVType_Structure element and iterator
    /// handles enter/exit container.
    ///
    /// Possible return values:
    ///
    /// - CHIP_NO_ERROR: a new tag was found. `context_tag` is valid
    /// - CHIP_ERROR_END_OF_TLV: no more tags. SUCCESSFULLY completed.
    /// - Any other error meaning failure
    ///
    /// During the first call, the iterator will EnterContainer into the reader.
    /// During the last call (CHIP_ERROR_END_OF_TLV), the iterator will ExitContainer.
    CHIP_ERROR Next(uint8_t & context_tag);

private:
    bool mEntered = false;
    TLV::TLVType mOuter;
    TLV::TLVReader & mReader;
};

} // namespace detail
} // namespace Clusters
} // namespace app
} // namespace chip
