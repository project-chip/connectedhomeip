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

#include "TvChannelManager.h"
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <jni.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/logging/CHIPLogging.h>

#include <map>
#include <string>
#include <vector>

using namespace chip;

TvChannelManager TvChannelManager::sInstance;

class ChannelInfoAttrAccess : public app::AttributeAccessInterface
{
public:
    ChannelInfoAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), app::Clusters::TvChannel::Id) {}
    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override
    {
        if (aPath.mAttributeId == app::Clusters::TvChannel::Attributes::TvChannelList::Id)
        {
            return TvChannelMgr().getTvChannelList(aEncoder);
        }
        else if (aPath.mAttributeId == app::Clusters::TvChannel::Attributes::TvChannelLineup::Id)
        {
            return TvChannelMgr().getTvChannelLineup(aEncoder);
        }
        else if (aPath.mAttributeId == app::Clusters::TvChannel::Attributes::CurrentTvChannel::Id)
        {
            return TvChannelMgr().getCurrentTvChannel(aEncoder);
        }

        return CHIP_NO_ERROR;
    }
};

ChannelInfoAttrAccess gTvChannelAttrAccess;

/** @brief Tv Channel  Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfTvChannelClusterInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gTvChannelAttrAccess);
        attrAccessRegistered = true;
    }
}

TvChannelInfo tvChannelClusterChangeChannel(std::string match)
{
    return TvChannelMgr().ChangeChannelByMatch(match);
}

bool tvChannelClusterChangeChannelByNumber(uint16_t majorNumber, uint16_t minorNumber)
{
    return TvChannelMgr().changeChannelByNumber(majorNumber, minorNumber);
}

bool tvChannelClusterSkipChannel(uint16_t count)
{
    return TvChannelMgr().skipChannnel(count);
}

void TvChannelManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for TvChannelManager"));

    mTvChannelManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturn(mTvChannelManagerObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef TvChannelManager"));

    jclass managerClass = env->GetObjectClass(mTvChannelManagerObject);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(Zcl, "Failed to get TvChannelManager Java class"));

    mGetChannelListMethod = env->GetMethodID(managerClass, "getChannelList", "()[Lcom/tcl/chip/tvapp/TvChannelInfo;");
    if (mGetChannelListMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access TvChannelManager 'getChannelList' method");
        env->ExceptionClear();
    }

    mGetLineupMethod = env->GetMethodID(managerClass, "getLineup", "()Lcom/tcl/chip/tvapp/TvChannelLineupInfo;");
    if (mGetLineupMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access TvChannelManager 'getLineup' method");
        env->ExceptionClear();
    }

    mGetCurrentChannelMethod = env->GetMethodID(managerClass, "getCurrentChannel", "()Lcom/tcl/chip/tvapp/TvChannelInfo;");
    if (mGetCurrentChannelMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access TvChannelManager 'getCurrentChannel' method");
        env->ExceptionClear();
    }

    mChangeChannelMethod =
        env->GetMethodID(managerClass, "changeChannel", "(Ljava/lang/String;)Lcom/tcl/chip/tvapp/TvChannelInfo;");
    if (mChangeChannelMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access TvChannelManager 'changeChannel' method");
        env->ExceptionClear();
    }

    mchangeChannelByNumberMethod = env->GetMethodID(managerClass, "changeChannelByNumber", "(II)Z");
    if (mchangeChannelByNumberMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access TvChannelManager 'changeChannelByNumber' method");
        env->ExceptionClear();
    }

    mskipChannelMethod = env->GetMethodID(managerClass, "skipChannel", "(I)Z");
    if (mskipChannelMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access TvChannelManager 'skipChannel' method");
        env->ExceptionClear();
    }
}

CHIP_ERROR TvChannelManager::getTvChannelList(chip::app::AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received TvChannelManager::getTvChannelList");
    VerifyOrExit(mTvChannelManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetChannelListMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    return aEncoder.EncodeList([env, this](const auto & encoder) -> CHIP_ERROR {
        jobjectArray channelInfoList = (jobjectArray) env->CallObjectMethod(mTvChannelManagerObject, mGetChannelListMethod);
        jint length                  = env->GetArrayLength(channelInfoList);
        for (jint i = 0; i < length; i++)
        {
            chip::app::Clusters::TvChannel::Structs::TvChannelInfo::Type channelInfo;
            jobject channelObject = env->GetObjectArrayElement(channelInfoList, i);
            jclass channelClass   = env->GetObjectClass(channelObject);

            jfieldID getCallSignField = env->GetFieldID(channelClass, "callSign", "Ljava/lang/String;");
            jstring jcallSign         = static_cast<jstring>(env->GetObjectField(channelObject, getCallSignField));
            if (jcallSign != NULL)
            {
                JniUtfString callsign(env, jcallSign);
                channelInfo.callSign = callsign.charSpan();
            }

            jfieldID getNameField = env->GetFieldID(channelClass, "name", "Ljava/lang/String;");
            jstring jname         = static_cast<jstring>(env->GetObjectField(channelObject, getNameField));
            if (jname != NULL)
            {
                JniUtfString name(env, jname);
                channelInfo.callSign = name.charSpan();
            }

            jfieldID getJaffiliateCallSignField = env->GetFieldID(channelClass, "affiliateCallSign", "Ljava/lang/String;");
            jstring jaffiliateCallSign = static_cast<jstring>(env->GetObjectField(channelObject, getJaffiliateCallSignField));
            if (jaffiliateCallSign != NULL)
            {
                JniUtfString affiliateCallSign(env, jaffiliateCallSign);
                channelInfo.callSign = affiliateCallSign.charSpan();
            }

            jfieldID majorNumField  = env->GetFieldID(channelClass, "majorNumber", "I");
            jint jmajorNum          = env->GetIntField(channelObject, majorNumField);
            channelInfo.majorNumber = static_cast<uint16_t>(jmajorNum);

            jfieldID minorNumField  = env->GetFieldID(channelClass, "minorNumber", "I");
            jint jminorNum          = env->GetIntField(channelObject, minorNumField);
            channelInfo.majorNumber = static_cast<uint16_t>(jminorNum);
            ReturnErrorOnFailure(encoder.Encode(channelInfo));
        }
        return CHIP_NO_ERROR;
    });
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TvChannelManager::getTVChannelList status error: %s", err.AsString());
    }

    return err;
}

CHIP_ERROR TvChannelManager::getTvChannelLineup(chip::app::AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received TvChannelManager::getTvChannelLineup");
    VerifyOrExit(mTvChannelManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetLineupMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        chip::app::Clusters::TvChannel::Structs::TvChannelLineupInfo::Type channelLineupInfo;

        jobject channelLineupObject = env->CallObjectMethod(mTvChannelManagerObject, mGetLineupMethod);
        jclass channelLineupClazz   = env->GetObjectClass(channelLineupObject);

        jfieldID operatorNameFild = env->GetFieldID(channelLineupClazz, "operatorName", "Ljava/lang/String;");
        jstring joperatorName     = static_cast<jstring>(env->GetObjectField(channelLineupObject, operatorNameFild));
        if (joperatorName != NULL)
        {
            JniUtfString operatorName(env, joperatorName);
            channelLineupInfo.operatorName = operatorName.charSpan();
        }

        jfieldID lineupNameFild = env->GetFieldID(channelLineupClazz, "lineupName", "Ljava/lang/String;");
        jstring jlineupName     = static_cast<jstring>(env->GetObjectField(channelLineupObject, lineupNameFild));
        if (jlineupName != NULL)
        {
            JniUtfString lineupName(env, jlineupName);
            channelLineupInfo.lineupName = lineupName.charSpan();
        }

        jfieldID postalCodeFild = env->GetFieldID(channelLineupClazz, "postalCode", "Ljava/lang/String;");
        jstring jpostalCode     = static_cast<jstring>(env->GetObjectField(channelLineupObject, postalCodeFild));
        if (jpostalCode != NULL)
        {
            JniUtfString postalCode(env, jpostalCode);
            channelLineupInfo.postalCode = postalCode.charSpan();
        }

        jfieldID lineupInfoTypeFild      = env->GetFieldID(channelLineupClazz, "lineupInfoType", "I");
        jint jlineupInfoType             = (env->GetIntField(channelLineupObject, lineupInfoTypeFild));
        channelLineupInfo.lineupInfoType = static_cast<app::Clusters::TvChannel::TvChannelLineupInfoType>(jlineupInfoType);

        ReturnErrorOnFailure(aEncoder.Encode(channelLineupInfo));

        return CHIP_NO_ERROR;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TvChannelManager::getTvChannelLineup status error: %s", err.AsString());
    }

    return err;
}

CHIP_ERROR TvChannelManager::getCurrentTvChannel(chip::app::AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    ChipLogProgress(Zcl, "Received TvChannelManager::getCurrentTvChannel");
    VerifyOrExit(mTvChannelManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetCurrentChannelMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    {
        chip::app::Clusters::TvChannel::Structs::TvChannelInfo::Type channelInfo;

        jobject channelInfoObject = env->CallObjectMethod(mTvChannelManagerObject, mGetCurrentChannelMethod);
        jclass channelClass       = env->GetObjectClass(channelInfoObject);

        jfieldID getCallSignField = env->GetFieldID(channelClass, "callSign", "Ljava/lang/String;");
        jstring jcallSign         = static_cast<jstring>(env->GetObjectField(channelInfoObject, getCallSignField));
        if (jcallSign != NULL)
        {
            JniUtfString callsign(env, jcallSign);
            channelInfo.callSign = callsign.charSpan();
        }

        jfieldID getNameField = env->GetFieldID(channelClass, "name", "Ljava/lang/String;");
        jstring jname         = static_cast<jstring>(env->GetObjectField(channelInfoObject, getNameField));
        if (jname != NULL)
        {
            JniUtfString name(env, jname);
            channelInfo.callSign = name.charSpan();
        }

        jfieldID getJaffiliateCallSignField = env->GetFieldID(channelClass, "affiliateCallSign", "Ljava/lang/String;");
        jstring jaffiliateCallSign = static_cast<jstring>(env->GetObjectField(channelInfoObject, getJaffiliateCallSignField));
        if (jaffiliateCallSign != NULL)
        {
            JniUtfString affiliateCallSign(env, jaffiliateCallSign);
            channelInfo.callSign = affiliateCallSign.charSpan();
        }

        jfieldID majorNumField  = env->GetFieldID(channelClass, "majorNumber", "I");
        jint jmajorNum          = env->GetIntField(channelInfoObject, majorNumField);
        channelInfo.majorNumber = static_cast<uint16_t>(jmajorNum);

        jfieldID minorNumField  = env->GetFieldID(channelClass, "minorNumber", "I");
        jint jminorNum          = env->GetIntField(channelInfoObject, minorNumField);
        channelInfo.majorNumber = static_cast<uint16_t>(jminorNum);

        ReturnErrorOnFailure(aEncoder.Encode(channelInfo));
        return CHIP_NO_ERROR;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TvChannelManager::getTVChannel status error: %s", err.AsString());
    }

    return err;
}

TvChannelInfo TvChannelManager::ChangeChannelByMatch(std::string name)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    TvChannelInfo channelInfo{ 0, 0 };

    ChipLogProgress(Zcl, "Received TvChannelManager::ChangeChannelByMatch name %s", name.c_str());
    VerifyOrExit(mTvChannelManagerObject != nullptr, ChipLogError(Zcl, "mTvChannelManagerObject null"));
    VerifyOrExit(mChangeChannelMethod != nullptr, ChipLogError(Zcl, "mChangeChannelMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    {
        UtfString jniname(env, name.c_str());
        env->ExceptionClear();
        jobject channelObject = env->CallObjectMethod(mTvChannelManagerObject, mChangeChannelMethod, jniname.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in TvChannelManager::ChangeChannelByMatch");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return channelInfo;
        }

        jclass channelClass = env->GetObjectClass(channelObject);

        jfieldID getCallSignField = env->GetFieldID(channelClass, "callSign", "Ljava/lang/String;");
        jstring jcallSign         = static_cast<jstring>(env->GetObjectField(channelObject, getCallSignField));
        if (jcallSign != NULL)
        {
            JniUtfString callsign(env, jcallSign);
            channelInfo.callSign = callsign.charSpan();
        }

        jfieldID getNameField = env->GetFieldID(channelClass, "name", "Ljava/lang/String;");
        jstring jname         = static_cast<jstring>(env->GetObjectField(channelObject, getNameField));
        if (jname != NULL)
        {
            JniUtfString junitname(env, jname);
            channelInfo.callSign = junitname.charSpan();
        }
        jfieldID getJaffiliateCallSignField = env->GetFieldID(channelClass, "affiliateCallSign", "Ljava/lang/String;");
        jstring jaffiliateCallSign          = static_cast<jstring>(env->GetObjectField(channelObject, getJaffiliateCallSignField));
        if (jaffiliateCallSign != NULL)
        {
            JniUtfString affiliateCallSign(env, jaffiliateCallSign);
            channelInfo.callSign = affiliateCallSign.charSpan();
        }

        jfieldID majorNumField  = env->GetFieldID(channelClass, "majorNumber", "I");
        jint jmajorNum          = env->GetIntField(channelObject, majorNumField);
        channelInfo.majorNumber = static_cast<uint16_t>(jmajorNum);

        jfieldID minorNumField  = env->GetFieldID(channelClass, "minorNumber", "I");
        jint jminorNum          = env->GetIntField(channelObject, minorNumField);
        channelInfo.majorNumber = static_cast<uint16_t>(jminorNum);
    }

exit:
    return channelInfo;
}

bool TvChannelManager::changeChannelByNumber(uint16_t majorNumber, uint16_t minorNumber)
{
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received TvChannelManager::tvChannelClusterChangeChannelByNumber majorNumber %d, minorNumber %d",
                    majorNumber, minorNumber);
    VerifyOrExit(mTvChannelManagerObject != nullptr, ChipLogError(Zcl, "mTvChannelManagerObject null"));
    VerifyOrExit(mchangeChannelByNumberMethod != nullptr, ChipLogError(Zcl, "mchangeChannelByNumberMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    env->ExceptionClear();

    ret = env->CallBooleanMethod(mTvChannelManagerObject, mchangeChannelByNumberMethod, static_cast<jint>(majorNumber),
                                 static_cast<jint>(minorNumber));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in TvChannelManager::changeChannelByNumber");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}

bool TvChannelManager::skipChannnel(uint16_t count)
{
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received TvChannelManager::skipChannnel count %d", count);
    VerifyOrExit(mTvChannelManagerObject != nullptr, ChipLogError(Zcl, "mTvChannelManagerObject null"));
    VerifyOrExit(mskipChannelMethod != nullptr, ChipLogError(Zcl, "mskipChannelMethod null"));
    VerifyOrExit(env != NULL, ChipLogError(Zcl, "env null"));

    env->ExceptionClear();

    ret = env->CallBooleanMethod(mTvChannelManagerObject, mskipChannelMethod, static_cast<jint>(count));
    if (env->ExceptionCheck())
    {
        ChipLogError(DeviceLayer, "Java exception in TvChannelManager::SkipChannel");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }

exit:
    return static_cast<bool>(ret);
}
