/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 * @param endpoint   Ver.: always
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

    mPresentMessagesMethod = env->GetMethodID(
        managerClass, "presentMessages",
        "(Ljava/lang/String;Lcom/matter/tv/server/tvapp/Message$PriorityType;IJILjava/lang/String;Ljava/util/Vector;)Z");
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MessagesManager::HandleGetMessages");
    VerifyOrExit(mMessagesManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetMessagesMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jobjectArray messagesList = (jobjectArray) env->CallObjectMethod(mMessagesManagerObject.ObjectRef(), mGetMessagesMethod);
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

            jfieldID startTimeField = env->GetFieldID(messageClass, "startTime", "J");
            jint jstartTime         = env->GetIntField(messageObject, startTimeField);
            if (jstartTime >= 0)
            {
                message.startTime = DataModel::Nullable<uint32_t>(static_cast<uint32_t>(jstartTime));
            }

            jfieldID durationField = env->GetFieldID(messageClass, "duration", "I");
            jint jduration         = env->GetIntField(messageObject, durationField);
            if (jduration >= 0)
            {
                message.duration = DataModel::Nullable<uint16_t>(static_cast<uint16_t>(jduration));
            }

            jfieldID getResponseOptionsField = env->GetFieldID(messageClass, "responseOptions", "Ljava/lang/Vector;");

            jobjectArray responsesArray = (jobjectArray) env->GetObjectField(messageObject, getResponseOptionsField);
            jint size                   = env->GetArrayLength(responsesArray);
            if (size > 0)
            {
                // MessageResponseOption * optionArray = new MessageResponseOption[static_cast<size_t>(size)];
                // VerifyOrReturnError(optionArray != nullptr, CHIP_ERROR_NO_MEMORY,
                //                     ChipLogProgress(Controller, "HandleGetMessages MessageResponseOption alloc failed"));
                // optionToFree.push_back(optionArray); // TODO

                for (jint j = 0; j < size; j++)
                {
                    jobject responseOptionObject = env->GetObjectArrayElement(responsesArray, j);
                    jclass responseOptionClass   = env->GetObjectClass(responseOptionObject);

                    jfieldID getLabelField = env->GetFieldID(responseOptionClass, "label", "Ljava/lang/String;");
                    jstring jlabelText     = static_cast<jstring>(env->GetObjectField(responseOptionObject, getLabelField));
                    JniUtfString label(env, jlabelText);
                    MessageResponseOption option;
                    if (jlabelText != nullptr)
                    {
                        option.label = Optional<CharSpan>(label.charSpan());
                    }

                    jfieldID idField         = env->GetFieldID(responseOptionClass, "id", "J");
                    jint jid                 = env->GetIntField(responseOptionObject, idField);
                    option.messageResponseID = Optional<uint32_t>(static_cast<uint32_t>(jid));
                    options.push_back(option);
                }

                message.responses = Optional<DataModel::List<MessageResponseOption>>(
                    DataModel::List<MessageResponseOption>(options.data(), options.size()));
            }
            ReturnErrorOnFailure(encoder.Encode(message));
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MessagesManager::HandleGetActiveMessageIds");
    VerifyOrExit(mMessagesManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetMessagesMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jobjectArray messagesList = (jobjectArray) env->CallObjectMethod(mMessagesManagerObject.ObjectRef(), mGetMessagesMethod);
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
    const DataModel::Nullable<uint32_t> & startTime, const DataModel::Nullable<uint16_t> & duration, const CharSpan & messageText,
    const Optional<DataModel::DecodableList<MessageResponseOption>> & responses)
{
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

        UtfString jIdentifier(env, hex_buf);
        UtfString jMessageText(env, messageText);

        jint jcontrol  = static_cast<jint>(messageControl.Raw());
        jint jduration = -1;
        if (duration.IsNull())
        {
            jduration = static_cast<jint>(duration.Value());
        }
        jlong jstartTime = -1;
        if (startTime.IsNull())
        {
            jstartTime = static_cast<jlong>(startTime.Value());
        }

        jint jniPriority = static_cast<jint>(priority);
        jclass priorityTypeClass;
        VerifyOrReturnError(CHIP_NO_ERROR !=
                                chip::JniReferences::GetInstance().GetLocalClassRef(
                                    env, "com/matter/tv/server/tvapp/Message$PriorityType", priorityTypeClass),
                            CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Could not find class Message$PriorityType"));
        jmethodID priorityTypeCtor = env->GetMethodID(priorityTypeClass, "<init>", "(I)V");
        VerifyOrReturnError(priorityTypeCtor != nullptr, CHIP_ERROR_INCORRECT_STATE,
                            ChipLogError(Zcl, "Could not find Message$PriorityType constructor"));
        jobject jpriority = env->NewObject(priorityTypeClass, priorityTypeCtor, jniPriority);

        jclass vectorClass = env->FindClass("java/util/Vector");
        VerifyOrReturnError(vectorClass != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Could not find class Vector"));
        jmethodID vectorCtor = env->GetMethodID(priorityTypeClass, "<init>", "()V");
        VerifyOrReturnError(vectorCtor != nullptr, CHIP_ERROR_INCORRECT_STATE,
                            ChipLogError(Zcl, "Could not find Vector constructor"));
        jobject joptions = env->NewObject(vectorClass, vectorCtor);

        jmethodID vectorAdd = env->GetMethodID(priorityTypeClass, "add", "(Ljava/lang/Object;)Z");

        if (responses.HasValue())
        {
            jclass messageResponseOptionClass;
            VerifyOrReturnError(CHIP_NO_ERROR !=
                                    chip::JniReferences::GetInstance().GetLocalClassRef(
                                        env, "com/matter/tv/server/tvapp/MessageResponseOption", messageResponseOptionClass),
                                CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Could not find class MessageResponseOption"));
            jmethodID messageResponseOptionCtor = env->GetMethodID(messageResponseOptionClass, "<init>", "(JLjava/lang/String;)V");
            VerifyOrReturnError(messageResponseOptionCtor != nullptr, CHIP_ERROR_INCORRECT_STATE,
                                ChipLogError(Zcl, "Could not find MessageResponseOption constructor"));

            auto iter = responses.Value().begin();
            while (iter.Next())
            {
                auto & response = iter.GetValue();

                jlong jniid = static_cast<jlong>(response.messageResponseID.Value());
                UtfString jlabel(env, response.label.Value());

                jobject jmessageResponseOption =
                    env->NewObject(messageResponseOptionClass, messageResponseOptionCtor, jniid, jlabel.jniValue());

                // add to vector
                env->CallBooleanMethod(joptions, vectorAdd, jmessageResponseOption);
                if (env->ExceptionCheck())
                {
                    ChipLogError(DeviceLayer, "Java exception in MessagesManager::HandlePresentMessagesRequest");
                    env->ExceptionDescribe();
                    env->ExceptionClear();
                    return CHIP_ERROR_INTERNAL;
                }
            }
        }

        env->CallBooleanMethod(mMessagesManagerObject.ObjectRef(), mPresentMessagesMethod, jIdentifier.jniValue(), jpriority,
                               jcontrol, jstartTime, jduration, jMessageText.jniValue(), joptions);
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

        UtfString jIdentifier(env, hex_buf);

        env->CallBooleanMethod(mMessagesManagerObject.ObjectRef(), mCancelMessagesMethod, jIdentifier.jniValue());
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
