/**
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

/**
 * @file
 * @brief Declaration of PICS Boolean Reader, a class that read and parse
 *        a file with PICS Code and their enabled/disabled state.
 */

#include <string>
#include <typeindex>
#include <unordered_map>

#include <lib/core/CHIPError.h>
#include <lib/support/Variant.h>

class PixitTypeTransformer
{
public:
    using PixitValueHolder =
        chip::Variant<std::string, uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t, bool, size_t>;
    static PixitValueHolder TransformToType(const std::string & key, const std::string & valueAsString);
    static bool Exists(const char * key);
    static bool Exists(std::string str) { return Exists(str.c_str()); }

private:
    enum class PixitType
    {
        kString,
        kUint8,
        kUint16,
        kUint32,
        kUint64,
        kInt8,
        kInt16,
        kInt32,
        kInt64,
        kBool,
        kSizet,
    };
    static bool IsUnsigned(PixitType type);
    static bool IsSigned(PixitType type);
    static std::string GetName(PixitType type);
    static void PrintOutOfRangeError(const std::string & key, const std::string & valueAsString, PixitType type);

    template <typename T, typename ExtractedType>
    static CHIP_ERROR SetPixitValueHolder(ExtractedType extracted, PixitValueHolder & val)
    {
        if (extracted > std::numeric_limits<T>::max() || extracted < std::numeric_limits<T>::min())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        val.Set<T>(static_cast<T>(extracted));
        return CHIP_NO_ERROR;
    }

    static const std::unordered_map<std::string, PixitType> mPixitType;
};
