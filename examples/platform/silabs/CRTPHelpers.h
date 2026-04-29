/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
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

#include <type_traits>

/**
 * CRTP dispatch macros.
 *
 * Two categories:
 *   CRTP_OPTIONAL_* — Derived may override func. A base default exists; if Derived
 *                     does override, the signature is checked at compile time.
 *   CRTP_REQUIRED_* — Derived must provide func. No base default; a missing override
 *                     produces a compiler error.
 *
 * Suffix legend:
 *   _DISPATCH        instance method, returns a value, no extra args
 *   _DISPATCH_ARGS   instance method, returns a value, forwarded args
 *   _VOID_DISPATCH   instance method, void, forwarded args
 *   _STATIC_DISPATCH static method, void, forwarded args
 *   _CONST_DISPATCH_ARGS const instance method, returns a value, forwarded args
 */

/** Strip class type from a member function pointer, keeping return type, args, and const qualifier. */
template <typename>
struct mfp_sig;
template <typename C, typename Ret, typename... Args>
struct mfp_sig<Ret (C::*)(Args...)>
{
    using type = Ret(Args...);
};
template <typename C, typename Ret, typename... Args>
struct mfp_sig<Ret (C::*)(Args...) const>
{
    using type = Ret(Args...) const;
};

/**
 * Asserts that if Derived overrides func, its signature matches the base default.
 * Only for optional overrides (Base must have a default func).
 */
#define CRTP_CHECK_OPTIONAL_IMPL(Base, Derived, func)                                                                              \
    static_assert(std::is_same<decltype(&Derived::func), decltype(&Base::func)>::value ||                                          \
                      std::is_same<typename mfp_sig<decltype(&Derived::func)>::type,                                               \
                                   typename mfp_sig<decltype(&Base::func)>::type>::value,                                          \
                  #Derived "::" #func "() signature does not match " #Base "::" #func "()")

/** Cast this to Derived* (use in instance methods). */
#define CRTP_THIS(Derived) static_cast<Derived *>(this)

/** Get the app task singleton as Derived& (use in static methods). */
#define CRTP_APP_TASK(Derived) static_cast<Derived &>(Derived::GetAppTask())

// ---------------------------------------------------------------------------
// Optional dispatch — Derived may override func; base default exists.
// ---------------------------------------------------------------------------

/** Instance method, returns a value, no extra args. */
#define CRTP_OPTIONAL_DISPATCH(Base, Derived, func)                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        CRTP_CHECK_OPTIONAL_IMPL(Base, Derived, func);                                                                             \
        return static_cast<Derived *>(this)->func();                                                                               \
    } while (0)

/** Instance method, returns a value, forwarded args. */
#define CRTP_OPTIONAL_DISPATCH_ARGS(Base, Derived, func, ...)                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        CRTP_CHECK_OPTIONAL_IMPL(Base, Derived, func);                                                                             \
        return static_cast<Derived *>(this)->func(__VA_ARGS__);                                                                    \
    } while (0)

/** Const instance method, returns a value, forwarded args. */
#define CRTP_OPTIONAL_CONST_DISPATCH_ARGS(Base, Derived, func, ...)                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        CRTP_CHECK_OPTIONAL_IMPL(Base, Derived, func);                                                                             \
        return static_cast<const Derived *>(this)->func(__VA_ARGS__);                                                              \
    } while (0)

/** Instance method, void, forwarded args. */
#define CRTP_OPTIONAL_VOID_DISPATCH(Base, Derived, func, ...)                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        CRTP_CHECK_OPTIONAL_IMPL(Base, Derived, func);                                                                             \
        static_cast<Derived *>(this)->func(__VA_ARGS__);                                                                           \
    } while (0)

/** Static method, void, forwarded args. */
#define CRTP_OPTIONAL_STATIC_DISPATCH(Base, Derived, func, ...)                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        CRTP_CHECK_OPTIONAL_IMPL(Base, Derived, func);                                                                             \
        static_cast<Derived &>(Derived::GetAppTask()).func(__VA_ARGS__);                                                           \
    } while (0)

// ---------------------------------------------------------------------------
// Required dispatch — Derived must provide func; no base default.
// ---------------------------------------------------------------------------

/** Instance method, returns a value, no extra args. Missing func in Derived is a compiler error. */
#define CRTP_REQUIRED_DISPATCH(Base, Derived, func) return static_cast<Derived *>(this)->func()
