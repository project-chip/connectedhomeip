/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

#include <json/json.h>
#include <lib/core/TLV.h>

namespace chip {

/*
 * Given a TLVReader positioned at a particular cluster data payload, this function converts
 * the TLV data into a JSON object representation.
 *
 * NOTE: This only accepts data model payloads for events/commands/attributes. It does not support
 * arbitrary TLV conversion to JSON.
 */
CHIP_ERROR TlvToJson(TLV::TLVReader & reader, Json::Value & root);

/*
 * Converts a JSON object into string representation
 */
std::string JsonToString(Json::Value & json);

} // namespace chip
