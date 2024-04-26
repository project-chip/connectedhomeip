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

chip::app::DefaultICDClientStorage sICDClientStorage;

jobject getICDClientInfo(JNIEnv *env, const char * icdClientInfoSign, jint jFabricIndex)
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
    constructor = env->GetMethodID(infoClass, "<init>", "(JJJJ[B[B)V");
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

        err = chip::JniReferences::GetInstance().N2J_ByteArray(env, info.aes_key_handle.As<chip::Crypto::Symmetric128BitsKeyByteArray>(),
                                                               chip::Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, jIcdAesKey);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "ICD AES KEY N2J_ByteArray error!: %" CHIP_ERROR_FORMAT, err.Format()));

        err = chip::JniReferences::GetInstance().N2J_ByteArray(env, info.hmac_key_handle.As<chip::Crypto::Symmetric128BitsKeyByteArray>(),
                                                               chip::Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, jIcdHmacKey);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "ICD HMAC KEY N2J_ByteArray error!: %" CHIP_ERROR_FORMAT, err.Format()));

        jICDClientInfo = (jobject) env->NewObject(infoClass, constructor, static_cast<jlong>(info.peer_node.GetNodeId()),
                                                  static_cast<jlong>(info.start_icd_counter), static_cast<jlong>(info.offset),
                                                  static_cast<jlong>(info.monitored_subject), jIcdAesKey, jIcdHmacKey);

        err = chip::JniReferences::GetInstance().AddToList(jInfo, jICDClientInfo);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "AddToList error!: %" CHIP_ERROR_FORMAT, err.Format()));
    }

    return jInfo;
 }

chip::app::DefaultICDClientStorage * getICDClientStorage() { return &sICDClientStorage; }
