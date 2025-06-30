/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <json/json.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::CommodityTariffAttrsDataMgmt;

namespace JSON_Utilities {
template <typename StructType, typename DataClass, bool IsList = true>
class JsonParser
{
public:
    // Common field handlers
    static CHIP_ERROR ParseLabelFromJson(const Json::Value & value, DataModel::Nullable<chip::CharSpan> & outLabel)
    {
        // Check if the value exists and is a string
        if (value.isNull())
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }
        if (!value.isString())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        chip::CharSpan newLabel;

        CHIP_ERROR err = StrToSpan::Copy(value.asString(), newLabel, kDefaultStringValuesMaxBufLength - 1);

        if (err == CHIP_NO_ERROR)
        {
            outLabel.SetNonNull(newLabel);
        }
        else
        {
            outLabel.SetNull();
            // StrToSpan::Release(newLabel);
        }

        return err;
    }

    static CHIP_ERROR ParseIDArray(const Json::Value & value, DataModel::List<const uint32_t> & output, size_t maxSize)
    {
        if (value.empty() || !value.isArray() || value.size() > maxSize)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        std::vector<uint32_t> ids;
        ids.reserve(value.size());

        for (const auto & id : value)
        {
            if (id.isUInt())
                ids.push_back(id.asUInt());
        }

        bool is_success = SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(ids.data(), ids.size()), output, maxSize);
        ids.clear();

        if (!is_success)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        return CHIP_NO_ERROR;
    }

    // Common parsing interface
    static CHIP_ERROR ParseFromJson(const Json::Value & json, StructType & output)
    {
        static_assert(sizeof(StructType) == 0, "No ParseFromJson specialization provided for this type");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    // List-type loading
    template <bool T = IsList, typename = std::enable_if_t<T>>
    static CHIP_ERROR LoadFromJson(const Json::Value & json, DataClass & mgmtObj, size_t maxLength)
    {
        if (!json.isArray())
            return CHIP_ERROR_INVALID_ARGUMENT;
        if (json.size() > maxLength)
            return CHIP_ERROR_BUFFER_TOO_SMALL;

        std::vector<StructType> tempList;
        tempList.reserve(json.size());
        CHIP_ERROR err = CHIP_NO_ERROR;

        for (const auto & entryJson : json)
        {
            StructType entry;
            if ((err = ParseFromJson(entryJson, entry)) != CHIP_NO_ERROR)
                break;
            tempList.push_back(entry);
        }

        if ( (err == CHIP_NO_ERROR) && 
             ((err = mgmtObj.CreateNewValue(tempList.size())) == CHIP_NO_ERROR) )
        {
            std::copy(tempList.begin(), tempList.end(), mgmtObj.GetNewValueData());
            mgmtObj.MarkAsAssigned();
        }

        if (err != CHIP_NO_ERROR)
        {
            for (auto & entry : tempList)
                mgmtObj.CleanupExtEntry(entry);
        }

        return err;
    }

    // Single-object loading
    template <bool T = IsList, typename = std::enable_if_t<!T>>
    static CHIP_ERROR LoadFromJson(const Json::Value & json, DataClass & mgmtObj)
    {
        if (!json.isObject())
            return CHIP_ERROR_INVALID_ARGUMENT;

        StructType output;
        CHIP_ERROR err = ParseFromJson(json, output);
        if (err != CHIP_NO_ERROR)
            return err;

        if ((err = mgmtObj.CreateNewValue(0)) == CHIP_NO_ERROR)
        {
            *(mgmtObj.GetNewValueData()) = output;
            mgmtObj.MarkAsAssigned();
        }

        return err;
    }
};
} // namespace JSON_Utilities
