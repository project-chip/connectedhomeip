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

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class NetworkProvisioningServerImpl;

/**
 * Provides network provisioning services for a CHIP Device.
 */
class NetworkProvisioningServer
{
    using ImplClass = NetworkProvisioningServerImpl;

public:
    // Members for internal use by components within the CHIP Device Layer.

    CHIP_ERROR Init(void);
    // NetworkProvisioningDelegate * GetDelegate(void);
    void StartPendingScan(void);
    bool ScanInProgress(void);
    void OnPlatformEvent(const CHIPDeviceEvent * event);

protected:
    // Construction/destruction limited to subclasses.
    NetworkProvisioningServer()  = default;
    ~NetworkProvisioningServer() = default;

    // No copy, move or assignment.
    NetworkProvisioningServer(const NetworkProvisioningServer &)  = delete;
    NetworkProvisioningServer(const NetworkProvisioningServer &&) = delete;
    NetworkProvisioningServer & operator=(const NetworkProvisioningServer &) = delete;
};

/**
 * Returns a reference to the public interface of the NetworkProvisioningServer singleton object.
 *
 * Internal components should use this to access features of the NetworkProvisioningServer object
 * that are common to all platforms.
 */
extern NetworkProvisioningServer & NetworkProvisioningSvr();

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing the implementation of the NetworkProvisioningServer
 * object for the selected platform.
 */
#ifdef EXTERNAL_NETWORKPROVISIONINGSERVERIMPL_HEADER
#include EXTERNAL_NETWORKPROVISIONINGSERVERIMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define NETWORKPROVISIONINGSERVERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/NetworkProvisioningServerImpl.h>
#include NETWORKPROVISIONINGSERVERIMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

namespace chip {
namespace DeviceLayer {
namespace Internal {

inline CHIP_ERROR NetworkProvisioningServer::Init(void)
{
    return static_cast<ImplClass *>(this)->_Init();
}

inline void NetworkProvisioningServer::StartPendingScan(void)
{
    static_cast<ImplClass *>(this)->_StartPendingScan();
}

inline bool NetworkProvisioningServer::ScanInProgress(void)
{
    return static_cast<ImplClass *>(this)->_ScanInProgress();
}

inline void NetworkProvisioningServer::OnPlatformEvent(const CHIPDeviceEvent * event)
{
    static_cast<ImplClass *>(this)->_OnPlatformEvent(event);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

