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
#include <lib/support/Span.h>

#pragma once

namespace chip {

const char * SkipSpaces(const char * iter);

bool IsTokenEnd(char ch);

template <typename X,
          typename std::enable_if_t<
              std::is_integral<X>::value && !std::is_same<std::remove_cv_t<std::remove_reference_t<X>>, bool>::value, int> = 0>
CHIP_ERROR ParseValue(const char *& iter, const char * end, X & value, std::vector<std::function<void(void)>> & freeFunctions);

template <typename X, typename std::enable_if_t<std::is_floating_point<X>::value, int> = 0>
CHIP_ERROR ParseValue(const char *& iter, const char * end, X & value, std::vector<std::function<void(void)>> & freeFunctions);

template <typename T>
CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::app::DataModel::Nullable<T> & value,
                      std::vector<std::function<void(void)>> & freeFunctions);

template <typename T>
CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::BitFlags<T> & value,
                      std::vector<std::function<void(void)>> & freeFunctions);

template <typename X, typename std::enable_if_t<std::is_enum<X>::value, int> = 0>
CHIP_ERROR ParseValue(const char *& iter, const char * end, X & value, std::vector<std::function<void(void)>> & freeFunctions);

CHIP_ERROR ParseValue(const char *& iter, const char * end, bool & value, std::vector<std::function<void(void)>> & freeFunctions);

CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::CharSpan & value,
                      std::vector<std::function<void(void)>> & freeFunctions);

CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::ByteSpan & value,
                      std::vector<std::function<void(void)>> & freeFunctions);

template <typename X, typename std::enable_if_t<std::is_floating_point<X>::value, int>>
CHIP_ERROR ParseValue(const char *& iter, const char * end, X & value, std::vector<std::function<void(void)>> & freeFunctions)
{
    char * nextIter;
    double converted = strtod(iter, &nextIter);
    if (nextIter == iter)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    value = static_cast<X>(converted);
    iter  = nextIter;
    return CHIP_NO_ERROR;
}

template <typename X,
          typename std::enable_if_t<
              std::is_integral<X>::value && !std::is_same<std::remove_cv_t<std::remove_reference_t<X>>, bool>::value, int>>
CHIP_ERROR ParseValue(const char *& iter, const char * end, X & value, std::vector<std::function<void(void)>> & freeFunctions)
{
    while (isspace(*iter))
    {
        iter++;
    }
    std::from_chars_result result = std::from_chars(iter, end, value);
    if (result.ec == std::errc())
    {
        iter += (result.ptr - iter);
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

template <typename X, typename std::enable_if_t<std::is_enum<X>::value, int>>
CHIP_ERROR ParseValue(const char *& iter, const char * end, X & value, std::vector<std::function<void(void)>> & freeFunctions)
{
    using StorageType    = std::remove_cv_t<std::remove_reference_t<X>>;
    using UnderlyingType = std::underlying_type_t<StorageType>;
    UnderlyingType data;
    CHIP_ERROR error = ParseValue(iter, end, data, freeFunctions);
    if (error == CHIP_NO_ERROR)
    {
        value = static_cast<StorageType>(data);
    }
    return error;
}

template <typename T>
CHIP_ERROR ParseValue(const char *& iter, const char * end, chip::BitFlags<T> & value,
                      std::vector<std::function<void(void)>> & freeFunctions)
{
    return ParseValue(iter, end, *value.RawStorage(), freeFunctions);
}

} // namespace chip
