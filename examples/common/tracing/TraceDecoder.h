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
#include "TraceHandlers.h"

#include <json/json.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace trace {

class TraceDecoder : public TraceStream
{
public:
    CHIP_ERROR ReadFile(const char * fp);
    CHIP_ERROR ReadString(const char * str);

    void SetOptions(TraceDecoderOptions options) { mOptions = options; }

    // TraceStream Interface
    void StartEvent(const std::string & label) override { ReadString(label.c_str()); }

    void AddField(const std::string & tag, const std::string & data) override
    {
        char buffer[4096] = {};
        snprintf(buffer, sizeof(buffer), "    %s\t %s", tag.c_str(), data.c_str());
        CHIP_ERROR err = ReadString(buffer);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Automation, "Failed to add field: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    void FinishEvent() override {}

private:
    CHIP_ERROR LogJSON(Json::Value & value);
    CHIP_ERROR LogAndConsumeProtocol(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeHeaderFlags(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeSecurityFlags(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeMessageFlags(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumeExchangeFlags(Json::Value & value);
    CHIP_ERROR MaybeLogAndConsumePayload(Json::Value & value, bool isResponse);
    CHIP_ERROR MaybeLogAndConsumeOthers(Json::Value & value);

private:
    TraceDecoderOptions mOptions;
};

} // namespace trace
} // namespace chip
