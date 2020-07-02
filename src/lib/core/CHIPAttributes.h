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

template <typename ValueType, typename BaseValueType>
class CHIPAttribute
{
public:
    ValueType mValue;
    CHIPAttribute() {}
    CHIPAttribute(BaseValueType value) : mValue(value) {}
    CHIPAttribute(BaseValueType value, BaseValueType min, BaseValueType max) : mValue(value, min, max) {}
};

typedef class CHIPAttribute<CHIPValue_UInt32_t, uint32_t> CHIPAttribute_UInt32_t;
typedef class CHIPAttribute<CHIPValue_Bool_t, bool> CHIPAttribute_Bool_t;

} // namespace DataModel
} // namespace chip

#endif /* CHIPATTRIBUTES_H_ */
