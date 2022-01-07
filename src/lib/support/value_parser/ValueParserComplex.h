/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <charconv>
#include <functional>
#include <type_traits>
#include <vector>

#include <app/data-model/List.h>
#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <lib/support/value_parser/ValueParserIntegral.h>

namespace chip {

template <typename T>
CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::Optional<T> & value,
                      std::vector<std::function<void(void)>> & freeFunctions);
template <typename T>
CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::app::DataModel::Nullable<T> & value,
                      std::vector<std::function<void(void)>> & freeFunctions);
template <typename ListEntryType>
CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::app::DataModel::List<ListEntryType> & value,
                      std::vector<std::function<void(void)>> & freeFunctions);

template <typename T>
CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::Optional<T> & value,
                      std::vector<std::function<void(void)>> & freeFunctions)
{
    iter = SkipSpaces(iter);
    if (IsTokenEnd(*iter))
    {
        value = chip::Optional<T>();
        return CHIP_NO_ERROR;
    }
    using StorageType = std::remove_cv_t<std::remove_reference_t<T>>;
    StorageType entry;
    CHIP_ERROR error = ParseValue(iter, end, entry, freeFunctions);
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }
    value.SetValue(entry);
    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::app::DataModel::Nullable<T> & value,
                      std::vector<std::function<void(void)>> & freeFunctions)
{
    iter = SkipSpaces(iter);
    if (strncmp("null", iter, 4) == 0)
    {
        iter += 4;
        value = chip::app::DataModel::Nullable<T>();
        return CHIP_NO_ERROR;
    }
    using StorageType = std::remove_cv_t<std::remove_reference_t<T>>;
    StorageType entry;
    CHIP_ERROR error = ParseValue(iter, end, entry, freeFunctions);
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }
    value.SetNonNull(entry);
    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::app::DataModel::List<T> & value,
                      std::vector<std::function<void(void)>> & freeFunctions)
{
    using StorageType = std::remove_cv_t<std::remove_reference_t<T>>;
    if (end - iter < 2)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (*iter != '[')
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    iter++;
    std::vector<StorageType> entries;
    entries.clear();
    bool foundClose = false;
    while (iter < end)
    {
        if (*iter == ']')
        {
            iter++;
            foundClose = true;
            break;
        }
        StorageType entryValue;
        iter             = SkipSpaces(iter);
        CHIP_ERROR error = ParseValue(iter, end, entryValue, freeFunctions);
        iter             = SkipSpaces(iter);
        if (error != CHIP_NO_ERROR)
        {
            return error;
        }
        if (iter >= end || (*iter != ']' && *iter != ','))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        entries.push_back(entryValue);
        if (*iter == ']')
        {
            iter++;
            foundClose = true;
            break;
        }
        iter++;
    }
    if (!foundClose)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!entries.empty())
    {
        StorageType * storage = new StorageType[entries.size()];
        std::copy(entries.begin(), entries.end(), storage);
        value = chip::app::DataModel::List<T>(storage, entries.size());
        freeFunctions.push_back([storage]() { delete[] storage; });
    }
    else
    {
        value = chip::app::DataModel::List<T>();
    }
    return CHIP_NO_ERROR;
}

} // namespace chip
