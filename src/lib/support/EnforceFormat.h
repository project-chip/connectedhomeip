/*
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 * gcc and clang provide a way to warn for a custom formatter when formats don't
 * match arguments.  Use that so we catch mistakes.  The "format"
 * attribute takes the type of format, which arg is the format string, and which
 * arg is the first variadic arg, with both arg numbers 1-based.
 *
 * The second arg should be set to 0 if the function takes a va_list instead of
 * varargs.
 */

#if defined(__GNUC__) || defined(__clang__)
#define ENFORCE_FORMAT(n, m) __attribute__((format(printf, n, m)))
#else                        // __GNUC__
#define ENFORCE_FORMAT(n, m) /* How to do with MSVC? */
#endif                       // __GNUC__
