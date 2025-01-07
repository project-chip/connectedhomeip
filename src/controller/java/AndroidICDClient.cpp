/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/**
 *    @file
 *      Implementation of ICD Client API for Android Platform
 *
 */

#include "AndroidICDClient.h"

#include <app/icd/client/ICDClientInfo.h>
#include <lib/support/JniTypeWrappers.h>

chip::app::DefaultICDClientStorage sICDClientStorage;
static CHIP_ERROR ParseICDClientInfo(JNIEnv * env, jint jFabricIndex, jobject jIcdClientInfo,
                                     chip::app::ICDClientInfo & icdClientInfo);

jobject getICDClientInfo(JNIEnv * env, const char * icdClientInfoSign, jint jFabricIndex)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    jobject jInfo = nullptr;
    chip::app::ICDClientInfo info;
    chip::FabricIndex fabricIndex = static_cast<chip::FabricIndex>(jFabricIndex);

    ChipLogProgress(Controller, "getICDClientInfo(%u) called", fabricIndex);

    err = chip::JniReferences::GetInstance().CreateArrayList(jInfo);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                        ChipLogError(Controller, "CreateArrayList failed!: %" CHIP_ERROR_FORMAT, err.Format()));

    auto iter = getICDClientStorage()->IterateICDClientInfo();
    VerifyOrReturnValue(iter != nullptr, nullptr, ChipLogError(Controller, "IterateICDClientInfo failed!"));
    chip::app::DefaultICDClientStorage::ICDClientInfoIteratorWrapper clientInfoIteratorWrapper(iter);

    jmethodID constructor;
    jclass infoClass;
    chip::JniLocalReferenceScope scope(env);

    err = chip::JniReferences::GetInstance().GetLocalClassRef(env, icdClientInfoSign, infoClass);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                        ChipLogError(Controller, "Find ICDClientInfo class: %" CHIP_ERROR_FORMAT, err.Format()));

    env->ExceptionClear();
    constructor = env->GetMethodID(infoClass, "<init>", "(JJJJJ[B[B)V");
    VerifyOrReturnValue(constructor != nullptr, nullptr, ChipLogError(Controller, "Find GetMethodID error!"));

    while (iter->Next(info))
    {
        jbyteArray jIcdAesKey  = nullptr;
        jbyteArray jIcdHmacKey = nullptr;
        jobject jICDClientInfo = nullptr;

        if (info.peer_node.GetFabricIndex() != fabricIndex)
        {
            continue;
        }

        err = chip::JniReferences::GetInstance().N2J_ByteArray(env,
                                                               info.aes_key_handle.As<chip::Crypto::Symmetric128BitsKeyByteArray>(),
                                                               chip::Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, jIcdAesKey);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "ICD AES KEY N2J_ByteArray error!: %" CHIP_ERROR_FORMAT, err.Format()));

        err = chip::JniReferences::GetInstance().N2J_ByteArray(
            env, info.hmac_key_handle.As<chip::Crypto::Symmetric128BitsKeyByteArray>(),
            chip::Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, jIcdHmacKey);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "ICD HMAC KEY N2J_ByteArray error!: %" CHIP_ERROR_FORMAT, err.Format()));

        jICDClientInfo = static_cast<jobject>(
            env->NewObject(infoClass, constructor, static_cast<jlong>(info.peer_node.GetNodeId()),
                           static_cast<jlong>(info.check_in_node.GetNodeId()), static_cast<jlong>(info.start_icd_counter),
                           static_cast<jlong>(info.offset), static_cast<jlong>(info.monitored_subject), jIcdAesKey, jIcdHmacKey));

        err = chip::JniReferences::GetInstance().AddToList(jInfo, jICDClientInfo);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "AddToList error!: %" CHIP_ERROR_FORMAT, err.Format()));
    }

    return jInfo;
}

CHIP_ERROR StoreICDEntryWithKey(JNIEnv * env, jint jFabricIndex, jobject jicdClientInfo, jbyteArray jKey)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::app::ICDClientInfo clientInfo;
    chip::JniByteArray jniKey(env, jKey);

    err = ParseICDClientInfo(env, jFabricIndex, jicdClientInfo, clientInfo);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, err,
                        ChipLogError(Controller, "Failed to parse ICD Client info: %" CHIP_ERROR_FORMAT, err.Format()));

    err = getICDClientStorage()->SetKey(clientInfo, jniKey.byteSpan());

    if (err == CHIP_NO_ERROR)
    {
        err = getICDClientStorage()->StoreEntry(clientInfo);
    }
    else
    {
        getICDClientStorage()->RemoveKey(clientInfo);
        ChipLogError(Controller, "Failed to persist symmetric key with error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR RemoveICDEntryWithKey(JNIEnv * env, jint jFabricIndex, jobject jicdClientInfo)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::app::ICDClientInfo info;
    err = ParseICDClientInfo(env, jFabricIndex, jicdClientInfo, info);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, err,
                        ChipLogError(Controller, "Failed to parse ICD Client info: %" CHIP_ERROR_FORMAT, err.Format()));

    getICDClientStorage()->RemoveKey(info);

    return err;
}

CHIP_ERROR ClearICDClientInfo(JNIEnv * env, jint jFabricIndex, jlong jNodeId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::ScopedNodeId scopedNodeId(static_cast<chip::NodeId>(jNodeId), static_cast<chip::FabricIndex>(jFabricIndex));
    err = getICDClientStorage()->DeleteEntry(scopedNodeId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "ClearICDClientInfo error!: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

CHIP_ERROR ParseICDClientInfo(JNIEnv * env, jint jFabricIndex, jobject jIcdClientInfo, chip::app::ICDClientInfo & icdClientInfo)
{
    VerifyOrReturnError(jIcdClientInfo != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    jmethodID getPeerNodeIdMethod       = nullptr;
    jmethodID getCheckInNodeIdMethod    = nullptr;
    jmethodID getStartCounterMethod     = nullptr;
    jmethodID getOffsetMethod           = nullptr;
    jmethodID getMonitoredSubjectMethod = nullptr;
    jmethodID getIcdAesKeyMethod        = nullptr;
    jmethodID getIcdHmacKeyMethod       = nullptr;

    ReturnErrorOnFailure(
        chip::JniReferences::GetInstance().FindMethod(env, jIcdClientInfo, "getPeerNodeId", "()J", &getPeerNodeIdMethod));
    ReturnErrorOnFailure(
        chip::JniReferences::GetInstance().FindMethod(env, jIcdClientInfo, "getCheckInNodeId", "()J", &getCheckInNodeIdMethod));
    ReturnErrorOnFailure(
        chip::JniReferences::GetInstance().FindMethod(env, jIcdClientInfo, "getStartCounter", "()J", &getStartCounterMethod));
    ReturnErrorOnFailure(chip::JniReferences::GetInstance().FindMethod(env, jIcdClientInfo, "getOffset", "()J", &getOffsetMethod));
    ReturnErrorOnFailure(chip::JniReferences::GetInstance().FindMethod(env, jIcdClientInfo, "getMonitoredSubject", "()J",
                                                                       &getMonitoredSubjectMethod));
    ReturnErrorOnFailure(
        chip::JniReferences::GetInstance().FindMethod(env, jIcdClientInfo, "getIcdAesKey", "()[B", &getIcdAesKeyMethod));
    ReturnErrorOnFailure(
        chip::JniReferences::GetInstance().FindMethod(env, jIcdClientInfo, "getIcdHmacKey", "()[B", &getIcdHmacKeyMethod));

    jlong jPeerNodeId       = env->CallLongMethod(jIcdClientInfo, getPeerNodeIdMethod);
    jlong jCheckInNodeId    = env->CallLongMethod(jIcdClientInfo, getCheckInNodeIdMethod);
    jlong jStartCounter     = env->CallLongMethod(jIcdClientInfo, getStartCounterMethod);
    jlong jOffset           = env->CallLongMethod(jIcdClientInfo, getOffsetMethod);
    jlong jMonitoredSubject = env->CallLongMethod(jIcdClientInfo, getMonitoredSubjectMethod);
    jbyteArray jIcdAesKey   = static_cast<jbyteArray>(env->CallObjectMethod(jIcdClientInfo, getIcdAesKeyMethod));
    jbyteArray jIcdHmacKey  = static_cast<jbyteArray>(env->CallObjectMethod(jIcdClientInfo, getIcdHmacKeyMethod));

    chip::ScopedNodeId peerNodeId(static_cast<chip::NodeId>(jPeerNodeId), static_cast<chip::FabricIndex>(jFabricIndex));
    chip::ScopedNodeId checkInNodeId(static_cast<chip::NodeId>(jCheckInNodeId), static_cast<chip::FabricIndex>(jFabricIndex));
    chip::JniByteArray jniIcdAesKey(env, jIcdAesKey);
    chip::JniByteArray jniIcdHmacKey(env, jIcdHmacKey);

    icdClientInfo.peer_node         = peerNodeId;
    icdClientInfo.check_in_node     = checkInNodeId;
    icdClientInfo.start_icd_counter = static_cast<uint32_t>(jStartCounter);
    icdClientInfo.offset            = static_cast<uint32_t>(jOffset);
    icdClientInfo.monitored_subject = static_cast<uint64_t>(jMonitoredSubject);
    memcpy(icdClientInfo.aes_key_handle.AsMutable<chip::Crypto::Symmetric128BitsKeyByteArray>(), jniIcdAesKey.data(),
           sizeof(chip::Crypto::Symmetric128BitsKeyByteArray));
    memcpy(icdClientInfo.hmac_key_handle.AsMutable<chip::Crypto::Symmetric128BitsKeyByteArray>(), jniIcdHmacKey.data(),
           sizeof(chip::Crypto::Symmetric128BitsKeyByteArray));

    return CHIP_NO_ERROR;
}

chip::app::DefaultICDClientStorage * getICDClientStorage()
{
    return &sICDClientStorage;
}
