/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include <json/json.h>

#define CHECK_TYPE(source, fieldName, fieldType)                                                                                   \
    VerifyOrReturnError(source.is##fieldType(), CHIP_ERROR_SCHEMA_MISMATCH,                                                        \
                        ChipLogError(chipTool, "Type mismatch for field '%s': expected '%s', got '%s'", #fieldName, #fieldType,    \
                                     chip::json::GetTypeName(source)));

#define CHECK_REQUIRED_TYPE(source, fieldName, fieldType)                                                                          \
    VerifyOrReturnError(source.isMember(#fieldName), CHIP_ERROR_SCHEMA_MISMATCH,                                                   \
                        ChipLogError(chipTool, "Missing required field: '%s'", #fieldName));                                       \
    CHECK_TYPE(source[#fieldName], fieldName, fieldType)

#define CHECK_REQUIRED_VALUE(source, fieldName, expectedValue)                                                                     \
    CHECK_REQUIRED_TYPE(source, fieldName, UInt);                                                                                  \
    VerifyOrReturnError(source[#fieldName].asUInt() == expectedValue, CHIP_ERROR_INCORRECT_STATE,                                  \
                        ChipLogError(chipTool, "Value mismatch for '%s': expected '%u', got '%u'", #fieldName, expectedValue,      \
                                     source[#fieldName].asUInt()));

namespace chip {
namespace json {
const char * GetTypeName(const Json::Value & value);
} // namespace json
} // namespace chip
