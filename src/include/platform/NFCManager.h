/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

class NFCManagerImpl;

/**
 * @brief Defines an abstract interface for programming an NFC Tag.
 *
 * NFC Tag can be used in Project CHIP to share commissioning information such
 * as the device discriminator and PIN code with a commissioner. This class
 * provides an interface to set contents of a programmable NFC Tag.
 */
class NFCManager
{
public:
    /**
     * Initializes the NFCManager singleton object.
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
    NFCManager()  = default;
    ~NFCManager() = default;

    // No copy, move or assignment.
    NFCManager(const NFCManager &) = delete;
    NFCManager(NFCManager &&)      = delete;
    void operator=(const NFCManager &) = delete;

    // NFCManager implementation getters.
    NFCManagerImpl * Impl();
    const NFCManagerImpl * Impl() const;
};

/**
 * Returns a reference to the public interface of the NFCManager singleton object.
 */
extern NFCManager & NFCMgr();

/**
 * Returns a reference to the platform-specific NFCManager singleton object.
 */
extern NFCManagerImpl & NFCMgrImpl();

} // namespace DeviceLayer
} // namespace chip

#ifdef EXTERNAL_NFCMANAGERIMPL_HEADER
#include EXTERNAL_NFCMANAGERIMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define NFCMANAGERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/NFCManagerImpl.h>
#include NFCMANAGERIMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

namespace chip {
namespace DeviceLayer {

inline CHIP_ERROR NFCManager::Init()
{
    return Impl()->_Init();
}

inline CHIP_ERROR NFCManager::StartTagEmulation(const char * payload, size_t payloadLength)
{
    return Impl()->_StartTagEmulation(payload, payloadLength);
}

inline CHIP_ERROR NFCManager::StopTagEmulation()
{
    return Impl()->_StopTagEmulation();
}

inline bool NFCManager::IsTagEmulationStarted() const
{
    return Impl()->_IsTagEmulationStarted();
}

inline NFCManagerImpl * NFCManager::Impl()
{
    return static_cast<NFCManagerImpl *>(this);
}

inline const NFCManagerImpl * NFCManager::Impl() const
{
    return static_cast<const NFCManagerImpl *>(this);
}

} // namespace DeviceLayer
} // namespace chip
