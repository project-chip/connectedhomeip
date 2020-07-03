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

#ifndef CHIPATTRIBUTES_H_
#define CHIPATTRIBUTES_H_

#include <lib/core/CHIPValues.h>

namespace chip {
namespace DataModel {


class CHIPAttribute
{
public:
    CHIPAttribute(uint16_t attrId, CHIPValueTypes type) : mAttrId(attrId), mValue(type), mMin(type), mMax(type) {}
    CHIPAttribute(uint16_t attrId, CHIPValue min, CHIPValue max) : mAttrId(attrId), mValue(min.mType), mMin(min), mMax(max) {}

    int Set(CHIPValue newValue)
    {
        if (mValue.mType != newValue.mType)
        {
            return FAIL;
        }
        if (newValue < mMin ||
            newValue > mMax)
        {
            return FAIL;
        }
        mValue = newValue;
        return SUCCESS;
    }

    CHIPValue Get()
    {
        return mValue;
    }

private:
    uint16_t         mAttrId;
    CHIPValue        mValue;
    CHIPValue        mMin;
    CHIPValue        mMax;
};


} // namespace DataModel
} // namespace chip

#endif /* CHIPATTRIBUTES_H_ */
