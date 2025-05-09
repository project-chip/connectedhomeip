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
 *          Provides an implementation of the NFCCommissioningManager singleton object
 *          for the Android platforms.
 */

#pragma once

#include <jni.h>

#include <nfc/NfcApplicationDelegate.h>

#include <platform/internal/NFCCommissioningManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the NFCCommissioningManagerImpl singleton object for the Linux platforms.
 */
class NFCCommissioningManagerImpl final : public NFCCommissioningManager, private Nfc::NfcApplicationDelegate
{
    // Allow the NFCCommissioningManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend NFCCommissioningManager;

public:
    void InitializeWithObject(jobject managerObject);

    CHIP_ERROR OnNfcTagResponse(const Transport::PeerAddress & address, System::PacketBufferHandle && buffer);
    CHIP_ERROR OnNfcTagError(const Transport::PeerAddress & address);

    // ===== Members that implement virtual methods on NfcApplicationDelegate.

    void SetNFCBase(Transport::NFCBase * nfcBase) override;

    bool CanSendToPeer(const Transport::PeerAddress & address) override;

    CHIP_ERROR SendToNfcTag(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override;

private:
    // ===== Members that implement the NFCCommissioningManager internal interface.

    CHIP_ERROR _Init();
    void _Shutdown();

    // ===== Members for internal use by the following friends.

    friend NFCCommissioningManager & NFCCommissioningMgr();
    friend NFCCommissioningManagerImpl & NFCCommissioningMgrImpl();

    static NFCCommissioningManagerImpl sInstance;

    chip::JniGlobalReference mNFCCommissioningManagerObject;
    jmethodID mInitMethod           = nullptr;
    jmethodID mShutdownMethod       = nullptr;
    jmethodID mSendToNfcTagCallback = nullptr;

    Transport::NFCBase * mNFCBase = nullptr;
};

/**
 * Returns a reference to the public interface of the NFCCommissioningManager singleton object.
 *
 * Internal components should use this to access features of the NFCCommissioningManager object
 * that are common to all platforms.
 */
inline NFCCommissioningManager & NFCCommissioningMgr()
{
    return NFCCommissioningManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the NFCCommissioningManager singleton object.
 *
 * Internal components can use this to gain access to features of the NFCCommissioningManager
 * that are specific to the Linux platforms.
 */
inline NFCCommissioningManagerImpl & NFCCommissioningMgrImpl()
{
    return NFCCommissioningManagerImpl::sInstance;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
