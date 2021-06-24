/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides implementations of the CHIP System Layer platform
 *          timer functions that are suitable for use on all platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

using namespace ::chip::DeviceLayer;

CHIP_ERROR StartTimer(System::Layer & aLayer, void * aContext, uint32_t aMilliseconds)
{
    return PlatformMgr().StartChipTimer(aMilliseconds);
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
