/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include <lib/core/CHIPConfig.h>

#include <new>

namespace chip {

/**
 * A wrapper for global object that enables initialization and destruction to
 * be configured by the platform via `CHIP_CONFIG_GLOBALS_*` options.
 *
 * The contained object of type T is default constructed, possibly lazily.
 *
 * This class is generally NOT thread-safe; external synchronization is required.
 */
template <class T>
class Global
{
public:
    /// Returns the global object, initializing it if necessary.
    /// NOT thread-safe, external synchronization is required.
    T & get() { return _get(); }
    T * operator->() { return &_get(); }

#if CHIP_CONFIG_GLOBALS_LAZY_INIT
public:
    constexpr Global() = default;
    ~Global()          = default;

private:
    // Zero-initialize everything. We should technically leave mStorage uninitialized,
    // but that can sometimes cause clang to be unable to constant-initialize the object.
    alignas(T) unsigned char mStorage[sizeof(T)] = {};
    bool mInitialized                            = false;

    T & _value() { return *reinterpret_cast<T *>(mStorage); }

    T & _get()
    {
        if (!mInitialized)
        {
            new (mStorage) T();
            mInitialized = true;
#if !CHIP_CONFIG_GLOBALS_NO_DESTRUCT
            CHIP_CXA_ATEXIT(&destroy, this);
#endif // CHIP_CONFIG_GLOBALS_NO_DESTRUCT
        }
        return _value();
    }

    static void destroy(void * context) { static_cast<Global<T> *>(context)->_value().~T(); }

#else // CHIP_CONFIG_GLOBALS_LAZY_INIT
public:
    constexpr Global() : mValue() {}

private:
    T & _get() { return mValue; }

#if CHIP_CONFIG_GLOBALS_NO_DESTRUCT
public:
    // For not-trivially-destructible T, hiding it inside a union means the destructor
    // won't get called automatically, however our own default destructor will be implicitly
    // deleted. So Global<T> is technically not trivially-destructible, however the compiler
    // tends to be able to optimize away the empty destructor call in practice. Getting around
    // this cleanly requires using the "storage" approach used in the lazy variant, however
    // this would require std::construct_at from C++20 to get constexpr initialization.
    ~Global() {} // not "= default"

private:
    union
    {
        T mValue;
    };
#else  // CHIP_CONFIG_GLOBALS_NO_DESTRUCT
public:
    ~Global() = default;

private:
    T mValue;
#endif // CHIP_CONFIG_GLOBALS_NO_DESTRUCT
#endif // CHIP_CONFIG_GLOBALS_LAZY_INIT
};

} // namespace chip
