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
// Workaround for LeakSanitizer (LSan) false positives:
// When dynamically loaded system libraries (like ALSA or PulseAudio) are unloaded via dlclose(),
// LSan loses track of their static allocations (e.g., dynamic loader caching) and reports them
// as memory leaks from <unknown module>.
//
// Following the official recommendation in the Google Sanitizers issue tracker:
// https://github.com/google/sanitizers/issues/89#issuecomment-321602408:
// "don't dlclose anything when testing under asan/lsan", we stub dlclose() to a no-op when
// compiling under AddressSanitizer. This keeps the dynamic libraries mapped in memory at process
// exit, enabling LSan to scan their static data sections and correctly recognize allocations as reachable.
#if defined(__SANITIZE_ADDRESS__) || (defined(__has_feature) && __has_feature(address_sanitizer))
#include <dlfcn.h>
static inline int dummy_dlclose(void * handle)
{
    (void) handle;
    return 0;
}
#define dlclose(handle) dummy_dlclose(handle)
#endif

#define MINIAUDIO_IMPLEMENTATION

#include <miniaudio.h>
