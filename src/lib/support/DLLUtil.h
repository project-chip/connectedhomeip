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
 *      This file defines a set of macros for declaring C and C++
 *      symbol visibility for dynamic shared objects (DSOs) across
 *      different compilers and operating systems.
 *
 *      See https://gcc.gnu.org/wiki/Visibility for additional information
 *      regarding GCC.
 *
 */

#pragma once

#if defined _WIN32 || defined __CYGWIN__
#define DLL_EXPORT __declspec(dllexport)
#else
#if __GNUC__ >= 4
#define DLL_EXPORT __attribute__((visibility("default")))
#else
#define DLL_EXPORT
#endif
#endif
