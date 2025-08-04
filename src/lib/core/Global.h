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

#include <mutex>
#include <new>

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

namespace chip {
namespace detail {

#if CHIP_CONFIG_GLOBALS_LAZY_INIT

struct NonAtomicOnce
{
    bool mInitialized = false;
    void call(void (*func)(void *), void * context)
    {
        if (!mInitialized)
        {
            mInitialized = true;
            func(context);
        }
    }
};

struct AtomicOnce
{
// dispatch_once (if available) is more efficient than std::call_once because
// it takes advantage of the additional assumption that the dispatch_once_t
// is allocated within a static / global.
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_once_t mOnce = 0;
    void call(void (*func)(void *), void * context) { dispatch_once_f(&mOnce, context, func); }
#else // CHIP_SYSTEM_CONFIG_USE_DISPATCH
    std::once_flag mOnce;
    void call(void (*func)(void *), void * context) { std::call_once(mOnce, func, context); }
#endif
};

#endif // CHIP_CONFIG_GLOBALS_LAZY_INIT

} // namespace detail

/**
 * A wrapper for global object that enables initialization and destruction to
 * be configured by the platform via `CHIP_CONFIG_GLOBALS_*` options.
 *
 * The contained object of type T is default constructed, possibly lazily.
 *
 * Values of this type MUST be globals or static class members.
 *
 * This class is not thread-safe; external synchronization is required.
 * @see AtomicGlobal<T> for a thread-safe variant.
 */
#if CHIP_CONFIG_GLOBALS_LAZY_INIT
template <class T, class OnceStrategy = detail::NonAtomicOnce>
#else  // CHIP_CONFIG_GLOBALS_LAZY_INIT
template <class T>
#endif // CHIP_CONFIG_GLOBALS_LAZY_INIT
class Global
{
public:
    /// Returns the global object, initializing it if necessary.
    /// NOT thread-safe, external synchronization is required.
    T & get() { return _get(); }
    T * operator->() { return &_get(); }

    // Globals are not copyable or movable
    Global(const Global &)             = delete;
    Global(const Global &&)            = delete;
    Global & operator=(const Global &) = delete;

#if CHIP_CONFIG_GLOBALS_LAZY_INIT
public:
    constexpr Global() = default;
    ~Global()          = default;

private:
    // Zero-initialize everything. We should technically leave mStorage uninitialized,
    // but that can sometimes cause clang to be unable to constant-initialize the object.
    alignas(T) unsigned char mStorage[sizeof(T)] = {};
    OnceStrategy mOnce;

    T & _get()
    {
        T * value = reinterpret_cast<T *>(mStorage);
        mOnce.call(&create, value);
        return *value;
    }
    static void create(void * value)
    {
        new (value) T();
#if !CHIP_CONFIG_GLOBALS_NO_DESTRUCT
        CHIP_CXA_ATEXIT(&destroy, value);
#endif // CHIP_CONFIG_GLOBALS_NO_DESTRUCT
    }
    static void destroy(void * value) { static_cast<T *>(value)->~T(); }

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

/**
 * A variant of Global<T> that is thread-safe.
 */
template <class T>
using AtomicGlobal =
#if CHIP_CONFIG_GLOBALS_LAZY_INIT
    Global<T, detail::AtomicOnce>;
#else  // CHIP_CONFIG_GLOBALS_LAZY_INIT
    Global<T>; // eager globals are already thread-safe
#endif // CHIP_CONFIG_GLOBALS_LAZY_INIT

} // namespace chip
