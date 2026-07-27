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
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventHeader.h>
#include <app/MessageDef/StatusIB.h>
#include <app/data-model/DecodableList.h>
#include <commands/common/RemoteDataModelLogger.h>
#include <lib/support/BytesToHex.h>
#include <zap-generated/cluster/logging/EntryToText.h>

class DataModelLogger
{
public:
    static CHIP_ERROR LogAttribute(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data);
    static CHIP_ERROR LogCommand(const chip::app::ConcreteCommandPath & path, chip::TLV::TLVReader * data);
    static CHIP_ERROR LogEvent(const chip::app::EventHeader & header, chip::TLV::TLVReader * data);

private:
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
        // CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE includes various prefixes we don't
        // control (timestamps, process ids, etc).  Let's assume (hope?) that
        // those prefixes use up no more than half the total available space.
        // Right now it looks like the prefixes are 45 chars out of a 255 char
        // buffer.
        char buffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE / 2];
        size_t prefixSize = ComputePrefixSize(label, indent);
        if (prefixSize > MATTER_ARRAY_SIZE(buffer))
        {
            DataModelLogger::LogString("", 0, "Prefix is too long to fit in buffer");
            return CHIP_ERROR_INTERNAL;
        }

        const size_t availableSize = MATTER_ARRAY_SIZE(buffer) - prefixSize;
        // Each byte ends up as two hex characters.
        const size_t bytesPerLogCall = availableSize / 2;
        std::string labelStr(label);
        while (value.size() > bytesPerLogCall)
        {
            ReturnErrorOnFailure(
                chip::Encoding::BytesToUppercaseHexString(value.data(), bytesPerLogCall, &buffer[0], MATTER_ARRAY_SIZE(buffer)));
            LogString(labelStr, indent, buffer);
            value = value.SubSpan(bytesPerLogCall);
            // For the second and following lines, make it clear that they are
            // continuation lines by replacing the label with "....".
            labelStr.replace(labelStr.begin(), labelStr.end(), labelStr.size(), '.');
        }
        ReturnErrorOnFailure(
            chip::Encoding::BytesToUppercaseHexString(value.data(), value.size(), &buffer[0], MATTER_ARRAY_SIZE(buffer)));
        LogString(labelStr, indent, buffer);

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
        return DataModelLogger::LogValue(label, indent, chip::to_underlying(value));
    }

    template <typename X>
    static CHIP_ERROR LogValue(const char * label, size_t indent, chip::BitFlags<X> value)
    {
        return DataModelLogger::LogValue(label, indent, value.Raw());
    }

    template <typename T>
    static CHIP_ERROR LogValue(const char * label, size_t indent, const chip::app::DataModel::DecodableList<T> & value)
    {
        size_t count = 0;
        ReturnErrorOnFailure(value.ComputeSize(&count));
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
            return CHIP_NO_ERROR;
        }

        return DataModelLogger::LogValue(label, indent, value.Value());
    }

    template <typename T>
    static CHIP_ERROR LogValue(const char * label, size_t indent, const chip::Optional<T> & value)
    {
        if (value.HasValue())
        {
            return DataModelLogger::LogValue(label, indent, value.Value());
        }

        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR LogClusterId(const char * label, size_t indent,
                                   const chip::app::DataModel::DecodableList<chip::ClusterId> & value)
    {
        size_t count = 0;
        ReturnErrorOnFailure(value.ComputeSize(&count));
        DataModelLogger::LogString(label, indent, std::to_string(count) + " entries");

        auto iter = value.begin();
        size_t i  = 0;
        while (iter.Next())
        {
            ++i;
            std::string index = std::string("[") + std::to_string(i) + "]";
            std::string item  = std::to_string(iter.GetValue()) + " (" + ClusterIdToText(iter.GetValue()) + ")";
            DataModelLogger::LogString(index, indent + 1, item);
        }
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            DataModelLogger::LogString(indent + 1, "List truncated due to invalid value");
        }
        return iter.GetStatus();
    }

    static CHIP_ERROR LogAttributeId(const char * label, size_t indent,
                                     const chip::app::DataModel::DecodableList<chip::AttributeId> & value, chip::ClusterId cluster)
    {
        size_t count = 0;
        ReturnErrorOnFailure(value.ComputeSize(&count));
        DataModelLogger::LogString(label, indent, std::to_string(count) + " entries");

        auto iter = value.begin();
        size_t i  = 0;
        while (iter.Next())
        {
            ++i;
            std::string index = std::string("[") + std::to_string(i) + "]";
            std::string item  = std::to_string(iter.GetValue()) + " (" + AttributeIdToText(cluster, iter.GetValue()) + ")";
            DataModelLogger::LogString(index, indent + 1, item);
        }
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            DataModelLogger::LogString(indent + 1, "List truncated due to invalid value");
        }
        return iter.GetStatus();
    }

    static CHIP_ERROR LogAcceptedCommandId(const char * label, size_t indent,
                                           const chip::app::DataModel::DecodableList<chip::CommandId> & value,
                                           chip::ClusterId cluster)
    {
        size_t count = 0;
        ReturnErrorOnFailure(value.ComputeSize(&count));
        DataModelLogger::LogString(label, indent, std::to_string(count) + " entries");

        auto iter = value.begin();
        size_t i  = 0;
        while (iter.Next())
        {
            ++i;
            std::string index = std::string("[") + std::to_string(i) + "]";
            std::string item  = std::to_string(iter.GetValue()) + " (" + AcceptedCommandIdToText(cluster, iter.GetValue()) + ")";
            DataModelLogger::LogString(index, indent + 1, item);
        }
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            DataModelLogger::LogString(indent + 1, "List truncated due to invalid value");
        }
        return iter.GetStatus();
    }

    static CHIP_ERROR LogGeneratedCommandId(const char * label, size_t indent,
                                            const chip::app::DataModel::DecodableList<chip::CommandId> & value,
                                            chip::ClusterId cluster)
    {
        size_t count = 0;
        ReturnErrorOnFailure(value.ComputeSize(&count));
        DataModelLogger::LogString(label, indent, std::to_string(count) + " entries");

        auto iter = value.begin();
        size_t i  = 0;
        while (iter.Next())
        {
            ++i;
            std::string index = std::string("[") + std::to_string(i) + "]";
            std::string item  = std::to_string(iter.GetValue()) + " (" + GeneratedCommandIdToText(cluster, iter.GetValue()) + ")";
            DataModelLogger::LogString(index, indent + 1, item);
        }
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            DataModelLogger::LogString(indent + 1, "List truncated due to invalid value");
        }
        return iter.GetStatus();
    }

#include <zap-generated/cluster/logging/DataModelLogger.h>

    static void LogString(size_t indent, const std::string string) { LogString("", indent, string); }

    static void LogString(const std::string label, size_t indent, const std::string string)
    {
        std::string prefix = ComputePrefix(label, indent);

        ChipLogProgress(chipTool, "%s%s", prefix.c_str(), string.c_str());
    }

private:
    static std::string ComputePrefix(const std::string label, size_t indent)
    {
        std::string prefix;
        for (size_t i = 0; i < indent; ++i)
        {
            prefix.append("  ");
        }
        if (label.size() > 0)
        {
            prefix.append(label);
            prefix.append(":");
        }
        prefix.append(" ");

        return prefix;
    }

    static size_t ComputePrefixSize(const std::string label, size_t indent) { return ComputePrefix(label, indent).size(); }
};
