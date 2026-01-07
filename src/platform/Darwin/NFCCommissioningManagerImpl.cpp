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

#include <lib/core/Global.h>
#include <memory>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/NFCCommissioningManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

using namespace chip;
using namespace ::nl;
using namespace ::chip::Nfc;

namespace chip {
namespace DeviceLayer {
namespace Internal {

///////////////////////////////////////////////////////////////////////////////////////////////

Global<NFCCommissioningManagerImpl> sInstance;

/**
 * Returns a reference to the public interface of the NFCCommissioningManager singleton object.
 *
 * Internal components should use this to access features of the NFCCommissioningManager object
 * that are common to all platforms.
 */
NFCCommissioningManager & NFCCommissioningMgr()
{
    return sInstance.get();
}

/**
 * Returns the platform-specific implementation of the NFCCommissioningManager singleton object.
 *
 * Internal components can use this to gain access to features of the NFCCommissioningManager
 * that are specific to the Linux platforms.
 */
NFCCommissioningManagerImpl & NFCCommissioningMgrImpl()
{
    return sInstance.get();
}

// ===== start impl of NFCCommissioningManager internal interface, ref NFCCommissioningManager.h

CHIP_ERROR NFCCommissioningManagerImpl::_Init()
{
    ChipLogDetail(DeviceLayer, "Initializing Darwin NFC Commissioning Manager");
    return CHIP_NO_ERROR;
}

void NFCCommissioningManagerImpl::_Shutdown()
{
    ChipLogDetail(DeviceLayer, "Shutting down Darwin NFC Commissioning Manager and clearing reader transport");
    mReaderTransport.reset(nullptr);
}

void NFCCommissioningManagerImpl::_SetNFCReaderTransport(Nfc::NFCReaderTransport * readerTransport)
{
    ChipLogDetail(DeviceLayer, "Setting Darwin NFC Commissioning Reader Transport");
    mReaderTransport.reset(readerTransport);
}

// ===== start implement virtual methods on NfcApplicationDelegate.

void NFCCommissioningManagerImpl::SetNFCBase(Transport::NFCBase * nfcBase)
{
    mNFCBase = nfcBase;
    ChipLogDetail(DeviceLayer, "%s NFCBase for Darwin NFCCommissioningManagerImpl", nfcBase ? "Setting" : "Clearing");
}

bool NFCCommissioningManagerImpl::CanSendToPeer(const Transport::PeerAddress & address)
{
    if (!mReaderTransport)
    {
        ChipLogError(DeviceLayer, "Cannot send to NFC tag %u since reader transport is not valid", address.GetNFCShortId());
        return false;
    }

    chip::Nfc::NFCTag::Identifier identifier = { .discriminator = address.GetNFCShortId() };
    return mReaderTransport->FindTagMatchingIdentifier(identifier);
}

CHIP_ERROR NFCCommissioningManagerImpl::SendToNfcTag(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    if (!mReaderTransport)
    {
        ChipLogError(DeviceLayer, "Unable to send message to NFC tag %u since transport is not valid", address.GetNFCShortId());
        return CHIP_ERROR_INCORRECT_STATE;
    }

    chip::Nfc::NFCTag::Identifier identifier = { .discriminator = address.GetNFCShortId() };

    ChipLogProgress(DeviceLayer, "Sending message of length %lu bytes to NFC tag %u", msgBuf->DataLength(),
                    identifier.discriminator);

    CHIP_ERROR err = mReaderTransport->SendMessage(
        std::move(msgBuf), identifier, [&, identifier](System::PacketBufferHandle && responseBuffer, CHIP_ERROR error) -> void {
            if (error == CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer,
                                "Successfully sent message to NFC tag %u, received response buffer of length %lu bytes",
                                identifier.discriminator, responseBuffer->DataLength());
                this->mNFCBase->OnNfcTagResponse(address, std::move(responseBuffer));
            }
            else
            {
                ChipLogError(DeviceLayer, "Received failure response sending message to NFC tag %u, error: %u",
                             identifier.discriminator, error.AsInteger());
                this->mNFCBase->OnNfcTagError(address);
            }
        });

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to SendMessage to NFC tag %u, due to error: %u", identifier.discriminator,
                     err.AsInteger());
    }
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
