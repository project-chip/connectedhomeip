/* mbed Microcontroller Library
 * Copyright (c) 2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBED_NET_EVENT_FILE_HANDLE_H
#define MBED_NET_EVENT_FILE_HANDLE_H

#include "platform/FileHandle.h"
#include <atomic>
#include <limits>
#include <mstd_atomic>

namespace mbed {

/**
 * Mbed equivalent of eventfd.
 * It has the following limitations:
 * - Semaphore semantic is not supported
 * - It is non blocking
 * - Counter is 32bit wide
 */
struct EventFileHandle : public FileHandle
{
    using counter_type = uint64_t;

    /**
     * Initialize a new eventfd, the internal counter is set to 0.
     */
    EventFileHandle() = default;

    /**
     * Read the current buffer value.
     * @param buffer The destination of the read.
     * @param size Size of data to read, it must accommodate the size of
     * counter_type
     * @return On success, fill the buffer in input with the counter value and
     * return its size.
     * If the buffer in input is invalid, return -EINVAL.
     * If the internal counter is 0 then return -EAGAIN.
     */
    ssize_t read(void * buffer, size_t size) override
    {
        if (!buffer || size < sizeof(counter_type))
        {
            errno = EINVAL;
            return -1;
        }

        auto value = _counter.exchange(0);
        if (value == 0)
        {
            errno = EAGAIN;
            return -1;
        }
        else
        {
            memcpy(buffer, &value, sizeof(counter_type));
            return sizeof(counter_type);
        }
    }

    /**
     * Adds the value supplied to the counter.
     * @param buffer contains the value to add.
     * @param size Size of data to read, it must accommodate the size of
     * counter_type
     * @return On success, return the size of the counter.
     * It fails with -EINVAL if the buffer in input is invalid or if the value to
     * write is the maximum value of the counter (0xFFFFFFFF).
     * It fails with -EAGAIN if the value in input causes an overflow of the
     * derlying counter.
     */
    ssize_t write(const void * buffer, size_t size) override
    {
        if (!buffer || size < sizeof(counter_type))
        {
            errno = EINVAL;
            return -1;
        }

        auto limit = std::numeric_limits<counter_type>::max();
        counter_type input;
        memcpy(&input, buffer, sizeof(input));

        if (input == limit)
        {
            errno = EINVAL;
            return -1;
        }

        // Compare exchange is made in a loop as it spuriously fail and the
        // values must be reloaded
        while (true)
        {
            auto current = _counter.load();
            if (input > (limit - current))
            {
                errno = EAGAIN;
                return -1;
            }
            auto success = _counter.compare_exchange_weak(current, current + input);
            if (success)
            {
                if (_cb)
                {
                    _cb();
                }
                return sizeof(counter_type);
            }
        }
    }

    int close() override { return 0; }

    off_t seek(off_t offset, int whence = SEEK_SET) override { return -1; }

    off_t size() override { return -1; }

    int set_blocking(bool blocking) override
    {
        if (blocking)
        {
            return -EINVAL;
        }
        return 0;
    }

    bool is_blocking() const override { return false; }

    short poll(short events) const override
    {
        auto value = _counter.load();
        if (value)
        {
            return (POLLIN | POLLOUT) & events;
        }
        else
        {
            return POLLOUT & events;
        }
    }

    void sigio(Callback<void()> func) override
    {
        _cb = func;
        if (_cb && poll(POLLIN))
        {
            _cb();
        }
    }

private:
    Callback<void()> _cb                = nullptr;
    mstd::atomic<counter_type> _counter = { 0 };
};

} // namespace mbed

#endif // MBED_NET_EVENT_FILE_HANDLE_H
