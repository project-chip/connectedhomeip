/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

// miniaudio is a header-only library.
// This file is used to generate the implementation of the library.
// By defining MINIAUDIO_IMPLEMENTATION before including miniaudio.h,
// we tell the header to include the actual function definitions.
// This file should be compiled exactly once in the project to avoid
// duplicate symbol errors during linking.
// MINIAUDIO_IMPLEMENTATION definition follows
#if defined(__SANITIZE_ADDRESS__) || (defined(__has_feature) && __has_feature(address_sanitizer))
extern "C" const char * __lsan_default_suppressions()
{
    return "leak:<unknown module>\n";
}
#endif

#define MINIAUDIO_IMPLEMENTATION

#include <miniaudio.h>
