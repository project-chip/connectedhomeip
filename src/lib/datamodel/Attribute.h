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
 *      This file contains definitions for working with CHIP Attributes
 *
 */

#ifndef CHIP_ATTRIBUTE_H_
#define CHIP_ATTRIBUTE_H_

#include <datamodel/Deque.h>
#include <datamodel/Value.h>

#include <core/CHIPError.h>

namespace chip {
namespace DataModel {

typedef uint16_t AttributeId_t;
/**
 * @brief
 *   This class implements a single attribute.
 */
class Attribute
{
private:
    Deque<Attribute> mDeque;
    friend class Cluster;

public:
    AttributeId_t mAttrId;

    Attribute(AttributeId_t attrId) : mDeque(this), mAttrId(attrId) {}

    /**
     * @brief
     *   Set this attribute to a value
     *
     * @param value  the new value that this attribute should be updated with
     */
    virtual CHIP_ERROR Set(const Value & newValue) = 0;

    /* Need to define the behaviour when Value contains pointers
     * to allocated data
     */
    /**
     * @brief
     *   Get the value of this attribute
     *
     */
    virtual Value Get(void) = 0;
};

template <typename ValueType>
class AttributeSimple : public Attribute
{
private:
    ValueType mValue;

public:
    AttributeSimple(AttributeId_t attrId) : Attribute(attrId) {}
    AttributeSimple(AttributeId_t attrId, ValueType value) : Attribute(attrId), mValue(value) {}

    CHIP_ERROR Set(const Value & newValue) { return ValueToType(newValue, mValue); }

    Value Get(void) { return TypeToValue(mValue); }
};

template <typename ValueType, ValueType min, ValueType max>
class AttributeWithRange : public Attribute
{
private:
    ValueType mValue;
    const ValueType mMin = min;
    const ValueType mMax = max;

public:
    AttributeWithRange(AttributeId_t attrId) : Attribute(attrId) {}
    AttributeWithRange(AttributeId_t attrId, ValueType value) : Attribute(attrId), mValue(value) {}

    CHIP_ERROR Set(const Value & newValue)
    {
        ValueType tmp;
        if (ValueToType(newValue, tmp) == CHIP_NO_ERROR)
        {
            if ((tmp >= mMin) && (tmp <= mMax))
            {
                mValue = tmp;
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_INTERNAL;
    }

    Value Get(void) { return TypeToValue(mValue); }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIP_ATTRIBUTE_H_ */
