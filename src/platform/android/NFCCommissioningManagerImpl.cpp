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
 *          for Android platforms.
 */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/SafeInt.h>
#include <platform/internal/NFCCommissioningManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_COMMISSIONING

using namespace chip;
using namespace ::nl;
using namespace ::chip::Nfc;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {} // namespace

NFCCommissioningManagerImpl NFCCommissioningManagerImpl::sInstance;

void NFCCommissioningManagerImpl::InitializeWithObject(jobject manager)
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::InitializeWithObject()");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(DeviceLayer, "Failed to GetEnvForCurrentThread for NFCCommissioningManager"));

    mNFCCommissioningManagerObject = env->NewGlobalRef(manager);
    VerifyOrReturn(mNFCCommissioningManagerObject != nullptr,
                   ChipLogError(DeviceLayer, "Failed to NewGlobalRef NFCCommissioningManager"));

    jclass NFCCommissioningManagerClass = env->GetObjectClass(manager);
    VerifyOrReturn(NFCCommissioningManagerClass != nullptr,
                   ChipLogError(DeviceLayer, "Failed to get NFCCommissioningManagerClass Java class"));

    mInitMethod = env->GetMethodID(NFCCommissioningManagerClass, "init", "()I");
    if (mInitMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access NFCCommissioningManager 'init' method");
        env->ExceptionClear();
    }

    mSendToNfcTagCallback = env->GetMethodID(NFCCommissioningManagerClass, "sendToNfcTag", "([B)V");
    if (mSendToNfcTagCallback == nullptr)
    {
        ChipLogError(Controller, "Failed to access callback 'sendToNfcTag' method");
        env->ExceptionClear();
    }
}

// ===== start impl of NFCCommissioningManager internal interface, ref NFCCommissioningManager.h

CHIP_ERROR NFCCommissioningManagerImpl::_Init()
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::_Init()");

    return CHIP_NO_ERROR;
}

// ===== start implement virtual methods on NfcApplicationDelegate.

void NFCCommissioningManagerImpl::SetNFCBase(Transport::NFCBase * nfcBase)
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::SetNFCBase()");
    mNFCBase = nfcBase;
}

CHIP_ERROR NFCCommissioningManagerImpl::SendToNfcTag(System::PacketBufferHandle && msgBuf)
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::SendToNfcTag()");

    const uint8_t * buffer = msgBuf->Start();
    uint32_t len           = (uint32_t) msgBuf->DataLength();

    JNIEnv * env       = JniReferences::GetInstance().GetEnvForCurrentThread();
    jbyteArray jbArray = env->NewByteArray((int) len);
    env->SetByteArrayRegion(jbArray, 0, (int) len, (jbyte *) buffer);
    env->CallVoidMethod(mNFCCommissioningManagerObject, mSendToNfcTagCallback, jbArray);

    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCCommissioningManagerImpl::OnNfcTagResponse(System::PacketBufferHandle && buffer)
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::OnNfcTagResponse()");

    if (mNFCBase == NULL)
    {
        ChipLogError(DeviceLayer, "Error! mNFCBase is null!");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mNFCBase->OnNfcTagResponse(std::move(buffer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCCommissioningManagerImpl::OnNfcTagError()
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::OnNfcTagError()");

    if (mNFCBase == NULL)
    {
        ChipLogError(DeviceLayer, "Error! mNFCBase is null!");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mNFCBase->OnNfcTagError();

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_COMMISSIONING
