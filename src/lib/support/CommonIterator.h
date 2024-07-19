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

/**
 *    @file
 *          Contains a standard iterator class.
 */

#pragma once

#include <stddef.h>

namespace chip {

/**
 * Template used to generate a custom iterator
 */
template <typename T>
class CommonIterator
{
public:
    virtual ~CommonIterator() = default;
    /**
     *  @retval The number of entries in total that will be iterated.
     */
    virtual size_t Count() = 0;
    /**
     *   @param[out] item  Value associated with the next element in the iteration.
     *  @retval true if the next entry is successfully retrieved.
     *  @retval false if no more entries can be found.
     */
    virtual bool Next(T & item) = 0;
    /**
     * Release the memory allocated by this iterator.
     * Must be called before the iterator goes out of scope if the iterator was dynamically allocated.
     */
    virtual void Release() = 0;

protected:
    CommonIterator() = default;
};

} // namespace chip
