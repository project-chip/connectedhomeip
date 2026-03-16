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

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

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

    // Initialize the JniGlobalReference with the manager object
    VerifyOrReturn(mNFCCommissioningManagerObject.Init(manager) == CHIP_NO_ERROR,
                   ChipLogError(DeviceLayer, "Failed to init mNFCCommissioningManagerObject"));

    jclass NFCCommissioningManagerClass = env->GetObjectClass(manager);
    VerifyOrReturn(NFCCommissioningManagerClass != nullptr,
                   ChipLogError(DeviceLayer, "Failed to get NFCCommissioningManagerClass Java class"));

    mInitMethod = env->GetMethodID(NFCCommissioningManagerClass, "init", "()I");
    if (mInitMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access NFCCommissioningManager 'init' method");
        env->ExceptionClear();
    }

    mShutdownMethod = env->GetMethodID(NFCCommissioningManagerClass, "shutdown", "()V");
    if (mShutdownMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access NFCCommissioningManager 'shutdown' method");
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

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(mNFCCommissioningManagerObject.HasValidObjectRef(), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mInitMethod != nullptr, CHIP_ERROR_INTERNAL);

    env->CallIntMethod(mNFCCommissioningManagerObject.ObjectRef(), mInitMethod);
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
        ChipLogError(DeviceLayer, "Failed to call init() Java method");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void NFCCommissioningManagerImpl::_Shutdown()
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::_Shutdown()");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    VerifyOrReturn(env != nullptr);
    VerifyOrReturn(mNFCCommissioningManagerObject.HasValidObjectRef());
    VerifyOrReturn(mShutdownMethod != nullptr);

    env->CallVoidMethod(mNFCCommissioningManagerObject.ObjectRef(), mShutdownMethod);
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
        ChipLogError(DeviceLayer, "Failed to call shutdown() Java method");
        return;
    }
}

// ===== start implement virtual methods on NfcApplicationDelegate.

void NFCCommissioningManagerImpl::SetNFCBase(Transport::NFCBase * nfcBase)
{
    mNFCBase = nfcBase;
}

bool NFCCommissioningManagerImpl::CanSendToPeer(const Transport::PeerAddress & address)
{
    // on Android platform, a single NFC Tag can be used at a time so no need to use
    //  NFCshortId to determine to what NFC Tag the command should be sent
    return true;
}

CHIP_ERROR NFCCommissioningManagerImpl::SendToNfcTag(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    const uint8_t * buffer = msgBuf->Start();
    uint32_t len           = static_cast<uint32_t>(msgBuf->DataLength());

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_ERROR_INTERNAL);

    // Create a JniLocalReferenceScope to manage local references
    chip::JniLocalReferenceScope localRefScope(env);

    jbyteArray jbArray = env->NewByteArray(static_cast<int>(len));
    if (jbArray == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to create new Java byte array");
        return CHIP_ERROR_NO_MEMORY;
    }

    env->SetByteArrayRegion(jbArray, 0, static_cast<int>(len), reinterpret_cast<const jbyte *>(buffer));
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
        ChipLogError(DeviceLayer, "Failed to set byte array region");
        return CHIP_ERROR_INTERNAL;
    }

    VerifyOrReturnError(mNFCCommissioningManagerObject.HasValidObjectRef(), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(mSendToNfcTagCallback != nullptr, CHIP_ERROR_INTERNAL);

    env->CallVoidMethod(mNFCCommissioningManagerObject.ObjectRef(), mSendToNfcTagCallback, jbArray);
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
        ChipLogError(DeviceLayer, "Failed to call Java method sendToNfcTag");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCCommissioningManagerImpl::OnNfcTagResponse(const Transport::PeerAddress & address,
                                                         System::PacketBufferHandle && buffer)
{
    if (mNFCBase == NULL)
    {
        ChipLogError(DeviceLayer, "Error! mNFCBase is null!");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mNFCBase->OnNfcTagResponse(address, std::move(buffer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCCommissioningManagerImpl::OnNfcTagError(const Transport::PeerAddress & address)
{
    if (mNFCBase == NULL)
    {
        ChipLogError(DeviceLayer, "Error! mNFCBase is null!");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mNFCBase->OnNfcTagError(address);

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
