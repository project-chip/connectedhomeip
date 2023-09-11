/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief
 *   Class declarations for a Counter base class, and a monotonically-increasing counter.
 */

#pragma once

#include <lib/core/CHIPError.h>

namespace chip {

/**
 * @class Counter
 *
 * @brief
 *   An interface for managing a counter as an integer value.
 */

template <typename T>
class Counter
{
public:
    Counter() {}
    virtual ~Counter() {}

    /**
     *  @brief
     *  Advance the value of the counter.
     *
     *  @return A CHIP error code if anything failed, CHIP_NO_ERROR otherwise.
     */
    virtual CHIP_ERROR Advance() = 0;

    /**
     *  @brief
     *  Get the current value of the counter.
     *
     *  @return The current value of the counter.
     */
    virtual T GetValue() = 0;
};

/**
 * @class MonotonicallyIncreasingCounter
 *
 * @brief
 *   A class for managing a monotonically-increasing counter as an integer value.
 */

template <typename T>
class MonotonicallyIncreasingCounter : public Counter<T>
{
public:
    MonotonicallyIncreasingCounter() : mCounterValue(0) {}
    ~MonotonicallyIncreasingCounter() override{};

    /**
     *  @brief
     *    Initialize a MonotonicallyIncreasingCounter object.
     *
     *  @param[in] aStartValue  The starting value of the counter.
     *
     *  @return A CHIP error code if something fails, CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR Init(T aStartValue)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        mCounterValue = aStartValue;

        return err;
    }

    /**
     *  @brief
     *  Advance the value of the counter.
     *
     *  @return A CHIP error code if something fails, CHIP_NO_ERROR otherwise
     */
    CHIP_ERROR Advance() override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        mCounterValue++;

        return err;
    }

    /**
     *  @brief
     *  Get the current value of the counter.
     *
     *  @return The current value of the counter.
     */
    T GetValue() override { return mCounterValue; }

protected:
    T mCounterValue;
};

} // namespace chip
