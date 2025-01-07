/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "AndroidCheckInDelegate.h"

#include <app/icd/client/RefreshKeySender.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/logging/CHIPLogging.h>

#define PARSE_CLIENT_INFO(_clientInfo, _peerNodeId, _checkInNodeId, _startCounter, _offset, _monitoredSubject, _jniICDAesKey,      \
                          _jniICDHmacKey)                                                                                          \
    jlong _peerNodeId       = static_cast<jlong>(_clientInfo.peer_node.GetNodeId());                                               \
    jlong _checkInNodeId    = static_cast<jlong>(_clientInfo.check_in_node.GetNodeId());                                           \
    jlong _startCounter     = static_cast<jlong>(_clientInfo.start_icd_counter);                                                   \
    jlong _offset           = static_cast<jlong>(_clientInfo.offset);                                                              \
    jlong _monitoredSubject = static_cast<jlong>(_clientInfo.monitored_subject);                                                   \
    chip::ByteSpan aes_buf(_clientInfo.aes_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>());                                 \
    chip::ByteSpan hmac_buf(_clientInfo.hmac_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>());                               \
    chip::ByteArray _jniICDAesKey(env, aes_buf);                                                                                   \
    chip::ByteArray _jniICDHmacKey(env, hmac_buf);

namespace chip {
namespace app {

CHIP_ERROR AndroidCheckInDelegate::Init(ICDClientStorage * storage, InteractionModelEngine * engine)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mpStorage  = storage;
    mpImEngine = engine;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidCheckInDelegate::SetDelegate(jobject checkInDelegateObj)
{
    ReturnLogErrorOnFailure(mCheckInDelegate.Init(checkInDelegateObj));
    return CHIP_NO_ERROR;
}

void AndroidCheckInDelegate::OnCheckInComplete(const ICDClientInfo & clientInfo)
{
    ChipLogProgress(ICD,
                    "Check In Message processing complete: start_counter=%" PRIu32 " offset=%" PRIu32
                    " peernodeid=" ChipLogFormatScopedNodeId " checkinnodeid=" ChipLogFormatScopedNodeId,
                    clientInfo.start_icd_counter, clientInfo.offset, ChipLogValueScopedNodeId(clientInfo.peer_node),
                    ChipLogValueScopedNodeId(clientInfo.check_in_node));

    VerifyOrReturn(mCheckInDelegate.HasValidObjectRef(), ChipLogProgress(ICD, "check-in delegate is not implemented!"));

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "JNIEnv is null!"));
    PARSE_CLIENT_INFO(clientInfo, peerNodeId, checkInNodeId, startCounter, offset, monitoredSubject, jniICDAesKey, jniICDHmacKey)

    jmethodID onCheckInCompleteMethodID = nullptr;
    CHIP_ERROR err = chip::JniReferences::GetInstance().FindMethod(env, mCheckInDelegate.ObjectRef(), "onCheckInComplete",
                                                                   "(JJJJJ[B[B)V", &onCheckInCompleteMethodID);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogProgress(ICD, "onCheckInComplete - FindMethod is failed! : %" CHIP_ERROR_FORMAT, err.Format()));

    env->CallVoidMethod(mCheckInDelegate.ObjectRef(), onCheckInCompleteMethodID, peerNodeId, checkInNodeId, startCounter, offset,
                        monitoredSubject, jniICDAesKey.jniValue(), jniICDHmacKey.jniValue());
}

RefreshKeySender * AndroidCheckInDelegate::OnKeyRefreshNeeded(ICDClientInfo & clientInfo, ICDClientStorage * clientStorage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    RefreshKeySender::RefreshKeyBuffer newKey;

    bool hasSetKey = false;
    if (mCheckInDelegate.HasValidObjectRef())
    {
        JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(Controller, "JNIEnv is null!"));

        PARSE_CLIENT_INFO(clientInfo, peerNodeId, checkInNodeId, startCounter, offset, monitoredSubject, jniICDAesKey,
                          jniICDHmacKey)

        jmethodID onKeyRefreshNeededMethodID = nullptr;
        err = chip::JniReferences::GetInstance().FindMethod(env, mCheckInDelegate.ObjectRef(), "onKeyRefreshNeeded",
                                                            "(JJJJJ[B[B)[B", &onKeyRefreshNeededMethodID);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogProgress(ICD, "onKeyRefreshNeeded - FindMethod is failed! : %" CHIP_ERROR_FORMAT, err.Format()));

        jbyteArray key = static_cast<jbyteArray>(
            env->CallObjectMethod(mCheckInDelegate.ObjectRef(), onKeyRefreshNeededMethodID, peerNodeId, checkInNodeId, startCounter,
                                  offset, monitoredSubject, jniICDAesKey.jniValue(), jniICDHmacKey.jniValue()));

        if (key != nullptr)
        {
            JniByteArray jniKey(env, key);
            VerifyOrReturnValue(static_cast<size_t>(jniKey.size()) == newKey.Capacity(), nullptr,
                                ChipLogProgress(ICD, "Invalid key length : %d", jniKey.size()));
            memcpy(newKey.Bytes(), jniKey.data(), newKey.Capacity());
            hasSetKey = true;
        }
    }
    else
    {
        ChipLogProgress(ICD, "check-in delegate is not implemented!");
    }
    if (!hasSetKey)
    {
        err = Crypto::DRBG_get_bytes(newKey.Bytes(), newKey.Capacity());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(ICD, "Generation of new key failed: %" CHIP_ERROR_FORMAT, err.Format());
            return nullptr;
        }
        err = newKey.SetLength(newKey.Capacity());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(ICD, "Fail to set new key length with error: %" CHIP_ERROR_FORMAT, err.Format());
            return nullptr;
        }
    }

    auto refreshKeySender = Platform::New<RefreshKeySender>(this, clientInfo, clientStorage, mpImEngine, newKey);
    if (refreshKeySender == nullptr)
    {
        return nullptr;
    }
    return refreshKeySender;
}

void AndroidCheckInDelegate::OnKeyRefreshDone(RefreshKeySender * refreshKeySender, CHIP_ERROR error)
{
    if (error == CHIP_NO_ERROR)
    {
        ChipLogProgress(ICD, "Re-registration with new key completed successfully");
    }
    else
    {
        ChipLogError(ICD, "Re-registration with new key failed with error : %" CHIP_ERROR_FORMAT, error.Format());
        // The callee can take corrective action  based on the error received.
    }

    VerifyOrReturn(mCheckInDelegate.HasValidObjectRef(), ChipLogProgress(ICD, "check-in delegate is not implemented!"));

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "JNIEnv is null!"));

    jmethodID onKeyRefreshDoneMethodID = nullptr;
    CHIP_ERROR err = chip::JniReferences::GetInstance().FindMethod(env, mCheckInDelegate.ObjectRef(), "onKeyRefreshDone", "(J)V",
                                                                   &onKeyRefreshDoneMethodID);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogProgress(ICD, "onKeyRefreshDone - FindMethod is failed! : %" CHIP_ERROR_FORMAT, err.Format()));

    env->CallVoidMethod(mCheckInDelegate.ObjectRef(), onKeyRefreshDoneMethodID, static_cast<jlong>(error.AsInteger()));

    if (refreshKeySender != nullptr)
    {
        Platform::Delete(refreshKeySender);
        refreshKeySender = nullptr;
    }
}
} // namespace app
} // namespace chip
