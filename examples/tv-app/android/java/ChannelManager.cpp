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
#include <app/util/config.h>
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
    DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ChannelManager::HandleGetChannelList");
    VerifyOrExit(mChannelManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetChannelListMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    return aEncoder.EncodeList([this, env](const auto & encoder) -> CHIP_ERROR {
        jobjectArray channelInfoList =
            (jobjectArray) env->CallObjectMethod(mChannelManagerObject.ObjectRef(), mGetChannelListMethod);
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
            chip::app::Clusters::Channel::Structs::ChannelInfoStruct::Type channelInfo;
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
    DeviceLayer::StackUnlock unlock;
    chip::app::Clusters::Channel::Structs::LineupInfoStruct::Type lineupInfo;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ChannelManager::HandleGetLineup");
    VerifyOrExit(mChannelManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetLineupMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    {
        jobject channelLineupObject = env->CallObjectMethod(mChannelManagerObject.ObjectRef(), mGetLineupMethod);
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
    DeviceLayer::StackUnlock unlock;
    chip::app::Clusters::Channel::Structs::ChannelInfoStruct::Type channelInfo;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ChannelManager::HandleGetCurrentChannel");
    VerifyOrExit(mChannelManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetCurrentChannelMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    {
        jobject channelInfoObject = env->CallObjectMethod(mChannelManagerObject.ObjectRef(), mGetCurrentChannelMethod);
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

            jfieldID getJidentifierField = env->GetFieldID(channelClass, "identifier", "Ljava/lang/String;");
            jstring jidentifier          = static_cast<jstring>(env->GetObjectField(channelInfoObject, getJidentifierField));
            JniUtfString identifier(env, jidentifier);
            if (jidentifier != nullptr)
            {
                channelInfo.identifier = Optional<CharSpan>(identifier.charSpan());
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
    DeviceLayer::StackUnlock unlock;
    std::string name(match.data(), match.size());
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ChannelManager::HandleChangeChannel name %s", name.c_str());
    VerifyOrExit(mChannelManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mChannelManagerObject null"));
    VerifyOrExit(mChangeChannelMethod != nullptr, ChipLogError(Zcl, "mChangeChannelMethod null"));

    env->ExceptionClear();

    {
        UtfString jniname(env, name.c_str());
        jobject channelObject = env->CallObjectMethod(mChannelManagerObject.ObjectRef(), mChangeChannelMethod, jniname.jniValue());
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
        response.status         = static_cast<app::Clusters::Channel::StatusEnum>(jstatus);

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
    DeviceLayer::StackUnlock unlock;
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ChannelManager::HandleChangeChannelByNumber majorNumber %d, minorNumber %d", majorNumber,
                    minorNumber);
    VerifyOrExit(mChannelManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mChannelManagerObject null"));
    VerifyOrExit(mChangeChannelByNumberMethod != nullptr, ChipLogError(Zcl, "mChangeChannelByNumberMethod null"));

    env->ExceptionClear();

    ret = env->CallBooleanMethod(mChannelManagerObject.ObjectRef(), mChangeChannelByNumberMethod, static_cast<jint>(majorNumber),
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

bool ChannelManager::HandleSkipChannel(const int16_t & count)
{
    DeviceLayer::StackUnlock unlock;
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ChannelManager::HandleSkipChannel count %d", count);
    VerifyOrExit(mChannelManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mChannelManagerObject null"));
    VerifyOrExit(mSkipChannelMethod != nullptr, ChipLogError(Zcl, "mSkipChannelMethod null"));

    env->ExceptionClear();

    ret = env->CallBooleanMethod(mChannelManagerObject.ObjectRef(), mSkipChannelMethod, static_cast<jint>(count));
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

void ChannelManager::HandleGetProgramGuide(
    CommandResponseHelper<ProgramGuideResponseType> & helper, const chip::Optional<uint32_t> & startTime,
    const chip::Optional<uint32_t> & endTime,
    const chip::Optional<chip::app::DataModel::DecodableList<ChannelInfoType>> & channelList,
    const chip::Optional<PageTokenType> & pageToken, const chip::Optional<chip::BitMask<RecordingFlagBitmap>> & recordingFlag,
    const chip::Optional<chip::app::DataModel::DecodableList<AdditionalInfoType>> & externalIdList,
    const chip::Optional<chip::ByteSpan> & data)
{
    DeviceLayer::StackUnlock unlock;
    ProgramGuideResponseType response;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    std::vector<ProgramType *> needToFreePrograms;
    std::vector<ChannelInfoType *> needToFreeChannels;
    std::vector<ProgramType> tempPrograms;
    std::vector<JniUtfString *> needToFreeStrings;

    ChipLogProgress(Zcl, "Received ChannelManager::HandleGetProgramGuide");
    VerifyOrExit(mChannelManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetProgramGuideMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    {
        // NOTE: this example app does not pass the Data, PageToken, ChannelsArray or ExternalIdList through to the Java layer
        UtfString jData(env, "");
        UtfString jToken(env, "");
        jobjectArray channelsArray       = nullptr;
        jobjectArray externalIDListArray = (jobjectArray) env->NewObjectArray(0, env->FindClass("java/util/Map$Entry"), NULL);

        jobject resp = env->CallObjectMethod(
            mChannelManagerObject.ObjectRef(), mGetProgramGuideMethod, static_cast<jlong>(startTime.ValueOr(0)),
            static_cast<jlong>(endTime.ValueOr(0)), channelsArray, jToken.jniValue(),
            static_cast<jboolean>(recordingFlag.ValueOr(0).Raw() != 0), externalIDListArray, jData.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ChannelManager::HandleGetProgramGuide");
            env->ExceptionDescribe();
            env->ExceptionClear();
            err = CHIP_ERROR_INCORRECT_STATE;
            goto exit;
        }

        VerifyOrExit(resp != nullptr, err = CHIP_JNI_ERROR_NULL_OBJECT);
        jclass respCls       = env->GetObjectClass(resp);
        jfieldID programsFid = env->GetFieldID(respCls, "programs", "[Lcom/matter/tv/server/tvapp/ChannelProgramInfo;");
        VerifyOrExit(programsFid != nullptr, err = CHIP_JNI_ERROR_FIELD_NOT_FOUND);

        jobjectArray programsArray = (jobjectArray) env->GetObjectField(resp, programsFid);
        jint size                  = env->GetArrayLength(programsArray);
        for (jint i = 0; i < size; i++)
        {
            ProgramType * program = new ProgramType();
            VerifyOrExit(program != nullptr, err = CHIP_ERROR_NO_MEMORY);

            jobject programObject = env->GetObjectArrayElement(programsArray, i);
            jclass programClass   = env->GetObjectClass(programObject);

            jfieldID getIdentifierField = env->GetFieldID(programClass, "identifier", "Ljava/lang/String;");
            jstring jidentifier         = static_cast<jstring>(env->GetObjectField(programObject, getIdentifierField));
            JniUtfString * identifier   = new JniUtfString(env, jidentifier);
            VerifyOrExit(identifier != nullptr, err = CHIP_ERROR_NO_MEMORY);
            needToFreeStrings.push_back(identifier);
            if (jidentifier != nullptr)
            {
                program->identifier = identifier->charSpan();
            }

            jfieldID getChannelField  = env->GetFieldID(programClass, "channel", "Lcom/matter/tv/server/tvapp/ChannelInfo;");
            jobject channelInfoObject = env->GetObjectField(programObject, getChannelField);
            if (channelInfoObject != nullptr)
            {
                jclass channelClass = env->GetObjectClass(channelInfoObject);

                ChannelInfoType * channelInfo = new ChannelInfoType();
                VerifyOrExit(channelInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

                jfieldID getCallSignField = env->GetFieldID(channelClass, "callSign", "Ljava/lang/String;");
                jstring jcallSign         = static_cast<jstring>(env->GetObjectField(channelInfoObject, getCallSignField));
                JniUtfString * callsign   = new JniUtfString(env, jcallSign);
                VerifyOrExit(callsign != nullptr, err = CHIP_ERROR_NO_MEMORY);
                needToFreeStrings.push_back(callsign);
                if (jcallSign != nullptr)
                {
                    channelInfo->callSign = Optional<CharSpan>(callsign->charSpan());
                }

                jfieldID getNameField = env->GetFieldID(channelClass, "name", "Ljava/lang/String;");
                jstring jname         = static_cast<jstring>(env->GetObjectField(channelInfoObject, getNameField));
                JniUtfString * name   = new JniUtfString(env, jname);
                VerifyOrExit(name != nullptr, err = CHIP_ERROR_NO_MEMORY);
                needToFreeStrings.push_back(name);
                if (jname != nullptr)
                {
                    channelInfo->name = Optional<CharSpan>(name->charSpan());
                }

                jfieldID getJaffiliateCallSignField = env->GetFieldID(channelClass, "affiliateCallSign", "Ljava/lang/String;");
                jstring jaffiliateCallSign =
                    static_cast<jstring>(env->GetObjectField(channelInfoObject, getJaffiliateCallSignField));
                JniUtfString * affiliateCallSign = new JniUtfString(env, jaffiliateCallSign);
                VerifyOrExit(affiliateCallSign != nullptr, err = CHIP_ERROR_NO_MEMORY);
                needToFreeStrings.push_back(affiliateCallSign);
                if (jaffiliateCallSign != nullptr)
                {
                    channelInfo->affiliateCallSign = Optional<CharSpan>(affiliateCallSign->charSpan());
                }

                jfieldID getJchanIdentifierField = env->GetFieldID(channelClass, "identifier", "Ljava/lang/String;");
                jstring jchanidentifier = static_cast<jstring>(env->GetObjectField(channelInfoObject, getJchanIdentifierField));
                JniUtfString * chanidentifier = new JniUtfString(env, jchanidentifier);
                VerifyOrExit(chanidentifier != nullptr, err = CHIP_ERROR_NO_MEMORY);
                needToFreeStrings.push_back(chanidentifier);
                if (jchanidentifier != nullptr)
                {
                    channelInfo->identifier = Optional<CharSpan>(chanidentifier->charSpan());
                }

                jfieldID majorNumField   = env->GetFieldID(channelClass, "majorNumber", "I");
                jint jmajorNum           = env->GetIntField(channelInfoObject, majorNumField);
                channelInfo->majorNumber = static_cast<uint16_t>(jmajorNum);

                jfieldID minorNumField   = env->GetFieldID(channelClass, "minorNumber", "I");
                jint jminorNum           = env->GetIntField(channelInfoObject, minorNumField);
                channelInfo->minorNumber = static_cast<uint16_t>(jminorNum);

                program->channel = *channelInfo;
                needToFreeChannels.push_back(channelInfo);
            }

            jfieldID getTitleField = env->GetFieldID(programClass, "title", "Ljava/lang/String;");
            jstring jtitle         = static_cast<jstring>(env->GetObjectField(programObject, getTitleField));
            JniUtfString * title   = new JniUtfString(env, jtitle);
            VerifyOrExit(title != nullptr, err = CHIP_ERROR_NO_MEMORY);
            needToFreeStrings.push_back(title);
            if (jtitle != nullptr)
            {
                program->title = title->charSpan();
            }

            jfieldID getSubTitleField = env->GetFieldID(programClass, "subTitle", "Ljava/lang/String;");
            jstring jsubTitle         = static_cast<jstring>(env->GetObjectField(programObject, getSubTitleField));
            JniUtfString * subTitle   = new JniUtfString(env, jsubTitle);
            VerifyOrExit(subTitle != nullptr, err = CHIP_ERROR_NO_MEMORY);
            needToFreeStrings.push_back(subTitle);
            if (jsubTitle != nullptr)
            {
                program->subtitle = MakeOptional(subTitle->charSpan());
            }

            jfieldID getDescriptionField = env->GetFieldID(programClass, "description", "Ljava/lang/String;");
            jstring jdescription         = static_cast<jstring>(env->GetObjectField(programObject, getDescriptionField));
            JniUtfString * description   = new JniUtfString(env, jdescription);
            VerifyOrExit(description != nullptr, err = CHIP_ERROR_NO_MEMORY);
            needToFreeStrings.push_back(description);
            if (jdescription != nullptr)
            {
                program->description = Optional<CharSpan>(description->charSpan());
            }

            jfieldID startTimeField = env->GetFieldID(programClass, "startTime", "J");
            jlong jstartTime        = env->GetLongField(programObject, startTimeField);
            program->startTime      = static_cast<uint32_t>(jstartTime);

            jfieldID endTimeField = env->GetFieldID(programClass, "endTime", "J");
            jlong jendTime        = env->GetLongField(programObject, endTimeField);
            program->endTime      = static_cast<uint32_t>(jendTime);

            uint32_t recordFlag               = 0;
            jfieldID recordFlagScheduledField = env->GetFieldID(programClass, "recordFlagScheduled", "Z");
            jboolean jrecordFlagScheduled     = env->GetBooleanField(programObject, recordFlagScheduledField);
            if (jrecordFlagScheduled)
            {
                recordFlag |= 1;
            }
            jfieldID recordFlagSeriesField = env->GetFieldID(programClass, "recordFlagSeries", "Z");
            jboolean jrecordFlagSeries     = env->GetBooleanField(programObject, recordFlagSeriesField);
            if (jrecordFlagSeries)
            {
                recordFlag |= 2;
            }
            jfieldID recordFlagRecordedField = env->GetFieldID(programClass, "recordFlagRecorded", "Z");
            jboolean jrecordFlagRecorded     = env->GetBooleanField(programObject, recordFlagRecordedField);
            if (jrecordFlagRecorded)
            {
                recordFlag |= 4;
            }

            program->recordingFlag = MakeOptional(static_cast<uint32_t>(recordFlag));

            needToFreePrograms.push_back(program);
            tempPrograms.push_back(*program);
        }

        response.programList = DataModel::List<const ProgramType>(tempPrograms.data(), tempPrograms.size());

        err = helper.Success(response);
    }

exit:
    for (ProgramType * program : needToFreePrograms)
    {
        delete program;
    }

    for (JniUtfString * str : needToFreeStrings)
    {
        delete str;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ChannelManager::HandleGetProgramGuide status error: %s", err.AsString());
    }
}

bool ChannelManager::HandleRecordProgram(const chip::CharSpan & programIdentifier, bool shouldRecordSeries,
                                         const DataModel::DecodableList<AdditionalInfo> & externalIdList,
                                         const chip::ByteSpan & data)
{
    DeviceLayer::StackUnlock unlock;
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ChannelManager::HandleRecordProgram");
    VerifyOrExit(mChannelManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mChannelManagerObject null"));
    VerifyOrExit(mRecordProgramMethod != nullptr, ChipLogError(Zcl, "mRecordProgramMethod null"));

    env->ExceptionClear();

    {
        UtfString jIdentifier(env, programIdentifier);

        // NOTE: this example app does not pass the Data or ExternalIdList through to the Java layer
        UtfString jData(env, "");
        jobjectArray externalIDListArray = (jobjectArray) env->NewObjectArray(0, env->FindClass("java/util/Map$Entry"), NULL);

        ret = env->CallBooleanMethod(mChannelManagerObject.ObjectRef(), mRecordProgramMethod, jIdentifier.jniValue(),
                                     static_cast<jboolean>(shouldRecordSeries), externalIDListArray, jData.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in ChannelManager::HandleRecordProgram");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return false;
        }
    }

exit:
    return static_cast<bool>(ret);
}

bool ChannelManager::HandleCancelRecordProgram(const chip::CharSpan & programIdentifier, bool shouldRecordSeries,
                                               const DataModel::DecodableList<AdditionalInfo> & externalIdList,
                                               const chip::ByteSpan & data)
{
    DeviceLayer::StackUnlock unlock;
    jboolean ret = JNI_FALSE;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received ChannelManager::HandleCancelRecordProgram");
    VerifyOrExit(mChannelManagerObject.HasValidObjectRef(), ChipLogError(Zcl, "mChannelManagerObject null"));
    VerifyOrExit(mCancelRecordProgramMethod != nullptr, ChipLogError(Zcl, "mCancelRecordProgramMethod null"));

    env->ExceptionClear();

    {
        UtfString jIdentifier(env, programIdentifier);

        // NOTE: this example app does not pass the Data or ExternalIdList through to the Java layer
        UtfString jData(env, "");
        jobjectArray externalIDListArray = (jobjectArray) env->NewObjectArray(0, env->FindClass("java/util/Map$Entry"), NULL);

        ret = env->CallBooleanMethod(mChannelManagerObject.ObjectRef(), mCancelRecordProgramMethod, jIdentifier.jniValue(),
                                     static_cast<jboolean>(shouldRecordSeries), externalIDListArray, jData.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(DeviceLayer, "Java exception in ChannelManager::HandleCancelRecordProgram");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return false;
        }
    }

exit:
    return static_cast<bool>(ret);
}

void ChannelManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for ChannelManager"));

    VerifyOrReturn(mChannelManagerObject.Init(managerObject) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "Failed to init mChannelManagerObject"));

    jclass managerClass = env->GetObjectClass(managerObject);
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

    mGetProgramGuideMethod = env->GetMethodID(managerClass, "getProgramGuide",
                                              "(JJ[Lcom/matter/tv/server/tvapp/ChannelInfo;Ljava/lang/String;Z[Ljava/util/"
                                              "Map$Entry;Ljava/lang/String;)Lcom/matter/tv/server/tvapp/ChannelProgramResponse;");
    if (mGetProgramGuideMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'getProgramGuide' method");
        env->ExceptionClear();
    }

    mRecordProgramMethod =
        env->GetMethodID(managerClass, "recordProgram", "(Ljava/lang/String;Z[Ljava/util/Map$Entry;Ljava/lang/String;)Z");
    if (mRecordProgramMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'recordProgram' method");
        env->ExceptionClear();
    }

    mCancelRecordProgramMethod =
        env->GetMethodID(managerClass, "cancelRecordProgram", "(Ljava/lang/String;Z[Ljava/util/Map$Entry;Ljava/lang/String;)Z");
    if (mCancelRecordProgramMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'cancelRecordProgram' method");
        env->ExceptionClear();
    }
}

uint32_t ChannelManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}

uint16_t ChannelManager::GetClusterRevision(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kClusterRevision;
    }

    uint16_t clusterRevision = 0;
    bool success =
        (Attributes::ClusterRevision::Get(endpoint, &clusterRevision) == chip::Protocols::InteractionModel::Status::Success);
    if (!success)
    {
        ChipLogError(Zcl, "ChannelManager::GetClusterRevision error reading cluster revision");
    }
    return clusterRevision;
}
