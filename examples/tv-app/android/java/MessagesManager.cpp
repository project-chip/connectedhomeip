/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "MessagesManager.h"
#include "TvApp-JNI.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/config.h>
#include <cstdlib>
#include <jni.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Messages;
using namespace chip::Uint8;
using MessageResponseOption = chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type;

/** @brief Messages  Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 */
void emberAfMessagesClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "------------TV Android App: Messages::PostClusterInit");
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::Messages::Id, endpoint);
}

void MessagesManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "-----TV Android App: Messages::SetDefaultDelegate");
    MessagesManager * mgr = new MessagesManager();
    VerifyOrReturn(mgr != nullptr, ChipLogError(Zcl, "Failed to create MessagesManager"));
    mgr->InitializeWithObjects(manager);
    chip::app::Clusters::Messages::SetDefaultDelegate(static_cast<EndpointId>(endpoint), mgr);
}

void MessagesManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for MessagesManager"));

    VerifyOrReturn(mMessagesManagerObject.Init(managerObject) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "Failed to init mMessagesManagerObject"));

    jclass managerClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(Zcl, "Failed to get MessagesManager Java class"));

    mGetMessagesMethod = env->GetMethodID(managerClass, "getMessages", "()[Lcom/matter/tv/server/tvapp/Message;");
    if (mGetMessagesMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MessagesManager 'getMessages' method");
        env->ExceptionClear();
    }

    mPresentMessagesMethod =
        env->GetMethodID(managerClass, "presentMessages", "(Ljava/lang/String;IIJJLjava/lang/String;Ljava/util/HashMap;)Z");
    if (mPresentMessagesMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MessagesManager 'presentMessages' method");
        env->ExceptionClear();
    }

    mCancelMessagesMethod = env->GetMethodID(managerClass, "cancelMessage", "(Ljava/lang/String;)Z");
    if (mCancelMessagesMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MessagesManager 'cancelMessage' method");
        env->ExceptionClear();
    }
}

uint32_t MessagesManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kEndpointFeatureMap;
    }

    BitMask<Feature> FeatureMap;
    FeatureMap.Set(Feature::kReceivedConfirmation);
    FeatureMap.Set(Feature::kConfirmationResponse);
    FeatureMap.Set(Feature::kConfirmationReply);
    FeatureMap.Set(Feature::kProtectedMessages);

    uint32_t featureMap = FeatureMap.Raw();
    // forcing to all features since this implementation supports all
    // Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}

CHIP_ERROR MessagesManager::HandleGetMessages(AttributeValueEncoder & aEncoder)
{
    DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    env->ExceptionClear();

    ChipLogProgress(Zcl, "Received MessagesManager::HandleGetMessages");
    VerifyOrExit(mMessagesManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetMessagesMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jobjectArray messagesList =
            static_cast<jobjectArray>(env->CallObjectMethod(mMessagesManagerObject.ObjectRef(), mGetMessagesMethod));
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in MessagesManager::HandleGetMessages");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }

        jint length = env->GetArrayLength(messagesList);

        for (jint i = 0; i < length; i++)
        {
            std::vector<MessageResponseOption> options;
            std::vector<JniUtfString *> optionLabels;
            uint8_t buf[kMessageIdLength];

            chip::app::Clusters::Messages::Structs::MessageStruct::Type message;
            jobject messageObject = env->GetObjectArrayElement(messagesList, i);
            jclass messageClass   = env->GetObjectClass(messageObject);

            jfieldID getMessageIdField = env->GetFieldID(messageClass, "messageId", "Ljava/lang/String;");
            jstring jmessageId         = static_cast<jstring>(env->GetObjectField(messageObject, getMessageIdField));
            JniUtfString messageId(env, jmessageId);
            if (jmessageId != nullptr)
            {
                VerifyOrReturnValue(chip::Encoding::HexToBytes(messageId.charSpan().data(), messageId.charSpan().size(), buf,
                                                               sizeof(buf)) == sizeof(buf),
                                    CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(Zcl, "HexToBytes failed"));
                message.messageID = ByteSpan(buf, sizeof(buf));
            }

            jfieldID getMessageTextField = env->GetFieldID(messageClass, "messageText", "Ljava/lang/String;");
            jstring jmessageText         = static_cast<jstring>(env->GetObjectField(messageObject, getMessageTextField));
            JniUtfString messageText(env, jmessageText);
            if (jmessageText != nullptr)
            {
                message.messageText = messageText.charSpan();
            }

            jfieldID messageControlField = env->GetFieldID(messageClass, "messageControl", "I");
            jint jmessageControl         = env->GetIntField(messageObject, messageControlField);
            message.messageControl       = static_cast<chip::BitMask<MessageControlBitmap>>(static_cast<uint8_t>(jmessageControl));

            jfieldID priorityField = env->GetFieldID(messageClass, "priority", "I");
            jint jpriority         = env->GetIntField(messageObject, priorityField);
            if (jpriority >= 0)
            {
                message.priority = MessagePriorityEnum(static_cast<uint8_t>(jpriority));
            }

            jfieldID startTimeField = env->GetFieldID(messageClass, "startTime", "J");
            jlong jstartTime        = env->GetLongField(messageObject, startTimeField);
            if (jstartTime >= 0)
            {
                message.startTime = DataModel::Nullable<uint32_t>(static_cast<uint32_t>(jstartTime));
            }

            jfieldID durationField = env->GetFieldID(messageClass, "duration", "J");
            jlong jduration        = env->GetLongField(messageObject, durationField);
            if (jduration >= 0)
            {
                message.duration = DataModel::Nullable<uint64_t>(static_cast<uint64_t>(jduration));
            }

            jfieldID getResponseOptionsField =
                env->GetFieldID(messageClass, "responseOptions", "[Lcom/matter/tv/server/tvapp/MessageResponseOption;");

            jobjectArray responsesArray = static_cast<jobjectArray>(env->GetObjectField(messageObject, getResponseOptionsField));
            jint size                   = env->GetArrayLength(responsesArray);
            if (size > 0)
            {
                for (jint j = 0; j < size; j++)
                {
                    MessageResponseOption option;

                    jobject responseOptionObject = env->GetObjectArrayElement(responsesArray, j);
                    jclass responseOptionClass   = env->GetObjectClass(responseOptionObject);

                    jfieldID idField         = env->GetFieldID(responseOptionClass, "id", "J");
                    jlong jid                = env->GetLongField(responseOptionObject, idField);
                    option.messageResponseID = Optional<uint32_t>(static_cast<uint32_t>(jid));

                    jfieldID getLabelField = env->GetFieldID(responseOptionClass, "label", "Ljava/lang/String;");
                    jstring jlabelText     = static_cast<jstring>(env->GetObjectField(responseOptionObject, getLabelField));
                    VerifyOrReturnValue(jlabelText != nullptr, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(Zcl, "jlabelText null"));
                    JniUtfString * label = new JniUtfString(env, jlabelText);
                    VerifyOrReturnValue(label != nullptr, CHIP_ERROR_NO_MEMORY, ChipLogError(Zcl, "label null"));

                    optionLabels.push_back(label);

                    option.label = Optional<CharSpan>(label->charSpan());

                    options.push_back(option);
                }

                message.responses = Optional<DataModel::List<MessageResponseOption>>(
                    DataModel::List<MessageResponseOption>(options.data(), options.size()));
            }
            ReturnErrorOnFailure(encoder.Encode(message));
            for (JniUtfString * optionLabel : optionLabels)
            {
                delete optionLabel;
            }
        }

        return CHIP_NO_ERROR;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MessagesManager::HandleGetMessages status error: %s", err.AsString());
    }
    return err;
}

CHIP_ERROR MessagesManager::HandleGetActiveMessageIds(AttributeValueEncoder & aEncoder)
{
    DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MessagesManager::HandleGetActiveMessageIds");
    VerifyOrExit(mMessagesManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetMessagesMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jobjectArray messagesList =
            static_cast<jobjectArray>(env->CallObjectMethod(mMessagesManagerObject.ObjectRef(), mGetMessagesMethod));
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in MessagesManager::HandleGetActiveMessageIds");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }

        jint length = env->GetArrayLength(messagesList);

        for (jint i = 0; i < length; i++)
        {
            jobject messageObject = env->GetObjectArrayElement(messagesList, i);
            jclass messageClass   = env->GetObjectClass(messageObject);

            jfieldID getMessageIdField = env->GetFieldID(messageClass, "messageId", "Ljava/lang/String;");
            jstring jmessageId         = static_cast<jstring>(env->GetObjectField(messageObject, getMessageIdField));
            JniUtfString messageId(env, jmessageId);
            if (jmessageId != nullptr)
            {
                uint8_t buf[kMessageIdLength];
                VerifyOrReturnValue(chip::Encoding::HexToBytes(messageId.charSpan().data(), messageId.charSpan().size(), buf,
                                                               sizeof(buf)) == sizeof(buf),
                                    CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(Zcl, "HexToBytes failed"));

                ReturnErrorOnFailure(encoder.Encode(ByteSpan(buf, sizeof(buf))));
            }
        }

        return CHIP_NO_ERROR;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MessagesManager::HandleGetMessages status error: %s", err.AsString());
    }

    return err;
}

CHIP_ERROR MessagesManager::HandlePresentMessagesRequest(
    const ByteSpan & messageId, const MessagePriorityEnum & priority, const BitMask<MessageControlBitmap> & messageControl,
    const DataModel::Nullable<uint32_t> & startTime, const DataModel::Nullable<uint64_t> & duration, const CharSpan & messageText,
    const Optional<DataModel::DecodableList<MessageResponseOption>> & responses)
{
    DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MessagesManager::HandlePresentMessagesRequest");
    VerifyOrReturnError(mMessagesManagerObject.HasValidObjectRef(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Zcl, "Invalid mMessagesManagerObject"));
    VerifyOrReturnError(mPresentMessagesMethod != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Zcl, "mPresentMessagesMethod null"));

    env->ExceptionClear();
    {
        char hex_buf[(kMessageIdLength * 2) + 1];
        VerifyOrReturnError(
            CHIP_NO_ERROR ==
                chip::Encoding::BytesToUppercaseHexString(messageId.data(), messageId.size(), hex_buf, sizeof(hex_buf)),
            CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "BytesToUppercaseHexString failed"));

        jstring jid = env->NewStringUTF(hex_buf);
        if (jid == nullptr)
        {
            return CHIP_ERROR_INTERNAL;
        }

        std::string smessageText(messageText.data(), messageText.size());
        jstring jmessageText = env->NewStringUTF(smessageText.c_str());
        if (jmessageText == nullptr)
        {
            return CHIP_ERROR_INTERNAL;
        }

        jint jcontrol   = static_cast<jint>(messageControl.Raw());
        jlong jduration = -1;
        if (!duration.IsNull())
        {
            jduration = static_cast<jlong>(duration.Value());
        }
        jlong jstartTime = -1;
        if (!startTime.IsNull())
        {
            jstartTime = static_cast<jlong>(startTime.Value());
        }

        jint jpriority = static_cast<jint>(priority);

        jclass hashMapClass = env->FindClass("java/util/HashMap");
        VerifyOrReturnError(hashMapClass != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Could not find class HashMap"));
        jmethodID hashMapCtor = env->GetMethodID(hashMapClass, "<init>", "()V");
        VerifyOrReturnError(hashMapCtor != nullptr, CHIP_ERROR_INCORRECT_STATE,
                            ChipLogError(Zcl, "Could not find HashMap constructor"));
        jobject joptions = env->NewObject(hashMapClass, hashMapCtor);
        VerifyOrReturnError(joptions != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Could not create HashMap"));

        if (responses.HasValue())
        {
            jmethodID hashMapPut =
                env->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
            VerifyOrReturnError(hashMapPut != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Could not find HashMap put"));

            jclass longClass = env->FindClass("java/lang/Long");
            VerifyOrReturnError(longClass != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Could not find class Long"));
            jmethodID longCtor = env->GetMethodID(longClass, "<init>", "(J)V");
            VerifyOrReturnError(longCtor != nullptr, CHIP_ERROR_INCORRECT_STATE,
                                ChipLogError(Zcl, "Could not find Long constructor"));

            auto iter = responses.Value().begin();
            while (iter.Next())
            {
                auto & response = iter.GetValue();

                std::string label(response.label.Value().data(), response.label.Value().size());
                jstring jlabel = env->NewStringUTF(label.c_str());
                if (jlabel == nullptr)
                {
                    return CHIP_ERROR_INTERNAL;
                }

                jobject jlongobj = env->NewObject(longClass, longCtor, static_cast<uint64_t>(response.messageResponseID.Value()));
                VerifyOrReturnError(jlongobj != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Could not create Long"));

                // add to HashMap
                env->CallObjectMethod(joptions, hashMapPut, jlongobj, jlabel);
                if (env->ExceptionCheck())
                {
                    ChipLogError(DeviceLayer, "Java exception in MessagesManager::HandlePresentMessagesRequest");
                    env->ExceptionDescribe();
                    env->ExceptionClear();
                    return CHIP_ERROR_INTERNAL;
                }
            }
        }

        env->CallBooleanMethod(mMessagesManagerObject.ObjectRef(), mPresentMessagesMethod, jid, jpriority, jcontrol, jstartTime,
                               jduration, jmessageText, joptions);
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in MessagesManager::HandlePresentMessagesRequest");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INTERNAL;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MessagesManager::HandleCancelMessagesRequest(const DataModel::DecodableList<ByteSpan> & messageIds)
{
    DeviceLayer::StackUnlock unlock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MessagesManager::HandleCancelMessagesRequest");
    VerifyOrReturnError(mMessagesManagerObject.HasValidObjectRef(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Zcl, "Invalid mMessagesManagerObject"));
    VerifyOrReturnError(mCancelMessagesMethod != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Zcl, "mCancelMessagesMethod null"));

    env->ExceptionClear();

    auto iter = messageIds.begin();
    while (iter.Next())
    {
        auto & id = iter.GetValue();

        char hex_buf[(kMessageIdLength * 2) + 1];
        VerifyOrReturnError(CHIP_NO_ERROR ==
                                chip::Encoding::BytesToUppercaseHexString(id.data(), id.size(), hex_buf, sizeof(hex_buf)),
                            CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "BytesToUppercaseHexString failed"));

        jstring jid = env->NewStringUTF(hex_buf);
        if (jid == nullptr)
        {
            return CHIP_ERROR_INTERNAL;
        }

        env->CallBooleanMethod(mMessagesManagerObject.ObjectRef(), mCancelMessagesMethod, jid);
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in MessagesManager::HandleCancelMessagesRequest");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INTERNAL;
        }
    }
    return CHIP_NO_ERROR;
}
