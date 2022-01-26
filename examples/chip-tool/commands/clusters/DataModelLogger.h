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

#include <string>

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/BytesToHex.h>

class DataModelLogger
{
public:
    static CHIP_ERROR LogValue(const char * label, size_t indent, bool value)
    {
        DataModelLogger::LogString(label, indent, value ? "TRUE" : "FALSE");
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR LogValue(const char * label, size_t indent, chip::CharSpan value)
    {
        DataModelLogger::LogString(label, indent, std::string(value.data(), value.size()));
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR LogValue(const char * label, size_t indent, chip::ByteSpan value)
    {
        char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
        if (CHIP_NO_ERROR ==
            chip::Encoding::BytesToUppercaseHexString(value.data(), value.size(), &buffer[0], CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE))
        {
            DataModelLogger::LogString(label, indent, buffer);
        }
        else
        {
            DataModelLogger::LogString(label, indent,
                                       std::string("Elided value too large of size ") + std::to_string(value.size()));
        }

        return CHIP_NO_ERROR;
    }

    template <typename X,
              typename std::enable_if_t<
                  std::is_integral<X>::value && !std::is_same<std::remove_cv_t<std::remove_reference_t<X>>, bool>::value, int> = 0>
    static CHIP_ERROR LogValue(const char * label, size_t indent, X value)
    {
        DataModelLogger::LogString(label, indent, std::to_string(value));
        return CHIP_NO_ERROR;
    }

    template <typename X, typename std::enable_if_t<std::is_floating_point<X>::value, int> = 0>
    static CHIP_ERROR LogValue(const char * label, size_t indent, X value)
    {
        DataModelLogger::LogString(label, indent, std::to_string(value));
        return CHIP_NO_ERROR;
    }

    template <typename X, typename std::enable_if_t<std::is_enum<X>::value, int> = 0>
    static CHIP_ERROR LogValue(const char * label, size_t indent, X value)
    {
        DataModelLogger::LogValue(label, indent, chip::to_underlying(value));
        return CHIP_NO_ERROR;
    }

    template <typename X>
    static CHIP_ERROR LogValue(const char * label, size_t indent, chip::BitFlags<X> value)
    {
        DataModelLogger::LogValue(label, indent, value.Raw());
        return CHIP_NO_ERROR;
    }

    template <typename T>
    static CHIP_ERROR LogValue(const char * label, size_t indent, const chip::app::DataModel::DecodableList<T> & value)
    {
        size_t count   = 0;
        CHIP_ERROR err = value.ComputeSize(&count);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        DataModelLogger::LogString(label, indent, std::to_string(count) + " entries");

        auto iter = value.begin();
        size_t i  = 0;
        while (iter.Next())
        {
            ++i;
            std::string itemLabel = std::string("[") + std::to_string(i) + "]";
            ReturnErrorOnFailure(DataModelLogger::LogValue(itemLabel.c_str(), indent + 1, iter.GetValue()));
        }
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            DataModelLogger::LogString(indent + 1, "List truncated due to invalid value");
        }
        return iter.GetStatus();
    }

    template <typename T>
    static CHIP_ERROR LogValue(const char * label, size_t indent, const chip::app::DataModel::Nullable<T> & value)
    {
        if (value.IsNull())
        {
            DataModelLogger::LogString(label, indent, "null");
        }
        else
        {
            DataModelLogger::LogValue(label, indent, value.Value());
        }

        return CHIP_NO_ERROR;
    }

    template <typename T>
    static CHIP_ERROR LogValue(const char * label, size_t indent, const chip::Optional<T> & value)
    {
        if (value.HasValue())
        {
            DataModelLogger::LogValue(label, indent, value.Value());
        }

        return CHIP_NO_ERROR;
    }

#include <zap-generated/cluster/logging/DataModelLogger.h>

private:
    static void LogString(size_t indent, const std::string string) { LogString("", indent, string); }

    static void LogString(const std::string label, size_t indent, const std::string string)
    {
        std::string indentation;
        for (size_t i = 0; i < indent; ++i)
        {
            indentation.append("  ");
        }

        ChipLogProgress(chipTool, "%s%s%s %s", indentation.c_str(), label.c_str(), label.size() ? ":" : "", string.c_str());
    }
};
