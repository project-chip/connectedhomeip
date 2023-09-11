/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
        char buffer[2048] = {};
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
