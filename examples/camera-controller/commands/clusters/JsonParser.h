/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include <commands/common/CustomStringPrefix.h>

#include <json/json.h>
#include <lib/core/Optional.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

class JsonParser
{
public:
    // Returns whether the parse succeeded.
    static bool ParseComplexArgument(const char * label, const char * json, Json::Value & value)
    {
        return Parse(label, json, /* strictRoot = */ true, value);
    }

    // Returns whether the parse succeeded.
    static bool ParseCustomArgument(const char * label, const char * json, Json::Value & value)
    {
        return Parse(label, json, /* strictRoot = */ false, value);
    }

private:
    static bool Parse(const char * label, const char * json, bool strictRoot, Json::Value & value)
    {
        Json::CharReaderBuilder readerBuilder;
        readerBuilder.settings_["strictRoot"]        = strictRoot;
        readerBuilder.settings_["allowSingleQuotes"] = true;
        readerBuilder.settings_["failIfExtra"]       = true;
        readerBuilder.settings_["rejectDupKeys"]     = true;

        auto reader = std::unique_ptr<Json::CharReader>(readerBuilder.newCharReader());
        std::string errors;
        if (reader->parse(json, json + strlen(json), &value, &errors))
        {
            return true;
        }

        // The CharReader API allows us to set failIfExtra, unlike Reader, but does
        // not allow us to get structured errors.  We get to try to manually undo
        // the work it did to create a string from the structured errors it had.
        ChipLogError(NotSpecified, "Error parsing JSON for %s:", label);

        // For each error "errors" has the following:
        //
        // 1) A line starting with "* " that has line/column info
        // 2) A line with the error message.
        // 3) An optional line with some extra info.
        //
        // We keep track of the last error column, in case the error message
        // reporting needs it.
        std::istringstream stream(errors);
        std::string error;
        chip::Optional<unsigned> errorColumn;
        while (getline(stream, error))
        {
            if (error.rfind("* ", 0) == 0)
            {
                // Flush out any pending error location.
                LogErrorLocation(errorColumn, json);

                // The format of this line is:
                //
                // * Line N, Column M
                //
                // Unfortunately it does not indicate end of error, so we can only
                // show its start.
                unsigned errorLine; // ignored in practice
                if (sscanf(error.c_str(), "* Line %u, Column %u", &errorLine, &errorColumn.Emplace()) != 2)
                {
                    ChipLogError(NotSpecified, "Unexpected location string: %s\n", error.c_str());
                    // We don't know how to make sense of this thing anymore.
                    break;
                }
                if (errorColumn.Value() == 0)
                {
                    ChipLogError(NotSpecified, "Expected error column to be at least 1");
                    // We don't know how to make sense of this thing anymore.
                    break;
                }
                // We are using our column numbers as offsets, so want them to be
                // 0-based.
                --errorColumn.Value();
            }
            else
            {
                ChipLogError(NotSpecified, "  %s", error.c_str());
                if (error == "  Missing ',' or '}' in object declaration" && errorColumn.HasValue() && errorColumn.Value() > 0 &&
                    json[errorColumn.Value() - 1] == '0' && (json[errorColumn.Value()] == 'x' || json[errorColumn.Value()] == 'X'))
                {
                    // Log the error location marker before showing the NOTE
                    // message.
                    LogErrorLocation(errorColumn, json);
                    ChipLogError(NotSpecified,
                                 "NOTE: JSON does not allow hex syntax beginning with 0x for numbers.  Try putting the hex number "
                                 "in quotes (like {\"name\": \"0x100\"}).");
                }
            }
        }

        // Write out the marker for our last error.
        LogErrorLocation(errorColumn, json);

        return false;
    }

private:
    static void LogErrorLocation(chip::Optional<unsigned> & errorColumn, const char * json)
    {
#if CHIP_ERROR_LOGGING
        if (!errorColumn.HasValue())
        {
            return;
        }

        const char * sourceText = json;
        unsigned error_start    = errorColumn.Value();
        // The whole JSON string might be too long to fit in our log
        // messages.  Just include 30 chars before the error.
        constexpr ptrdiff_t kMaxContext = 30;
        std::string errorMarker;
        if (error_start > kMaxContext)
        {
            sourceText += (error_start - kMaxContext);
            error_start = kMaxContext;
            ChipLogError(NotSpecified, "... %s", sourceText);
            // Add markers corresponding to the "... " above.
            errorMarker += "----";
        }
        else
        {
            ChipLogError(NotSpecified, "%s", sourceText);
        }
        for (unsigned i = 0; i < error_start; ++i)
        {
            errorMarker += "-";
        }
        errorMarker += "^";
        ChipLogError(NotSpecified, "%s", errorMarker.c_str());
        errorColumn.ClearValue();
#endif // CHIP_ERROR_LOGGING
    }
};
