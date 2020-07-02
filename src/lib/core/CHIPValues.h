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

template <enum CHIPValueTypes EnumType, typename ValueType>
class CHIPValue
{
public:
    CHIPValueTypes mType;
    ValueType mValue;
    ValueType mMin;
    ValueType mMax;
    
    CHIPValue() : mType(EnumType) {}
    CHIPValue(ValueType value) : mType(EnumType), mValue(value) {}
    CHIPValue(ValueType value, ValueType min, ValueType max) : mType(EnumType),
                                                               mValue(value),
                                                               mMin(min),
                                                               mMax(max) {}
};

typedef class CHIPValue<kCHIPValueType_UInt32, uint32_t> CHIPValue_UInt32_t;
typedef class CHIPValue<kCHIPValueType_Bool, bool> CHIPValue_Bool_t;
} // namespace DataModel
} // namespace chip

#endif /* CHIPVALUES_H_ */
