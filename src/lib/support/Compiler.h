/*
 *    Copyright (c) 2024 Project CHIP Authors
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

// CHIP_HAVE_RTTI: Is C++ RTTI enabled?
#if defined(__clang__)
#define CHIP_HAVE_RTTI __has_feature(cxx_rtti)
#elif defined(__GNUC__) && defined(__GXX_RTTI)
#define CHIP_HAVE_RTTI 1
#else
#define CHIP_HAVE_RTTI 0
#endif
