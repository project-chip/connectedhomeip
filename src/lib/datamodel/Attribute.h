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

#ifndef CHIP_ATTRIBUTE_H_
#define CHIP_ATTRIBUTE_H_

#include <datamodel/Value.h>

namespace chip {
namespace DataModel {

class Attribute
{
public:
    uint16_t mAttrId;
    Value mValue;
    Value mMin;
    Value mMax;

    Attribute(uint16_t attrId, ValueTypes type) : mAttrId(attrId), mValue(type), mMin(type), mMax(type) {}
    Attribute(uint16_t attrId, Value value) : mAttrId(attrId), mValue(value), mMin(value.mType), mMax(value.mType) {}
    Attribute(uint16_t attrId, ValueTypes type, uint64_t min, uint64_t max) :
        mAttrId(attrId), mValue(type), mMin(type, min), mMax(type, max)
    {}

    /* An attribute is typically set in either of the following 2 ways:
     *
     * - From the top of the stack or the most derived class, like
     *   brightness.Set(45). This is handled through the derived
     *   class's Set method.
     *
     * - From the bottom of the stack when the stack receives a schema
     *   with just a list of values. This will typically get called
     *   with a Set() call on the base-ptr. In this case the argument
     *   will be Value.
     */
    int Set(const Value & newValue)
    {
        /* We have to check the element type match in this case */
        if (mValue.mType != newValue.mType)
        {
            return FAIL;
        }
        if (withinRange(newValue))
        {
            mValue = newValue;
            return SUCCESS;
        }
        return FAIL;
    }

    /* Need to define the behaviour when Value contains pointers
     * to allocated data
     */
    Value Get() { return mValue; }

protected:
    bool withinRange(const uint64_t & value) { return (value >= mMin.Int64) && (value <= mMax.Int64); }

    bool withinRange(const Value value)
    {
        switch (mValue.mType)
        {
        case kCHIPValueType_Int8:
        case kCHIPValueType_Int16:
        case kCHIPValueType_Int32:
        case kCHIPValueType_Int64:
        case kCHIPValueType_UInt8:
        case kCHIPValueType_UInt16:
        case kCHIPValueType_UInt32:
        case kCHIPValueType_UInt64:
            return withinRange(value.Int64);
            break;
        case kCHIPValueType_Bool:
            return true;
        default:
            return false;
        }
        return false;
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIP_ATTRIBUTE_H_ */
