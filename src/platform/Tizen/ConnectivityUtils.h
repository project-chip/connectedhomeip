/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class ConnectivityUtils
{
public:
    static app::Clusters::GeneralDiagnostics::InterfaceType GetInterfaceConnectionType(const char * ifname);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
