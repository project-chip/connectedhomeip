/**
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

#import <Matter/MTRDefines.h>

#if __has_attribute(objc_direct)
#define MTR_DIRECT __attribute__((objc_direct))
#else
#define MTR_DIRECT
#endif

#if __has_attribute(objc_direct_members)
#define MTR_DIRECT_MEMBERS __attribute__((objc_direct_members))
#else
#define MTR_DIRECT_MEMBERS
#endif

#ifdef DEBUG
#define MTR_TESTABLE MTR_EXPORT
#else
#define MTR_TESTABLE
#endif

// clang-format off
/// Creates a weak shadow copy of the variable `local`
#define mtr_weakify(local)                                      \
    __weak typeof(local) _mtr_weak_##local = local

/// Copies the weak shadow copy of `local` created by `mtr_weakify`
/// back into a strong variable of the same name.
#define mtr_strongify(local)                                    \
    _Pragma("clang diagnostic push")                            \
    _Pragma("clang diagnostic ignored \"-Wshadow\"")            \
    __strong typeof(local) _Nullable local = _mtr_weak_##local  \
    _Pragma("clang diagnostic pop")

/// Declares an unused local of unspecified type, to prevent accidental
/// references to a shadowed variable of the same name. Note that hiding
/// `self` does not prevent implicit references to self due to ivar access.
#define mtr_hide(local)                                         \
    _Pragma("clang diagnostic push")                            \
    _Pragma("clang diagnostic ignored \"-Wshadow\"")            \
    __attribute__((unused)) variable_hidden_by_mtr_hide local;  \
    _Pragma("clang diagnostic pop")
typedef struct {} variable_hidden_by_mtr_hide;
// clang-format on

// mtr_[un]likely(expr): Evaluates a boolean expression and hints to the compiler that it is [un]likely to be true.
#define mtr_likely(expr) __builtin_expect(!!(expr), 1)
#define mtr_unlikely(expr) __builtin_expect(!!(expr), 0)

// Default timed interaction timeout, in ms, if another one is not provided.
#define MTR_DEFAULT_TIMED_INTERACTION_TIMEOUT_MS 10000
