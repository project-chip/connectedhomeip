/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
