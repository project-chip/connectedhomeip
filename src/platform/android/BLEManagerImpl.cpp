/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an implementation of the BLEManager singleton object
 *          for Linux platforms.
 */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <ble/CHIPBleServiceData.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/SafeInt.h>
#include <platform/internal/BLEManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

using namespace chip;
using namespace ::nl;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::ConfigureBle(uint32_t aAdapterId, bool aIsCentral)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerImpl::InitializeWithObject(jobject manager)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(DeviceLayer, "Failed to GetEnvForCurrentThread for BLEManager"));

    mBLEManagerObject = env->NewGlobalRef(manager);
    VerifyOrReturn(mBLEManagerObject != nullptr, ChipLogError(DeviceLayer, "Failed to NewGlobalRef BLEManager"));

    jclass BLEManagerClass = env->GetObjectClass(manager);
    VerifyOrReturn(BLEManagerClass != nullptr, ChipLogError(DeviceLayer, "Failed to get BLEManager Java class"));

    mInitMethod = env->GetMethodID(BLEManagerClass, "init", "()I");
    if (mInitMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'init' method");
        env->ExceptionClear();
    }

    mSetFlagMethod = env->GetMethodID(BLEManagerClass, "setFlag", "(JZ)J");
    if (mSetFlagMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'setFlag' method");
        env->ExceptionClear();
    }

    mHasFlagMethod = env->GetMethodID(BLEManagerClass, "hasFlag", "(J)Z");
    if (mHasFlagMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'hasFlag' method");
        env->ExceptionClear();
    }

    mOnSubscribeCharacteristicMethod = env->GetMethodID(BLEManagerClass, "onSubscribeCharacteristic", "(I[B[B)Z");
    if (mOnSubscribeCharacteristicMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'onSubscribeCharacteristic' method");
        env->ExceptionClear();
    }

    mOnUnsubscribeCharacteristicMethod = env->GetMethodID(BLEManagerClass, "onUnsubscribeCharacteristic", "(I[B[B)Z");
    if (mOnUnsubscribeCharacteristicMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'onUnsubscribeCharacteristic' method");
        env->ExceptionClear();
    }

    mOnCloseConnectionMethod = env->GetMethodID(BLEManagerClass, "onCloseConnection", "(I)Z");
    if (mOnCloseConnectionMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'onCloseConnection' method");
        env->ExceptionClear();
    }

    mOnGetMTUMethod = env->GetMethodID(BLEManagerClass, "onGetMTU", "(I)I");
    if (mOnGetMTUMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'onGetMTU' method");
        env->ExceptionClear();
    }

    mOnSendWriteRequestMethod = env->GetMethodID(BLEManagerClass, "onSendWriteRequest", "(I[B[B[B)Z");
    if (mOnSendWriteRequestMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'onSendWriteRequest' method");
        env->ExceptionClear();
    }

    mOnNotifyChipConnectionClosedMethod = env->GetMethodID(BLEManagerClass, "onNotifyChipConnectionClosed", "(I)V");
    if (mOnNotifyChipConnectionClosedMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'onNotifyChipConnectionClosed' method");
        env->ExceptionClear();
    }

    mOnNewConnectionMethod = env->GetMethodID(BLEManagerClass, "onNewConnection", "(I)V");
    if (mOnNewConnectionMethod == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to access BLEManager 'onNewConnection' method");
        env->ExceptionClear();
    }
}

// ===== start impl of BLEManager internal interface, ref BLEManager.h

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    // SystemLayer is defined in platform::Globals.cpp
    err = BleLayer::Init(this, this, this, &DeviceLayer::SystemLayer());
    ReturnLogErrorOnFailure(err);

    VerifyOrReturnLogError(mBLEManagerObject != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(mInitMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(env != nullptr, CHIP_JNI_ERROR_NO_ENV);

    jint ret = env->CallIntMethod(mBLEManagerObject, mInitMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in BLEManager::init");
        env->ExceptionDescribe();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }
    VerifyOrReturnLogError(ret == 0, CHIP_JNI_ERROR_JAVA_ERROR);

    return err;
}

bool BLEManagerImpl::_IsAdvertisingEnabled()
{
    bool has       = false;
    CHIP_ERROR err = HasFlag(Flags::kAdvertisingEnabled, has);

    VerifyOrReturnError(err == CHIP_NO_ERROR, false);
    return has;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    return SetFlag(Flags::kAdvertisingEnabled, val);
}

bool BLEManagerImpl::_IsAdvertising()
{
    bool has       = false;
    CHIP_ERROR err = HasFlag(Flags::kAdvertising, has);

    VerifyOrReturnError(err == CHIP_NO_ERROR, false);
    return has;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

uint16_t BLEManagerImpl::_NumConnections()
{
    uint16_t numCons = 0;
    return numCons;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
}

BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}
// ===== end impl of BLEManager internal interface

// ===== start implement virtual methods on BlePlatformDelegate.

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jbyteArray svcIdObj;
    jbyteArray charIdObj;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(DeviceLayer, "Received SubscribeCharacteristic");

    VerifyOrExit(mBLEManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mOnSubscribeCharacteristicMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    err = JniReferences::GetInstance().N2J_ByteArray(env, static_cast<const uint8_t *>(svcId->bytes), 16, svcIdObj);
    SuccessOrExit(err);

    err = JniReferences::GetInstance().N2J_ByteArray(env, static_cast<const uint8_t *>(charId->bytes), 16, charIdObj);
    SuccessOrExit(err);

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(conId);
    rc = (bool) env->CallBooleanMethod(mBLEManagerObject, mOnSubscribeCharacteristicMethod, static_cast<jint>(tmpConnObj), svcIdObj,
                                       charIdObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ReportError(env, err, __FUNCTION__);
        rc = false;
    }
    env->ExceptionClear();

    return rc;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jbyteArray svcIdObj;
    jbyteArray charIdObj;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(DeviceLayer, "Received UnsubscribeCharacteristic");

    VerifyOrExit(mBLEManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mOnUnsubscribeCharacteristicMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    err = JniReferences::GetInstance().N2J_ByteArray(env, static_cast<const uint8_t *>(svcId->bytes), 16, svcIdObj);
    SuccessOrExit(err);

    err = JniReferences::GetInstance().N2J_ByteArray(env, static_cast<const uint8_t *>(charId->bytes), 16, charIdObj);
    SuccessOrExit(err);

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(conId);
    rc         = (bool) env->CallBooleanMethod(mBLEManagerObject, mOnUnsubscribeCharacteristicMethod, static_cast<jint>(tmpConnObj),
                                       svcIdObj, charIdObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ReportError(env, err, __FUNCTION__);
        rc = false;
    }
    env->ExceptionClear();

    return rc;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(DeviceLayer, "Received CloseConnection");

    VerifyOrExit(mBLEManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mOnCloseConnectionMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(conId);
    rc         = (bool) env->CallBooleanMethod(mBLEManagerObject, mOnCloseConnectionMethod, static_cast<jint>(tmpConnObj));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ReportError(env, err, __FUNCTION__);
        rc = false;
    }
    env->ExceptionClear();
    return rc;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    intptr_t tmpConnObj;
    uint16_t mtu = 0;

    ChipLogProgress(DeviceLayer, "Received GetMTU");
    VerifyOrExit(mBLEManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mOnGetMTUMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);
    ;

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(conId);
    mtu        = (int16_t) env->CallIntMethod(mBLEManagerObject, mOnGetMTUMethod, static_cast<jint>(tmpConnObj));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ReportError(env, err, __FUNCTION__);
        mtu = 0;
    }
    env->ExceptionClear();

    return mtu;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                    chip::System::PacketBufferHandle pBuf)
{
    return false;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                      chip::System::PacketBufferHandle pBuf)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    jbyteArray svcIdObj;
    jbyteArray charIdObj;
    jbyteArray characteristicDataObj;
    intptr_t tmpConnObj;
    bool rc = false;

    ChipLogProgress(DeviceLayer, "Received SendWriteRequest");
    VerifyOrExit(mBLEManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mOnSendWriteRequestMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    err = JniReferences::GetInstance().N2J_ByteArray(env, static_cast<const uint8_t *>(svcId->bytes), 16, svcIdObj);
    SuccessOrExit(err);

    err = JniReferences::GetInstance().N2J_ByteArray(env, static_cast<const uint8_t *>(charId->bytes), 16, charIdObj);
    SuccessOrExit(err);

    err = JniReferences::GetInstance().N2J_ByteArray(env, pBuf->Start(), pBuf->DataLength(), characteristicDataObj);
    SuccessOrExit(err);

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(conId);
    rc = (bool) env->CallBooleanMethod(mBLEManagerObject, mOnSendWriteRequestMethod, static_cast<jint>(tmpConnObj), svcIdObj,
                                       charIdObj, characteristicDataObj);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ReportError(env, err, __FUNCTION__);
        rc = false;
    }
    env->ExceptionClear();

    return rc;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                     chip::System::PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "SendReadRequest: Not implemented");
    return true;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "SendReadRBluezonse: Not implemented");
    return true;
}

// ===== end implement virtual methods on BlePlatformDelegate.

// ===== start implement virtual methods on BleApplicationDelegate.

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    intptr_t tmpConnObj;

    ChipLogProgress(DeviceLayer, "Received NotifyChipConnectionClosed");
    VerifyOrExit(mBLEManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mOnNotifyChipConnectionClosedMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    env->ExceptionClear();
    tmpConnObj = reinterpret_cast<intptr_t>(conId);
    env->CallVoidMethod(mBLEManagerObject, mOnNotifyChipConnectionClosedMethod, static_cast<jint>(tmpConnObj));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ReportError(env, err, __FUNCTION__);
    }
    env->ExceptionClear();
}

// ===== start implement virtual methods on BleConnectionDelegate.

void BLEManagerImpl::NewConnection(BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Controller, "Received New Connection");
    VerifyOrExit(mBLEManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mOnNewConnectionMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    env->ExceptionClear();
    // TODO: The API we have here does not handle short discriminators in any
    // sane way.  Just do what we used to do, which is pretend that a short
    // discriminator is actually a long discriminator with the low bits all 0.
    uint16_t discriminator;
    if (connDiscriminator.IsShortDiscriminator())
    {
        discriminator = static_cast<uint16_t>(connDiscriminator.GetShortValue())
            << (SetupDiscriminator::kLongBits - SetupDiscriminator::kShortBits);
    }
    else
    {
        discriminator = connDiscriminator.GetLongValue();
    }
    env->CallVoidMethod(mBLEManagerObject, mOnNewConnectionMethod, static_cast<jint>(discriminator));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ReportError(env, err, __FUNCTION__);
    }
    env->ExceptionClear();
}

CHIP_ERROR BLEManagerImpl::CancelConnection()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// ===== end implement virtual methods on BleConnectionDelegate.

//== helpers

CHIP_ERROR BLEManagerImpl::HasFlag(BLEManagerImpl::Flags flag, bool & has)
{
    chip::DeviceLayer::StackUnlock unlock;
    jlong f = static_cast<jlong>(flag);

    VerifyOrReturnLogError(mBLEManagerObject != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(mHasFlagMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(env != nullptr, CHIP_JNI_ERROR_NO_ENV);

    jboolean jret = env->CallBooleanMethod(mBLEManagerObject, mHasFlagMethod, f);
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in BLEManager::hasFlag");
        env->ExceptionDescribe();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    has = static_cast<bool>(jret);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::SetFlag(BLEManagerImpl::Flags flag, bool isSet)
{
    chip::DeviceLayer::StackUnlock unlock;
    jlong jFlag     = static_cast<jlong>(flag);
    jboolean jIsSet = static_cast<jboolean>(isSet);

    VerifyOrReturnLogError(mBLEManagerObject != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(mSetFlagMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(env != nullptr, CHIP_JNI_ERROR_NO_ENV);

    env->CallLongMethod(mBLEManagerObject, mSetFlagMethod, jFlag, jIsSet);
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in BLEManager::satFlag");
        env->ExceptionDescribe();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
