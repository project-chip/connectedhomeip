/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Contains a standard iterator class.
 */

#pragma once

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
     * Must be called before the iterator goes out of scope in the iterator was dynamically allocated.
     */
    virtual void Release() = 0;

protected:
    CommonIterator() = default;
};

} // namespace chip
