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
#include <string>

namespace chip {

/*
 * Given a JSON object that represents TLV, this function writes the corresponding TLV bytes into the provided buffer.
 * The size of tlv will be adjusted to the size of the actual data written to the buffer.
 */
CHIP_ERROR JsonToTlv(const std::string & jsonString, MutableByteSpan & tlv);

/*
 * Given a JSON object that represents TLV, this function makes encode calls on the given TLVWriter to encode the corresponding TLV
 * bytes.
 */
CHIP_ERROR JsonToTlv(const std::string & jsonString, TLV::TLVWriter & writer);

/*
 * Convert a uint64_t tagNumber to a TLV tag. When tagNumber is less than or equal to UINT8_MAX,
 * the tag is encoded using ContextTag. When tagNumber is larger than UINT8_MAX and less than or equal to UINT32_MAX,
 * the tag is encoded using an implicit profile tag.
 */
CHIP_ERROR ConvertTlvTag(const uint64_t tagNumber, TLV::Tag & tag);

} // namespace chip
