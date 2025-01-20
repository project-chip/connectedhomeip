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

#include "JsonSchemaMacros.h"

namespace {
constexpr const char * kJsonTypeNull    = "Null";
constexpr const char * kJsonTypeInt     = "Int";
constexpr const char * kJsonTypeUInt    = "UInt";
constexpr const char * kJsonTypeReal    = "Real";
constexpr const char * kJsonTypeString  = "String";
constexpr const char * kJsonTypeBool    = "Bool";
constexpr const char * kJsonTypeArray   = "Array";
constexpr const char * kJsonTypeObject  = "Object";
constexpr const char * kJsonTypeUnknown = "Unknown";
} // namespace

namespace chip {
namespace json {
const char * GetTypeName(const Json::Value & value)
{
    const char * type = kJsonTypeUnknown;

    switch (value.type())
    {
    case Json::nullValue:
        return kJsonTypeNull;
    case Json::intValue:
        return kJsonTypeInt;
    case Json::uintValue:
        return kJsonTypeUInt;
    case Json::realValue:
        return kJsonTypeReal;
    case Json::stringValue:
        return kJsonTypeString;
    case Json::booleanValue:
        return kJsonTypeBool;
    case Json::arrayValue:
        return kJsonTypeArray;
    case Json::objectValue:
        return kJsonTypeObject;
    default:
        return kJsonTypeUnknown;
    }

    return type;
}
} // namespace json
} // namespace chip
