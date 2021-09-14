/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file contains declarations of the following classes and
 *      templates:
 *
 *        - class chip::System::Object
 *        - template<typename ALIGN, size_t SIZE> union chip::System::ObjectArena
 *        - template<class T, unsigned int N> class chip::System::ObjectPool
 */

#pragma once

#include <system/SystemPoolHeap.h>
#include <system/SystemPoolNonHeap.h>

namespace chip {
namespace System {

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <class T, unsigned int N>
using ObjectPool = ObjectPoolHeap<T, N>;
#else
template <class T, unsigned int N>
using ObjectPool = ObjectPoolNonHeap<T, N>;
#endif

} // namespace System
} // namespace chip
