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
 * Given a TLVReader positioned at a particular cluster data payload, this function converts
 * the TLV data into a JSON object representation.
 *
 * NOTE: This only accepts data model payloads for events/commands/attributes. It does not support
 * arbitrary TLV conversion to JSON.
 */
CHIP_ERROR TlvToJson(TLV::TLVReader & reader, std::string & jsonString);

/*
 * Given a TLV encoded byte array, this function converts it into JSON object.
 */
CHIP_ERROR TlvToJson(const ByteSpan & tlv, std::string & jsonString);

/*
 * Pretty-prints the input Json string using standard library pretty-printer.
 * This pretty-printer generates a Json string in a human friendly format with 3 space indentation
 * and nice representation of arrays and objects.
 * The input can be any string, as long as it's valid Json.
 */
std::string PrettyPrintJsonString(const std::string & jsonString);

/*
 * Reformats the input Json string as a single-line string with no spaces or newlines.
 * The input can be any string, as long as it's valid Json.
 */
std::string MakeJsonSingleLine(const std::string & jsonString);

/*
 *
 * The attribute/event TLV blob from the report during IM read/subscribe Interaction does not have a struct container
 * and attribute/event tag, but the tlvToJson function expects the struct container and tag Id for the TLV payload.
 * This convenient function firstly adds the struct container, if id is less than or equal to UINT8_MAX, it would encodes with the
 * context tag, if id is larger than UINT8_MAX and less than or equal to UINT32_MAX, it would encode with the implicit profile tag.
 * @param[in]   id              a attribute Id or event Id
 * @param[in]   tlv      a single tlv element from report attribute or event
 * @param[out]  jsonString             a mutable bytespan with data encode by attribute/event Id and struct container
 */
CHIP_ERROR SingleElementTlvToJson(uint32_t id, const TLV::TLVReader & tlv, std::string & jsonString);
} // namespace chip
