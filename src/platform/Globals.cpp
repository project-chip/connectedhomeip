/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <system/SystemLayerImpl.h>

namespace chip {
namespace DeviceLayer {

chip::System::LayerImpl * gMockedSystemLayer = nullptr;

void SetSystemLayerForTesting(System::LayerImpl * layer)
{
    gMockedSystemLayer = layer;
}

chip::System::LayerImpl & SystemLayerImpl()
{
    if (gMockedSystemLayer != nullptr)
        return *gMockedSystemLayer;

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

namespace Internal {
const char * const TAG = "CHIP[DL]";
} // namespace Internal

} // namespace DeviceLayer
} // namespace chip
