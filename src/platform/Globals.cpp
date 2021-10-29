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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/internal/BLEManager.h>
#include <system/SystemLayerImpl.h>

namespace chip {
namespace DeviceLayer {

chip::Inet::InetLayer & InetLayer()
{
    static chip::Inet::InetLayer gInetLayer;
    return gInetLayer;
}

chip::System::LayerImpl & SystemLayerImpl()
{
    static chip::System::LayerImpl gSystemLayerImpl;
    return gSystemLayerImpl;
}

chip::System::Layer & SystemLayer()
{
    return SystemLayerImpl();
}

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
chip::System::LayerSockets & SystemLayerSockets()
{
    return SystemLayerImpl();
}
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace DeviceLayer
} // namespace chip
