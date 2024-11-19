/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 * @file
 *   API for managing a programmable NFC Tag.
 */

#pragma once

#include <lib/core/CHIPError.h>

#include <cstddef>

namespace chip {
namespace DeviceLayer {

class NFCOnboardingManagerImpl;

/**
 * @brief Defines an abstract interface for programming onboarding data in an NFC Tag.
 *
 * NFC Tag can be used in Project CHIP to share onboarding information such
 * as the device discriminator and PIN code with a commissioner. This class
 * provides an interface to set contents of a programmable NFC Tag.
 */
class NFCOnboardingManager
{
public:
    /**
     * Initializes the NFCOnboardingManager singleton object.
     *
     * @return CHIP_NO_ERROR On success.
     */
    CHIP_ERROR Init();

    /**
     * Starts NFC Tag emulation using the specified payload.
     *
     * @param[in] payload       Address of the payload to be written to the NFC Tag.
     * @param[in] payloadLength Length of the payload.
     *
     * @return CHIP_NO_ERROR On success.
     *         CHIP_ERROR_BUFFER_TOO_SMALL Payload could not fit in an internal buffer.
     */
    CHIP_ERROR StartTagEmulation(const char * payload, size_t payloadLength);

    /**
     * Stops NFC Tag emulation.
     *
     * @return CHIP_NO_ERROR On success.
     */
    CHIP_ERROR StopTagEmulation();

    /** Determines whether NFC Tag emulation is started
     *
     * @return true NFC Tag is enabled.
     *         false NFC Tag is disabled.
     */
    bool IsTagEmulationStarted() const;

protected:
    // Construction/destruction limited to subclasses.
    NFCOnboardingManager()  = default;
    ~NFCOnboardingManager() = default;

    // No copy, move or assignment.
    NFCOnboardingManager(const NFCOnboardingManager &) = delete;
    NFCOnboardingManager(NFCOnboardingManager &&)      = delete;
    void operator=(const NFCOnboardingManager &)       = delete;

    // NFCOnboardingManager implementation getters.
    NFCOnboardingManagerImpl * Impl();
    const NFCOnboardingManagerImpl * Impl() const;
};

/**
 * Returns a reference to the public interface of the NFCOnboardingManager singleton object.
 */
extern NFCOnboardingManager & NFCOnboardingMgr();

/**
 * Returns a reference to the platform-specific NFCOnboardingManager singleton object.
 */
extern NFCOnboardingManagerImpl & NFCOnboardingMgrImpl();

} // namespace DeviceLayer
} // namespace chip

#ifdef EXTERNAL_NFC_ONBOARDING_MANAGER_IMPL_HEADER
#include EXTERNAL_NFC_ONBOARDING_MANAGER_IMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define NFC_ONBOARDING_MANAGER_IMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/NFCOnboardingManagerImpl.h>
#include NFC_ONBOARDING_MANAGER_IMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

namespace chip {
namespace DeviceLayer {

inline CHIP_ERROR NFCOnboardingManager::Init()
{
    return Impl()->_Init();
}

inline CHIP_ERROR NFCOnboardingManager::StartTagEmulation(const char * payload, size_t payloadLength)
{
    return Impl()->_StartTagEmulation(payload, payloadLength);
}

inline CHIP_ERROR NFCOnboardingManager::StopTagEmulation()
{
    return Impl()->_StopTagEmulation();
}

inline bool NFCOnboardingManager::IsTagEmulationStarted() const
{
    return Impl()->_IsTagEmulationStarted();
}

inline NFCOnboardingManagerImpl * NFCOnboardingManager::Impl()
{
    return static_cast<NFCOnboardingManagerImpl *>(this);
}

inline const NFCOnboardingManagerImpl * NFCOnboardingManager::Impl() const
{
    return static_cast<const NFCOnboardingManagerImpl *>(this);
}

} // namespace DeviceLayer
} // namespace chip
