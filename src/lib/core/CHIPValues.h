/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    @file
 *      This file contains definitions for working with CHIP values.
 *
 */

#ifndef CHIPVALUES_H_
#define CHIPVALUES_H_

namespace chip {
namespace DataModel {

/* XXX Remove later*/
#define SUCCESS 0
#define FAIL    -1

enum CHIPValueTypes
{
    kCHIPValueType_NotSpecified           = -1,
    kCHIPValueType_Int8,
    kCHIPValueType_Int16,
    kCHIPValueType_Int32,
    kCHIPValueType_Int64,
    kCHIPValueType_UInt8,
    kCHIPValueType_UInt16,
    kCHIPValueType_UInt32,
    kCHIPValueType_UInt64,
    kCHIPValueType_Bool,
    kCHIPValueType_FloatingPointNumber32,
    kCHIPValueType_FloatingPointNumber64,
    kCHIPValueType_UTF8String_1ByteLength,
    kCHIPValueType_UTF8String_2ByteLength,
    kCHIPValueType_UTF8String_4ByteLength,
    kCHIPValueType_UTF8String_8ByteLength,
    kCHIPValueType_ByteString_1ByteLength,
    kCHIPValueType_ByteString_2ByteLength,
    kCHIPValueType_ByteString_4ByteLength,
    kCHIPValueType_ByteString_8ByteLength,
    kCHIPValueType_Null,
    kCHIPValueType_EndOfContainer,
};

class CHIPValue
{
public:
    CHIPValueTypes mType;
    union {
        uint64_t Int64;
    };
    CHIPValue(CHIPValueTypes type) : mType(type) {}
    CHIPValue(uint32_t value) : mType(kCHIPValueType_UInt32), Int64((uint64_t)value) {}
    CHIPValue(bool value) : mType(kCHIPValueType_Bool), Int64((uint64_t)value) {}

    int Set(uint32_t &value)
    {
        if (mType != kCHIPValueType_UInt32)
            return FAIL;
        Int64 = (uint64_t)value;
        return SUCCESS;
    }

    int Set(bool &value)
    {
        if (mType != kCHIPValueType_Bool)
            return FAIL;
        Int64 = (uint64_t)value;
        return SUCCESS;
    }

    int Get(uint32_t &value)
    {
        if (mType != kCHIPValueType_UInt32)
            return FAIL;
        value = (uint32_t)Int64;
        return SUCCESS;
    }

    int Get(bool &value)
    {
        if (mType != kCHIPValueType_Bool)
            return FAIL;
        value = (bool)Int64;
        return SUCCESS;
    }

    bool operator <(const CHIPValue& value)
    {
        if (mType != value.mType)
        {
            return false;
        }
        switch (mType)
        {
        case kCHIPValueType_Int8:
        case kCHIPValueType_Int16:
        case kCHIPValueType_Int32:
        case kCHIPValueType_Int64:
        case kCHIPValueType_UInt8:
        case kCHIPValueType_UInt16:
        case kCHIPValueType_UInt32:
        case kCHIPValueType_UInt64:
        case kCHIPValueType_Bool:
            return Int64 < value.Int64;
            break;
        default:
            return false;
        }
        return false;
    }

    bool operator >(const CHIPValue& value)
    {
        if (mType != value.mType)
        {
            return false;
        }
        switch (mType)
        {
        case kCHIPValueType_Int8:
        case kCHIPValueType_Int16:
        case kCHIPValueType_Int32:
        case kCHIPValueType_Int64:
        case kCHIPValueType_UInt8:
        case kCHIPValueType_UInt16:
        case kCHIPValueType_UInt32:
        case kCHIPValueType_UInt64:
        case kCHIPValueType_Bool:
            return Int64 > value.Int64;
            break;
        default:
            return false;
        }
        return false;
    }

private:
    CHIPValue() {}

};

} // namespace DataModel
} // namespace chip

#endif /* CHIPVALUES_H_ */
