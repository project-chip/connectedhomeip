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

} // namespace chip
