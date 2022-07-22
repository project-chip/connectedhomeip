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

#include "ChannelManager.h"
#include "TvApp-JNI.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <cstdlib>
#include <jni.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Channel;
using namespace chip::Uint8;

/** @brief Channel  Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfChannelClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Android App: Channel::PostClusterInit");
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::Channel::Id, endpoint);
}

void ChannelManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "TV Android App: Channel::SetDefaultDelegate");
    ChannelManager * mgr = new ChannelManager();
    mgr->InitializeWithObjects(manager);
    chip::app::Clusters::Channel::SetDefaultDelegate(static_cast<EndpointId>(endpoint), mgr);
}

CHIP_ERROR ChannelManager::HandleGetChannelList(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received ChannelManager::HandleGetChannelList");
    VerifyOrExit(mChannelManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetChannelListMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jobjectArray channelInfoList = (jobjectArray) env->CallObjectMethod(mChannelManagerObject, mGetChannelListMethod);
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ChannelManager::HandleGetChannelList");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }

        jint length = env->GetArrayLength(channelInfoList);

        for (jint i = 0; i < length; i++)
        {
            chip::app::Clusters::Channel::Structs::ChannelInfo::Type channelInfo;
            jobject channelObject = env->GetObjectArrayElement(channelInfoList, i);
            jclass channelClass   = env->GetObjectClass(channelObject);

            jfieldID getCallSignField = env->GetFieldID(channelClass, "callSign", "Ljava/lang/String;");
            jstring jcallSign         = static_cast<jstring>(env->GetObjectField(channelObject, getCallSignField));
            JniUtfString callsign(env, jcallSign);
            if (jcallSign != nullptr)
            {
                channelInfo.callSign = Optional<CharSpan>(callsign.charSpan());
            }

            jfieldID getNameField = env->GetFieldID(channelClass, "name", "Ljava/lang/String;");
            jstring jname         = static_cast<jstring>(env->GetObjectField(channelObject, getNameField));
            JniUtfString name(env, jname);
            if (jname != nullptr)
            {
                channelInfo.name = Optional<CharSpan>(name.charSpan());
            }

            jfieldID getJaffiliateCallSignField = env->GetFieldID(channelClass, "affiliateCallSign", "Ljava/lang/String;");
            jstring jaffiliateCallSign = static_cast<jstring>(env->GetObjectField(channelObject, getJaffiliateCallSignField));
            JniUtfString affiliateCallSign(env, jaffiliateCallSign);
            if (jaffiliateCallSign != nullptr)
            {
                channelInfo.affiliateCallSign = Optional<CharSpan>(affiliateCallSign.charSpan());
            }

            jfieldID majorNumField  = env->GetFieldID(channelClass, "majorNumber", "I");
            jint jmajorNum          = env->GetIntField(channelObject, majorNumField);
            channelInfo.majorNumber = static_cast<uint16_t>(jmajorNum);

            jfieldID minorNumField  = env->GetFieldID(channelClass, "minorNumber", "I");
            jint jminorNum          = env->GetIntField(channelObject, minorNumField);
            channelInfo.minorNumber = static_cast<uint16_t>(jminorNum);

            ReturnErrorOnFailure(encoder.Encode(channelInfo));
        }

        return CHIP_NO_ERROR;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ChannelManager::getChannelList status error: %s", err.AsString());
    }

    return err;
}

CHIP_ERROR ChannelManager::HandleGetLineup(AttributeValueEncoder & aEncoder)
{
    chip::app::Clusters::Channel::Structs::LineupInfo::Type lineupInfo;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received ChannelManager::HandleGetLineup");
    VerifyOrExit(mChannelManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetLineupMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        jobject channelLineupObject = env->CallObjectMethod(mChannelManagerObject, mGetLineupMethod);
        if (channelLineupObject != nullptr)
        {
            jclass channelLineupClazz = env->GetObjectClass(channelLineupObject);

            jfieldID operatorNameFild = env->GetFieldID(channelLineupClazz, "operatorName", "Ljava/lang/String;");
            jstring joperatorName     = static_cast<jstring>(env->GetObjectField(channelLineupObject, operatorNameFild));
            JniUtfString operatorName(env, joperatorName);
            if (joperatorName != nullptr)
            {
                lineupInfo.operatorName = operatorName.charSpan();
            }

            jfieldID lineupNameFild = env->GetFieldID(channelLineupClazz, "lineupName", "Ljava/lang/String;");
            jstring jlineupName     = static_cast<jstring>(env->GetObjectField(channelLineupObject, lineupNameFild));
            JniUtfString lineupName(env, jlineupName);
            if (jlineupName != nullptr)
            {
                lineupInfo.lineupName = Optional<CharSpan>(lineupName.charSpan());
            }

            jfieldID postalCodeFild = env->GetFieldID(channelLineupClazz, "postalCode", "Ljava/lang/String;");
            jstring jpostalCode     = static_cast<jstring>(env->GetObjectField(channelLineupObject, postalCodeFild));
            JniUtfString postalCode(env, jpostalCode);
            if (jpostalCode != nullptr)
            {
                lineupInfo.postalCode = Optional<CharSpan>(postalCode.charSpan());
            }

            jfieldID lineupInfoTypeFild = env->GetFieldID(channelLineupClazz, "lineupInfoType", "I");
            jint jlineupInfoType        = (env->GetIntField(channelLineupObject, lineupInfoTypeFild));
            lineupInfo.lineupInfoType   = static_cast<app::Clusters::Channel::LineupInfoTypeEnum>(jlineupInfoType);

            err = aEncoder.Encode(lineupInfo);
        }
        else
        {
            err = aEncoder.EncodeNull();
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ChannelManager::getChannelLineup status error: %s", err.AsString());
    }

    return err;
}

CHIP_ERROR ChannelManager::HandleGetCurrentChannel(AttributeValueEncoder & aEncoder)
{
    chip::app::Clusters::Channel::Structs::ChannelInfo::Type channelInfo;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    ChipLogProgress(Zcl, "Received ChannelManager::HandleGetCurrentChannel");
    VerifyOrExit(mChannelManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetCurrentChannelMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        jobject channelInfoObject = env->CallObjectMethod(mChannelManagerObject, mGetCurrentChannelMethod);
        if (channelInfoObject != nullptr)
        {
            jclass channelClass = env->GetObjectClass(channelInfoObject);

            jfieldID getCallSignField = env->GetFieldID(channelClass, "callSign", "Ljava/lang/String;");
            jstring jcallSign         = static_cast<jstring>(env->GetObjectField(channelInfoObject, getCallSignField));
            JniUtfString callsign(env, jcallSign);
            if (jcallSign != nullptr)
            {
                channelInfo.callSign = Optional<CharSpan>(callsign.charSpan());
            }

            jfieldID getNameField = env->GetFieldID(channelClass, "name", "Ljava/lang/String;");
            jstring jname         = static_cast<jstring>(env->GetObjectField(channelInfoObject, getNameField));
            JniUtfString name(env, jname);
            if (jname != nullptr)
            {
                channelInfo.name = Optional<CharSpan>(name.charSpan());
            }

            jfieldID getJaffiliateCallSignField = env->GetFieldID(channelClass, "affiliateCallSign", "Ljava/lang/String;");
            jstring jaffiliateCallSign = static_cast<jstring>(env->GetObjectField(channelInfoObject, getJaffiliateCallSignField));
            JniUtfString affiliateCallSign(env, jaffiliateCallSign);
            if (jaffiliateCallSign != nullptr)
            {
                channelInfo.affiliateCallSign = Optional<CharSpan>(affiliateCallSign.charSpan());
            }

            jfieldID majorNumField  = env->GetFieldID(channelClass, "majorNumber", "I");
            jint jmajorNum          = env->GetIntField(channelInfoObject, majorNumField);
            channelInfo.majorNumber = static_cast<uint16_t>(jmajorNum);

            jfieldID minorNumField  = env->GetFieldID(channelClass, "minorNumber", "I");
            jint jminorNum          = env->GetIntField(channelInfoObject, minorNumField);
            channelInfo.minorNumber = static_cast<uint16_t>(jminorNum);

            err = aEncoder.Encode(channelInfo);
        }
        else
        {
            err = aEncoder.EncodeNull();
            return err;
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ChannelManager::HandleGetCurrentChannel status error: %s", err.AsString());
    }

    return err;
}

void ChannelManager::HandleChangeChannel(CommandResponseHelper<ChangeChannelResponseType> & helper, const CharSpan & match)
{
    std::string name(match.data(), match.size());
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received ChannelManager::HandleChangeChannel name %s", name.c_str());
    VerifyOrExit(mChannelManagerObject != nullptr, ChipLogError(Zcl, "mChannelManagerObject null"));
    VerifyOrExit(mChangeChannelMethod != nullptr, ChipLogError(Zcl, "mChangeChannelMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    {
        UtfString jniname(env, name.c_str());
        env->ExceptionClear();
        jobject channelObject = env->CallObjectMethod(mChannelManagerObject, mChangeChannelMethod, jniname.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in ChannelManager::HandleChangeChannel");
            env->ExceptionDescribe();
            env->ExceptionClear();
            goto exit;
        }

        jclass channelClass = env->GetObjectClass(channelObject);

        ChangeChannelResponseType response;

        jfieldID getStatusField = env->GetFieldID(channelClass, "status", "I");
        jint jstatus            = env->GetIntField(channelObject, getStatusField);
        response.status         = static_cast<app::Clusters::Channel::ChannelStatusEnum>(jstatus);

        jfieldID getNameField = env->GetFieldID(channelClass, "name", "Ljava/lang/String;");
        jstring jname         = static_cast<jstring>(env->GetObjectField(channelObject, getNameField));
        JniUtfString junitname(env, jname);
        if (jname != NULL)
        {
            response.data = MakeOptional(junitname.charSpan());
        }

        helper.Success(response);
    }

exit:
    return;
}

bool ChannelManager::HandleChangeChannelByNumber(const uint16_t & majorNumber, const uint16_t & minorNumber)
{
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received ChannelManager::HandleChangeChannelByNumber majorNumber %d, minorNumber %d", majorNumber,
                    minorNumber);
    VerifyOrExit(mChannelManagerObject != nullptr, ChipLogError(Zcl, "mChannelManagerObject null"));
    VerifyOrExit(mChangeChannelByNumberMethod != nullptr, ChipLogError(Zcl, "mChangeChannelByNumberMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    env->ExceptionClear();

    ret = env->CallBooleanMethod(mChannelManagerObject, mChangeChannelByNumberMethod, static_cast<jint>(majorNumber),
                                 static_cast<jint>(minorNumber));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in ChannelManager::changeChannelByNumber");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}

bool ChannelManager::HandleSkipChannel(const uint16_t & count)
{
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received ChannelManager::HandleSkipChannel count %d", count);
    VerifyOrExit(mChannelManagerObject != nullptr, ChipLogError(Zcl, "mChannelManagerObject null"));
    VerifyOrExit(mSkipChannelMethod != nullptr, ChipLogError(Zcl, "mSkipChannelMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    env->ExceptionClear();

    ret = env->CallBooleanMethod(mChannelManagerObject, mSkipChannelMethod, static_cast<jint>(count));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in ChannelManager::HandleSkipChannel");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}

void ChannelManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for ChannelManager"));

    mChannelManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturn(mChannelManagerObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef ChannelManager"));

    jclass managerClass = env->GetObjectClass(mChannelManagerObject);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(Zcl, "Failed to get ChannelManager Java class"));

    mGetChannelListMethod = env->GetMethodID(managerClass, "getChannelList", "()[Lcom/matter/tv/server/tvapp/ChannelInfo;");
    if (mGetChannelListMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'getChannelList' method");
        env->ExceptionClear();
    }

    mGetLineupMethod = env->GetMethodID(managerClass, "getLineup", "()Lcom/matter/tv/server/tvapp/ChannelLineupInfo;");
    if (mGetLineupMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'getLineup' method");
        env->ExceptionClear();
    }

    mGetCurrentChannelMethod = env->GetMethodID(managerClass, "getCurrentChannel", "()Lcom/matter/tv/server/tvapp/ChannelInfo;");
    if (mGetCurrentChannelMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'getCurrentChannel' method");
        env->ExceptionClear();
    }

    mChangeChannelMethod =
        env->GetMethodID(managerClass, "changeChannel", "(Ljava/lang/String;)Lcom/matter/tv/server/tvapp/ChannelInfo;");
    if (mChangeChannelMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'changeChannel' method");
        env->ExceptionClear();
    }

    mChangeChannelByNumberMethod = env->GetMethodID(managerClass, "changeChannelByNumber", "(II)Z");
    if (mChangeChannelByNumberMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'changeChannelByNumber' method");
        env->ExceptionClear();
    }

    mSkipChannelMethod = env->GetMethodID(managerClass, "skipChannel", "(I)Z");
    if (mSkipChannelMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'skipChannel' method");
        env->ExceptionClear();
    }
}

uint32_t ChannelManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= EMBER_AF_CONTENT_LAUNCH_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return mDynamicEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
