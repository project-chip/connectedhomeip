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
} // namespace chip
