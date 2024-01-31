/*
 *
 *    Copyright (c) 2020-24 Project CHIP Authors
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

#include "MatterCommandObjects-JNI.h"

namespace matter {
namespace casting {
namespace clusters {

CHIP_ERROR ContentLauncherClusterLaunchURLCommand::GetCppRequestFromJava(
    jobject inRequest, chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type & outRequest)
{
    VerifyOrReturnValue(inRequest != nullptr, CHIP_NO_ERROR);

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass jRequestClass;
    ReturnErrorOnFailure(chip::JniReferences::GetInstance().GetClassRef(
        env, "com/matter/casting/clusters/MatterCommands$ContentLauncherClusterLaunchURLRequest", jRequestClass));

    jfieldID jContentURLField = env->GetFieldID(jRequestClass, "contentURL", "Ljava/lang/String;");
    jstring jContentURLObj    = (jstring) env->GetObjectField(inRequest, jContentURLField);
    VerifyOrReturnError(jContentURLObj != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    outRequest.contentURL = chip::CharSpan::fromCharString(env->GetStringUTFChars(jContentURLObj, 0));

    jfieldID jDisplayStringField = env->GetFieldID(jRequestClass, "displayString", "Ljava/lang/String;");
    jstring jDisplayStringObj    = (jstring) env->GetObjectField(inRequest, jDisplayStringField);
    if (jDisplayStringObj != nullptr)
    {
        const char * nativeValue = env->GetStringUTFChars(jDisplayStringObj, 0);
        outRequest.displayString = chip::Optional<chip::CharSpan>(chip::CharSpan::fromCharString(nativeValue));
    }
    else
    {
        outRequest.displayString = chip::NullOptional;
    }

    // TODO: translate brandingInformation
    outRequest.brandingInformation =
        chip::MakeOptional(chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type());
    return CHIP_NO_ERROR;
}

jobject ContentLauncherClusterLaunchURLCommand::GetJResponseFromCpp(
    const chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type::ResponseType * response)
{
    VerifyOrReturnValue(response != nullptr, nullptr);
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass responseTypeClass = nullptr;
    VerifyOrReturnValue(
        chip::JniReferences::GetInstance().GetClassRef(
            env, "com/matter/casting/clusters/MatterCommands$ContentLauncherClusterResponse", responseTypeClass) == CHIP_NO_ERROR,
        nullptr, ChipLogError(AppServer, "convertResponseFromCppToJava could not get ContentLauncherClusterResponse class ref"));

    jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "()V");
    jobject jResponseObj  = env->NewObject(responseTypeClass, constructor);

    if (response->data.HasValue())
    {
        jfieldID dataField = env->GetFieldID(responseTypeClass, "data", "Ljava/lang/String;");
        char * buffer      = new char[response->data.Value().size() + 1];
        strncpy(buffer, response->data.Value().data(), response->data.Value().size());
        buffer[response->data.Value().size()] = '\0';
        env->SetObjectField(jResponseObj, dataField, env->NewStringUTF(buffer));
        delete[] buffer;
    }

    jclass integerClass  = env->FindClass("java/lang/Integer");
    jmethodID valueOf    = env->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;");
    jobject statusObj    = env->CallStaticObjectMethod(integerClass, valueOf, static_cast<jint>(response->status));
    jfieldID statusField = env->GetFieldID(responseTypeClass, "status", "Ljava/lang/Integer;");
    env->SetObjectField(jResponseObj, statusField, statusObj);

    return jResponseObj;
}

}; // namespace clusters
}; // namespace casting
}; // namespace matter
