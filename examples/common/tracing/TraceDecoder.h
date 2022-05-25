/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include "TraceDecoderOptions.h"

#include <json/json.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace trace {

class TraceDecoder
{
public:
    CHIP_ERROR ReadFile(const char * fp);
    CHIP_ERROR ReadString(const char * str);

    void SetOptions(TraceDecoderOptions options) { mOptions = options; }

private:
    CHIP_ERROR Log(Json::Value & value);
    CHIP_ERROR LogAndConsumeProtocol(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeHeaderFlags(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeSecurityFlags(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeMessageFlags(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeExchangeFlags(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumePayload(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeEncryptedPayload(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeOthers(Json::Value & value);

private:
    TraceDecoderOptions mOptions;
    Json::Value mJsonBuffer;
};

} // namespace trace
} // namespace chip
