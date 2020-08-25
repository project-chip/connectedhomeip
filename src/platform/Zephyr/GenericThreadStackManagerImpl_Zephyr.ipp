/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Contains non-inline method definitions for the
 *          GenericThreadStackManagerImpl_Zephyr<> template.
 */

#ifndef GENERIC_THREAD_STACK_MANAGER_IMPL_ZEPHYR_IPP
#define GENERIC_THREAD_STACK_MANAGER_IMPL_ZEPHYR_IPP

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.ipp>
#include <platform/Zephyr/GenericThreadStackManagerImpl_Zephyr.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericThreadStackManagerImpl_Zephyr<ThreadStackManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_THREAD_STACK_MANAGER_IMPL_ZEPHYR_IPP
