/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
