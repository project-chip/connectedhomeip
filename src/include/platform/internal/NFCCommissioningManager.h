/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *          Defines the abstract interface for the Device Layer's
 *          internal NFCCommissioningManager object.
 */

#pragma once

#include <lib/support/CodeUtils.h>
#include <platform/ConnectivityManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

namespace chip {
namespace DeviceLayer {
namespace Internal {

class NFCCommissioningManagerImpl;

/**
 * Provides control over CHIPoNFC services and connectivity for a chip device.
 *
 * NFCCommissioningManager defines the abstract interface of a singleton object that provides
 * control over CHIPoNFC services and connectivity for a chip device.  NFCCommissioningManager
 * is an internal object that is used by other components with the chip Device
 * Layer, but is not directly accessible to the application.
 */
class NFCCommissioningManager
{
    using ImplClass = NFCCommissioningManagerImpl;

public:
    // ===== Members that define the internal interface of the NFCCommissioningManager

    using CHIPoNFCServiceMode = ConnectivityManager::CHIPoNFCServiceMode;

    CHIP_ERROR Init();
    void Shutdown();

protected:
    // Construction/destruction limited to subclasses.
    NFCCommissioningManager()  = default;
    ~NFCCommissioningManager() = default;

    // No copy, move or assignment.
    NFCCommissioningManager(const NFCCommissioningManager &)             = delete;
    NFCCommissioningManager(const NFCCommissioningManager &&)            = delete;
    NFCCommissioningManager & operator=(const NFCCommissioningManager &) = delete;
};

/**
 * Returns a reference to the public interface of the NFCCommissioningManager singleton object.
 *
 * Internal components should use this to access features of the NFCCommissioningManager object
 * that are common to all platforms.
 */
extern NFCCommissioningManager & NFCCommissioningMgr();

/**
 * Returns the platform-specific implementation of the NFCCommissioningManager singleton object.
 *
 * chip applications can use this to gain access to features of the NFCCommissioningManager
 * that are specific to the selected platform.
 */
extern NFCCommissioningManagerImpl & NFCCommissioningMgrImpl();

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing the implementation of the NFCCommissioningManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_NFC_COMMISSIONING_MANAGER_IMPL_HEADER
#include EXTERNAL_NFC_COMMISSIONING_MANAGER_IMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define NFC_COMMISSIONING_MANAGER_IMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/NFCCommissioningManagerImpl.h>
#include NFC_COMMISSIONING_MANAGER_IMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

namespace chip {
namespace DeviceLayer {
namespace Internal {

inline CHIP_ERROR NFCCommissioningManager::Init()
{
    return static_cast<ImplClass *>(this)->_Init();
}

inline void NFCCommissioningManager::Shutdown()
{
    static_cast<ImplClass *>(this)->_Shutdown();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
