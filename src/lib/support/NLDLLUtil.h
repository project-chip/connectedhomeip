/*
 *
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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

#ifndef NLDLLEXPORT_H_
#define NLDLLEXPORT_H_

#if defined _WIN32 || defined __CYGWIN__
  #define NL_DLL_IMPORT __declspec(dllimport)
  #define NL_DLL_EXPORT __declspec(dllexport)
  #define NL_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define NL_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define NL_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define NL_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define NL_DLL_IMPORT
    #define NL_DLL_EXPORT
    #define NL_DLL_LOCAL
  #endif
#endif

#endif /* NLDLLEXPORT_H_ */
