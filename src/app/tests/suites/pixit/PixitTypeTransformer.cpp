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

#include "PixitTypeTransformer.h"
#include <errno.h>
#include <string>

namespace {
std::string Unquote(const std::string & str)
{
    size_t start = str.find_first_not_of("\"");
    size_t end   = str.find_last_not_of("\"");
    if (start == std::string::npos || end == std::string::npos || start > end)
    {
        return "";
    }
    return str.substr(start, end - start + 1);
}
} // namespace

void PixitTypeTransformer::PrintOutOfRangeError(const std::string & key, const std::string & valueAsString,
                                                PixitTypeTransformer::PixitType type)
{
    ChipLogError(chipTool, "Value out of range for %s - read %s, expected %s type", key.c_str(), valueAsString.c_str(),
                 GetName(type).c_str());
}

const std::unordered_map<std::string, PixitTypeTransformer::PixitType> PixitTypeTransformer::mPixitType = {
    { "PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID", PixitTypeTransformer::PixitType::kString },
    { "PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS", PixitTypeTransformer::PixitType::kString },
    { "PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID", PixitTypeTransformer::PixitType::kString },
    { "PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS", PixitTypeTransformer::PixitType::kString },
    { "PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET", PixitTypeTransformer::PixitType::kString },
    { "PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET", PixitTypeTransformer::PixitType::kString },
    { "PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET", PixitTypeTransformer::PixitType::kString },
    { "PIXIT.CNET.ENDPOINT_WIFI", PixitTypeTransformer::PixitType::kUint8 },
    { "PIXIT.CNET.ENDPOINT_THREAD", PixitTypeTransformer::PixitType::kUint8 },
    { "PIXIT.CNET.ENDPOINT_ETHERNET", PixitTypeTransformer::PixitType::kUint8 },

    // PIXITS used for testing
    { "PIXIT.UNITTEST.STRING.1", PixitTypeTransformer::PixitType::kString },
    { "PIXIT.UNITTEST.STRING.2", PixitTypeTransformer::PixitType::kString },
    { "PIXIT.UNITTEST.UINT8.1", PixitTypeTransformer::PixitType::kUint8 },
    { "PIXIT.UNITTEST.UINT8.2", PixitTypeTransformer::PixitType::kUint8 },
    { "PIXIT.UNITTEST.UINT16.1", PixitTypeTransformer::PixitType::kUint16 },
    { "PIXIT.UNITTEST.UINT16.2", PixitTypeTransformer::PixitType::kUint16 },
    { "PIXIT.UNITTEST.UINT32.1", PixitTypeTransformer::PixitType::kUint32 },
    { "PIXIT.UNITTEST.UINT32.2", PixitTypeTransformer::PixitType::kUint32 },
    { "PIXIT.UNITTEST.UINT64.1", PixitTypeTransformer::PixitType::kUint64 },
    { "PIXIT.UNITTEST.UINT64.2", PixitTypeTransformer::PixitType::kUint64 },
    { "PIXIT.UNITTEST.INT8.1", PixitTypeTransformer::PixitType::kInt8 },
    { "PIXIT.UNITTEST.INT8.2", PixitTypeTransformer::PixitType::kInt8 },
    { "PIXIT.UNITTEST.INT16.1", PixitTypeTransformer::PixitType::kInt16 },
    { "PIXIT.UNITTEST.INT16.2", PixitTypeTransformer::PixitType::kInt16 },
    { "PIXIT.UNITTEST.INT32.1", PixitTypeTransformer::PixitType::kInt32 },
    { "PIXIT.UNITTEST.INT32.2", PixitTypeTransformer::PixitType::kInt32 },
    { "PIXIT.UNITTEST.INT64.1", PixitTypeTransformer::PixitType::kInt64 },
    { "PIXIT.UNITTEST.INT64.2", PixitTypeTransformer::PixitType::kInt64 },
    { "PIXIT.UNITTEST.SIZET.1", PixitTypeTransformer::PixitType::kSizet },
    { "PIXIT.UNITTEST.SIZET.2", PixitTypeTransformer::PixitType::kSizet },
    { "PIXIT.UNITTEST.BOOL.T", PixitTypeTransformer::PixitType::kBool },
    { "PIXIT.UNITTEST.BOOL.F", PixitTypeTransformer::PixitType::kBool },
};

bool PixitTypeTransformer::IsSigned(PixitType type)
{
    switch (type)
    {
    case PixitType::kInt8:
    case PixitType::kInt16:
    case PixitType::kInt32:
    case PixitType::kInt64:
        return true;
    default:
        return false;
    }
}

bool PixitTypeTransformer::IsUnsigned(PixitType type)
{
    switch (type)
    {
    case PixitType::kUint8:
    case PixitType::kUint16:
    case PixitType::kUint32:
    case PixitType::kUint64:
    case PixitType::kSizet:
        return true;
    default:
        return false;
    }
}

std::string PixitTypeTransformer::GetName(PixitType type)
{
    switch (type)
    {
    case PixitType::kString:
        return "String";
    case PixitType::kInt8:
        return "int8_t";
    case PixitType::kInt16:
        return "int16_t";
    case PixitType::kInt32:
        return "int32_t";
    case PixitType::kInt64:
        return "int64_t";
    case PixitType::kUint8:
        return "uint8_t";
    case PixitType::kUint16:
        return "uint16_t";
    case PixitType::kUint32:
        return "uint32_t";
    case PixitType::kUint64:
        return "uint64_t";
    default:
        return "unknown";
    }
}

bool PixitTypeTransformer::Exists(const char * key)
{
    auto it = mPixitType.find(key);
    return it != mPixitType.end();
}

PixitTypeTransformer::PixitValueHolder PixitTypeTransformer::TransformToType(const std::string & key,
                                                                             const std::string & valueAsString)
{
    PixitTypeTransformer::PixitValueHolder ret;
    std::string upperKey = key;

    std::transform(upperKey.begin(), upperKey.end(), upperKey.begin(), [](unsigned char c) { return std::toupper(c); });
    auto it = mPixitType.find(upperKey);
    if (it == mPixitType.end())
    {
        ChipLogError(chipTool, "Unable to find %s in list of supported PIXIT values", key.c_str());
        return ret;
    }

    PixitType pixitType = it->second;
    unsigned long long unsignedNumber;
    long long signedNumber;
    char * end = nullptr;
    errno      = 0;

    if (IsUnsigned(pixitType))
    {

        unsignedNumber = strtoull(valueAsString.c_str(), &end, 0);
        // stroull does some very creative interpretations of what a negative in an unsigned number means. If we encounter a
        // negative number in the parsed string, let's just assume that it's not good and error.
        if (valueAsString.find('-') != std::string::npos)
        {
            errno = ERANGE;
        }
    }
    else if (IsSigned(pixitType))
    {
        {
            signedNumber = strtoll(valueAsString.c_str(), &end, 0);
        }
    }

    if (end == valueAsString.c_str())
    {
        ChipLogError(chipTool, "Unable to parse value for %s - read %s, expected %s type", key.c_str(), valueAsString.c_str(),
                     GetName(pixitType).c_str());
        return ret;
    }
    if (errno == ERANGE)
    {
        PrintOutOfRangeError(key, valueAsString, pixitType);
        return ret;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (pixitType)
    {
    case PixitType::kString:
        ret.Set<std::string>(Unquote(valueAsString));
        break;
    case PixitType::kUint8:
        err = SetPixitValueHolder<uint8_t>(unsignedNumber, ret);
        break;
    case PixitType::kUint16:
        err = SetPixitValueHolder<uint16_t>(unsignedNumber, ret);
        break;
    case PixitType::kUint32:
        err = SetPixitValueHolder<uint32_t>(unsignedNumber, ret);
        break;
    case PixitType::kUint64:
        err = SetPixitValueHolder<uint64_t>(unsignedNumber, ret);
        break;
    case PixitType::kInt8:
        err = SetPixitValueHolder<int8_t>(signedNumber, ret);
        break;
    case PixitType::kInt16:
        err = SetPixitValueHolder<int16_t>(signedNumber, ret);
        break;
    case PixitType::kInt32:
        err = SetPixitValueHolder<int32_t>(signedNumber, ret);
        break;
    case PixitType::kInt64:
        err = SetPixitValueHolder<int64_t>(signedNumber, ret);
        break;
    case PixitType::kSizet:
        err = SetPixitValueHolder<size_t>(unsignedNumber, ret);
        break;
    case PixitType::kBool: {
        std::string upperCaseVal = Unquote(valueAsString);
        std::transform(upperCaseVal.begin(), upperCaseVal.end(), upperCaseVal.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        ChipLogError(chipTool, "upper case val = %s", upperCaseVal.c_str());
        if (upperCaseVal == "0" || upperCaseVal == "FALSE")
        {
            ret.Set<bool>(false);
        }
        else if (upperCaseVal == "1" || upperCaseVal == "TRUE")
        {
            ret.Set<bool>(true);
        }
    }
    }
    if (err != CHIP_NO_ERROR)
    {
        PrintOutOfRangeError(key, valueAsString, pixitType);
    }
    return ret;
}
