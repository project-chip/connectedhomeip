/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include "TvCastingApp-JNI.h"
#include "CastingServer.h"
#include "Constants.h"
#include "ConversionUtils.h"
#include "JNIDACProvider.h"

#include <app/server/Server.h>
#include <app/server/java/AndroidAppServerWrapper.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <inet/InetInterface.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CHIPListUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_chip_casting_TvCastingApp_##METHOD_NAME

TvCastingAppJNI TvCastingAppJNI::sInstance;

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnLoad(jvm, reserved);
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnUnload(jvm, reserved);
}

JNI_METHOD(jboolean, init)(JNIEnv *, jobject, jobject jAppParameters)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_METHOD init called");

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (jAppParameters == nullptr)
    {
        err = CastingServer::GetInstance()->Init();
    }
    else
    {
        AppParams appParams;
        err = convertJAppParametersToCppAppParams(jAppParameters, appParams);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogError(AppServer, "Conversion of AppParameters from jobject to Cpp type failed: %" CHIP_ERROR_FORMAT,
                                  err.Format()));
        err = CastingServer::GetInstance()->Init(&appParams);
    }
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Call to CastingServer::GetInstance()->Init() failed: %" CHIP_ERROR_FORMAT, err.Format()));
exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(void, setDACProvider)(JNIEnv *, jobject, jobject provider)
{
    if (!chip::Credentials::IsDeviceAttestationCredentialsProviderSet())
    {
        JNIDACProvider * p = new JNIDACProvider(provider);
        chip::Credentials::SetDeviceAttestationCredentialsProvider(p);
    }
}

JNI_METHOD(jboolean, openBasicCommissioningWindow)
(JNIEnv * env, jobject, jint duration, jobject jCommissioningCompleteHandler, jobject jOnConnectionSuccessHandler,
 jobject jOnConnectionFailureHandler, jobject jOnNewOrUpdatedEndpointHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD openBasicCommissioningWindow called with duration %d", duration);
    CHIP_ERROR err = TvCastingAppJNIMgr().getCommissioningCompleteHandler().SetUp(env, jCommissioningCompleteHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI::SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getOnConnectionSuccessHandler(false).SetUp(env, jOnConnectionSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "OnConnectionSuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getOnConnectionFailureHandler(false).SetUp(env, jOnConnectionFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "OnConnectionFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getOnNewOrUpdatedEndpointHandler(false).SetUp(env, jOnNewOrUpdatedEndpointHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "OnNewOrUpdatedEndpointHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->OpenBasicCommissioningWindow(
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getCommissioningCompleteHandler().Handle(err); },
        [](TargetVideoPlayerInfo * videoPlayer) { TvCastingAppJNIMgr().getOnConnectionSuccessHandler(false).Handle(videoPlayer); },
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getOnConnectionFailureHandler(false).Handle(err); },
        [](TargetEndpointInfo * endpoint) { TvCastingAppJNIMgr().getOnNewOrUpdatedEndpointHandler(false).Handle(endpoint); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer::OpenBasicCommissioningWindow failed: %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jobject, readCachedVideoPlayers)(JNIEnv * env, jobject)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD readCachedVideoPlayers called");

    jobject jVideoPlayerList                          = nullptr;
    TargetVideoPlayerInfo * targetVideoPlayerInfoList = CastingServer::GetInstance()->ReadCachedTargetVideoPlayerInfos();
    if (targetVideoPlayerInfoList != nullptr)
    {
        chip::JniReferences::GetInstance().CreateArrayList(jVideoPlayerList);
        for (size_t i = 0; targetVideoPlayerInfoList[i].IsInitialized(); i++)
        {
            jobject jVideoPlayer = nullptr;
            CHIP_ERROR err       = convertTargetVideoPlayerInfoToJVideoPlayer(&targetVideoPlayerInfoList[i], jVideoPlayer);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "Conversion from TargetVideoPlayerInfo * to jobject VideoPlayer failed: %" CHIP_ERROR_FORMAT,
                             err.Format());
                continue;
            }
            chip::JniReferences::GetInstance().AddToList(jVideoPlayerList, jVideoPlayer);
        }
    }

    return jVideoPlayerList;
}

JNI_METHOD(jboolean, verifyOrEstablishConnection)
(JNIEnv * env, jobject, jobject videoPlayer, jobject jOnConnectionSuccessHandler, jobject jOnConnectionFailureHandler,
 jobject jOnNewOrUpdatedEndpointHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD verifyOrEstablishConnection called");

    TargetVideoPlayerInfo targetVideoPlayerInfo;
    CHIP_ERROR err = convertJVideoPlayerToTargetVideoPlayerInfo(videoPlayer, targetVideoPlayerInfo);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer,
                              "Conversion from jobject VideoPlayer to TargetVideoPlayerInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getOnConnectionSuccessHandler(true).SetUp(env, jOnConnectionSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "OnConnectionSuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getOnConnectionFailureHandler(true).SetUp(env, jOnConnectionFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "OnConnectionFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getOnNewOrUpdatedEndpointHandler(true).SetUp(env, jOnNewOrUpdatedEndpointHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "OnNewOrUpdatedEndpointHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->VerifyOrEstablishConnection(
        targetVideoPlayerInfo,
        [](TargetVideoPlayerInfo * videoPlayer) { TvCastingAppJNIMgr().getOnConnectionSuccessHandler(true).Handle(videoPlayer); },
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getOnConnectionFailureHandler(true).Handle(err); },
        [](TargetEndpointInfo * endpoint) { TvCastingAppJNIMgr().getOnNewOrUpdatedEndpointHandler(true).Handle(endpoint); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer::OpenBasicCommissioningWindow failed: %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(void, shutdownAllSubscriptions)(JNIEnv * env, jobject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_METHOD shutdownAllSubscriptions called");

    CastingServer::GetInstance()->ShutdownAllSubscriptions();
}

JNI_METHOD(void, disconnect)(JNIEnv * env, jobject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_METHOD disconnect called");
    CastingServer::GetInstance()->Disconnect();
}

JNI_METHOD(jobject, getActiveTargetVideoPlayers)(JNIEnv * env, jobject)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD getActiveTargetVideoPlayers called");

    jobject jVideoPlayerList                      = nullptr;
    TargetVideoPlayerInfo * targetVideoPlayerInfo = CastingServer::GetInstance()->GetActiveTargetVideoPlayer();
    if (targetVideoPlayerInfo != nullptr)
    {
        chip::JniReferences::GetInstance().CreateArrayList(jVideoPlayerList);
        jobject jVideoPlayer = nullptr;
        CHIP_ERROR err       = convertTargetVideoPlayerInfoToJVideoPlayer(targetVideoPlayerInfo, jVideoPlayer);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Conversion from TargetVideoPlayerInfo * to jobject VideoPlayer failed: %" CHIP_ERROR_FORMAT,
                         err.Format());
        }
        else
        {
            chip::JniReferences::GetInstance().AddToList(jVideoPlayerList, jVideoPlayer);
        }
    }
    return jVideoPlayerList;
}

JNI_METHOD(jboolean, sendUserDirectedCommissioningRequest)(JNIEnv * env, jobject, jstring addressJStr, jint port)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_METHOD sendUserDirectedCommissioningRequest called with port %d", port);
    Inet::IPAddress addressInet;
    JniUtfString addressJniString(env, addressJStr);
    if (Inet::IPAddress::FromString(addressJniString.c_str(), addressInet) == false)
    {
        ChipLogError(AppServer, "Failed to parse IP address");
        return false;
    }

    chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId::FromIPAddress(addressInet);
    chip::Transport::PeerAddress peerAddress =
        chip::Transport::PeerAddress::UDP(addressInet, static_cast<uint16_t>(port), interfaceId);
    CHIP_ERROR err = CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(peerAddress);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "TVCastingApp-JNI::sendUserDirectedCommissioningRequest failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }
    return true;
}

JNI_METHOD(jboolean, sendCommissioningRequest)(JNIEnv * env, jobject, jobject jDiscoveredNodeData)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_METHOD sendCommissioningRequest called");

    chip::Dnssd::DiscoveredNodeData commissioner;
    CHIP_ERROR err = convertJDiscoveredNodeDataToCppDiscoveredNodeData(jDiscoveredNodeData, commissioner);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer,
                              "Conversion from jobject DiscoveredNodeData to Cpp DiscoveredNodeData failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(&commissioner);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "TVCastingApp-JNI::sendCommissioningRequest failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }
    return true;
}

JNI_METHOD(jboolean, contentLauncherLaunchURL)
(JNIEnv * env, jobject, jobject contentApp, jstring contentUrl, jstring contentDisplayStr, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD contentLauncherLaunchURL called");
    const char * nativeContentUrl        = env->GetStringUTFChars(contentUrl, 0);
    const char * nativeContentDisplayStr = env->GetStringUTFChars(contentDisplayStr, 0);

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(ContentLauncher_LaunchURL).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI::SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ContentLauncherLaunchURL(
        &endpoint, nativeContentUrl, nativeContentDisplayStr,
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(ContentLauncher_LaunchURL).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer::ContentLauncherLaunchURL failed %" CHIP_ERROR_FORMAT, err.Format()));

    env->ReleaseStringUTFChars(contentUrl, nativeContentUrl);
    env->ReleaseStringUTFChars(contentDisplayStr, nativeContentDisplayStr);

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

CHIP_ERROR CreateParameter(JNIEnv * env, jobject jParameter,
                           chip::app::Clusters::ContentLauncher::Structs::Parameter::Type & parameter)
{
    jclass jParameterClass = env->GetObjectClass(jParameter);

    jfieldID jTypeField    = env->GetFieldID(jParameterClass, "type", "Ljava/lang/Integer;");
    jobject jTypeObj       = env->GetObjectField(jParameter, jTypeField);
    jclass jIntegerClass   = env->FindClass("java/lang/Integer");
    jmethodID jIntValueMid = env->GetMethodID(jIntegerClass, "intValue", "()I");
    parameter.type = static_cast<chip::app::Clusters::ContentLauncher::ParameterEnum>(env->CallIntMethod(jTypeObj, jIntValueMid));

    jfieldID jValueField     = env->GetFieldID(jParameterClass, "value", "Ljava/lang/String;");
    jstring jValueObj        = (jstring) env->GetObjectField(jParameter, jValueField);
    const char * nativeValue = env->GetStringUTFChars(jValueObj, 0);
    parameter.value          = CharSpan::fromCharString(nativeValue);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CreateContentSearch(JNIEnv * env, jobject jSearch,
                               chip::app::Clusters::ContentLauncher::Structs::ContentSearch::Type & search, ListFreer & listFreer)
{
    jclass jContentSearchClass;
    ReturnErrorOnFailure(
        JniReferences::GetInstance().GetClassRef(env, "com/chip/casting/ContentLauncherTypes$ContentSearch", jContentSearchClass));

    jfieldID jParameterListField = env->GetFieldID(jContentSearchClass, "parameterList", "Ljava/util/ArrayList;");
    jobject jParameterList       = env->GetObjectField(jSearch, jParameterListField);
    ReturnErrorOnFailure(jParameterList != nullptr ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT);

    jclass jArrayListClass   = env->FindClass("java/util/ArrayList");
    jmethodID sizeMid        = env->GetMethodID(jArrayListClass, "size", "()I");
    size_t parameterListSize = static_cast<size_t>(env->CallIntMethod(jParameterList, sizeMid));

    jobject jIterator = env->CallObjectMethod(
        jParameterList, env->GetMethodID(env->GetObjectClass(jParameterList), "iterator", "()Ljava/util/Iterator;"));
    jmethodID jNextMid    = env->GetMethodID(env->GetObjectClass(jIterator), "next", "()Ljava/lang/Object;");
    jmethodID jHasNextMid = env->GetMethodID(env->GetObjectClass(jIterator), "hasNext", "()Z");

    auto * parameterListHolder = new ListHolder<chip::app::Clusters::ContentLauncher::Structs::Parameter::Type>(parameterListSize);
    listFreer.add(parameterListHolder);
    int parameterIndex = 0;
    while (env->CallBooleanMethod(jIterator, jHasNextMid))
    {
        jobject jParameter = env->CallObjectMethod(jIterator, jNextMid);
        chip::app::Clusters::ContentLauncher::Structs::Parameter::Type parameter;
        ReturnErrorOnFailure(CreateParameter(env, jParameter, parameter));
        parameterListHolder->mList[parameterIndex].type           = parameter.type;
        parameterListHolder->mList[parameterIndex].value          = parameter.value;
        parameterListHolder->mList[parameterIndex].externalIDList = parameter.externalIDList;
        parameterIndex++;
    }
    search.parameterList = chip::app::DataModel::List<chip::app::Clusters::ContentLauncher::Structs::Parameter::Type>(
        parameterListHolder->mList, parameterListSize);

    return CHIP_NO_ERROR;
}

JNI_METHOD(jboolean, contentLauncher_1launchContent)
(JNIEnv * env, jobject, jobject contentApp, jobject jSearch, jboolean jAutoplay, jstring jData, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD contentLauncher_1launchContent called");

    // prepare arguments
    bool autoplay = static_cast<bool>(jAutoplay);

    const char * nativeData             = env->GetStringUTFChars(jData, 0);
    chip::Optional<chip::CharSpan> data = MakeOptional(CharSpan::fromCharString(nativeData));

    ListFreer listFreer;
    chip::app::Clusters::ContentLauncher::Structs::ContentSearch::Type search;
    CHIP_ERROR err = CreateContentSearch(env, jSearch, search, listFreer);

    TargetEndpointInfo endpoint;
    err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer,
                              "contentLauncher_1launchContent::Could not create ContentSearch object %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(ContentLauncher_LaunchContent).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI::SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ContentLauncher_LaunchContent(&endpoint, search, autoplay, data, [](CHIP_ERROR err) {
        TvCastingAppJNIMgr().getMediaCommandResponseHandler(ContentLauncher_LaunchContent).Handle(err);
    });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer::ContentLauncher_LaunchContent failed %" CHIP_ERROR_FORMAT, err.Format()));

    env->ReleaseStringUTFChars(jData, nativeData);

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, contentLauncher_1subscribeToSupportedStreamingProtocols)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD ContentLauncher_subscribeToSupportedStreamingProtocols called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getSupportedStreamingProtocolsSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getReadFailureHandler(ContentLauncher_SupportedStreamingProtocols).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(ContentLauncher_SupportedStreamingProtocols)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ContentLauncher_SubscribeToSupportedStreamingProtocols(
        &endpoint, nullptr,
        [](void * context,
           chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getSupportedStreamingProtocolsSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getReadFailureHandler(ContentLauncher_SupportedStreamingProtocols).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) {
            TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(ContentLauncher_SupportedStreamingProtocols).Handle();
        });

    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer,
                              "CastingServer.ContentLauncher_SubscribeToSupportedStreamingProtocols failed %" CHIP_ERROR_FORMAT,
                              err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, levelControl_1step)
(JNIEnv * env, jobject, jobject contentApp, jbyte stepMode, jbyte stepSize, jshort transitionTime, jbyte optionMask,
 jbyte optionOverride, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD levelControl_step called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(LevelControl_Step).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->LevelControl_Step(
        &endpoint, static_cast<chip::app::Clusters::LevelControl::StepMode>(stepMode), static_cast<uint8_t>(stepSize),
        static_cast<uint16_t>(transitionTime), static_cast<uint8_t>(optionMask), static_cast<uint8_t>(optionOverride),
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(LevelControl_Step).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.LevelControl_Step failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, levelControl_1moveToLevel)
(JNIEnv * env, jobject, jobject contentApp, jbyte level, jshort transitionTime, jbyte optionMask, jbyte optionOverride,
 jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD levelControl_moveToLevel called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(LevelControl_MoveToLevel).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->LevelControl_MoveToLevel(
        &endpoint, static_cast<uint8_t>(level), static_cast<uint16_t>(transitionTime), static_cast<uint8_t>(optionMask),
        static_cast<uint8_t>(optionOverride),
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(LevelControl_MoveToLevel).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.LevelControl_MoveToLevel failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, levelControl_1subscribeToCurrentLevel)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD levelControl_subscribeToCurrentLevel called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getCurrentLevelSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(LevelControl_CurrentLevel).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(LevelControl_CurrentLevel)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->LevelControl_SubscribeToCurrentLevel(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getCurrentLevelSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(LevelControl_CurrentLevel).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(LevelControl_CurrentLevel).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.LevelControl_SubscribeToCurrentLevel failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, levelControl_1subscribeToMinLevel)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD levelControl_subscribeToMinLevel called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMinLevelSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(LevelControl_MinLevel).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(LevelControl_MinLevel).SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->LevelControl_SubscribeToMinLevel(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getMinLevelSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(LevelControl_MinLevel).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(LevelControl_MinLevel).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.LevelControl_SubscribeToMinLevel failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, levelControl_1subscribeToMaxLevel)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD levelControl_subscribeToMaxLevel called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMaxLevelSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(LevelControl_MaxLevel).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(LevelControl_MaxLevel).SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->LevelControl_SubscribeToMaxLevel(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getMaxLevelSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(LevelControl_MaxLevel).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(LevelControl_MaxLevel).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.LevelControl_SubscribeToMaxLevel failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1play)
(JNIEnv * env, jobject, jobject contentApp, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_play called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Play).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_Play(
        &endpoint, [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Play).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_Play failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1pause)
(JNIEnv * env, jobject, jobject contentApp, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_pause called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Pause).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_Pause(
        &endpoint, [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Pause).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_Pause failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1stopPlayback)
(JNIEnv * env, jobject, jobject contentApp, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_stopPlayback called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_StopPlayback).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_StopPlayback(&endpoint, [](CHIP_ERROR err) {
        TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_StopPlayback).Handle(err);
    });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_StopPlayback failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1next)
(JNIEnv * env, jobject, jobject contentApp, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_next called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Next).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_Next(
        &endpoint, [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Next).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_Next failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1seek)
(JNIEnv * env, jobject, jobject contentApp, jlong position, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_seek called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Seek).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_Seek(&endpoint, static_cast<uint64_t>(position), [](CHIP_ERROR err) {
        TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Seek).Handle(err);
    });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_Seek failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1skipForward)
(JNIEnv * env, jobject, jobject contentApp, jlong deltaPositionMilliseconds, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_skipForward called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_SkipForward).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SkipForward(
        &endpoint, static_cast<uint64_t>(deltaPositionMilliseconds),
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_SkipForward).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_SkipForward failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1skipBackward)
(JNIEnv * env, jobject, jobject contentApp, jlong deltaPositionMilliseconds, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_skipBackward called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_SkipBackward).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SkipBackward(
        &endpoint, static_cast<uint64_t>(deltaPositionMilliseconds),
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_SkipBackward).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_SkipBackward failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1subscribeToCurrentState)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_subscribeToCurrentState called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getCurrentStateSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_CurrentState).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(MediaPlayback_CurrentState)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SubscribeToCurrentState(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getCurrentStateSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_CurrentState).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(MediaPlayback_CurrentState).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.MediaPlayback_SubscribeToCurrentState failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1subscribeToDuration)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_1subscribeToDuration called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getDurationSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_Duration).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err =
        TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(MediaPlayback_Duration).SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getDurationSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_Duration).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(MediaPlayback_Duration).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.mediaPlayback_subscribeToDuration failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1subscribeToSampledPosition)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_1subscribeToSampledPosition called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getSampledPositionSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_SampledPosition).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(MediaPlayback_SampledPosition)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SubscribeToSampledPosition(
        &endpoint, nullptr,
        [](void * context,
           chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getSampledPositionSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_SampledPosition).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(MediaPlayback_SampledPosition).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.mediaPlayback_subscribeToSampledPosition failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1subscribeToPlaybackSpeed)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_1subscribeToPlaybackSpeed called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getPlaybackSpeedSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_PlaybackSpeed).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(MediaPlayback_PlaybackSpeed)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SubscribeToPlaybackSpeed(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getPlaybackSpeedSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_PlaybackSpeed).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(MediaPlayback_PlaybackSpeed).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.mediaPlayback_subscribeToPlaybackSpeed failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1subscribeToSeekRangeEnd)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_1subscribeToSeekRangeEnd called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getSeekRangeEndSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_SeekRangeEnd).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(MediaPlayback_SeekRangeEnd)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SubscribeToSeekRangeEnd(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getSeekRangeEndSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_SeekRangeEnd).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(MediaPlayback_SeekRangeEnd).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.mediaPlayback_subscribeToSeekRangeEnd failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_1subscribeToSeekRangeStart)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_1subscribeToSeekRangeStart called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getSeekRangeStartSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_SeekRangeStart).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(MediaPlayback_SeekRangeStart)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SubscribeToSeekRangeStart(
        &endpoint, nullptr,
        [](void * context,
           chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getSeekRangeStartSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(MediaPlayback_SeekRangeStart).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(MediaPlayback_SeekRangeStart).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.mediaPlayback_subscribeToSeekRangeStart failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationLauncher_1launchApp)
(JNIEnv * env, jobject, jobject contentApp, jshort catalogVendorId, jstring applicationId, jbyteArray data,
 jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationLauncher_launchApp called");

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId      = static_cast<uint16_t>(catalogVendorId);
    const char * nativeApplicationId = env->GetStringUTFChars(applicationId, 0);
    application.applicationId        = CharSpan::fromCharString(nativeApplicationId);
    JniByteArray dataByteArray(env, data);

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(ApplicationLauncher_LaunchApp).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationLauncher_LaunchApp(
        &endpoint, application, chip::MakeOptional(dataByteArray.byteSpan()),
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(ApplicationLauncher_LaunchApp).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.ApplicationLauncher_LaunchApp failed %" CHIP_ERROR_FORMAT, err.Format()));

    env->ReleaseStringUTFChars(applicationId, nativeApplicationId);
exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationLauncher_1stopApp)
(JNIEnv * env, jobject, jobject contentApp, jshort catalogVendorId, jstring applicationId, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationLauncher_stopApp called");

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId      = static_cast<uint16_t>(catalogVendorId);
    const char * nativeApplicationId = env->GetStringUTFChars(applicationId, 0);
    application.applicationId        = CharSpan::fromCharString(nativeApplicationId);

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(ApplicationLauncher_StopApp).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationLauncher_StopApp(&endpoint, application, [&](CHIP_ERROR err) {
        TvCastingAppJNIMgr().getMediaCommandResponseHandler(ApplicationLauncher_StopApp).Handle(err);
    });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.ApplicationLauncher_StopApp failed %" CHIP_ERROR_FORMAT, err.Format()));

    env->ReleaseStringUTFChars(applicationId, nativeApplicationId);
exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationLauncher_1hideApp)
(JNIEnv * env, jobject, jobject contentApp, jshort catalogVendorId, jstring applicationId, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationLauncher_hideApp called");

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId      = static_cast<uint16_t>(catalogVendorId);
    const char * nativeApplicationId = env->GetStringUTFChars(applicationId, 0);
    application.applicationId        = CharSpan::fromCharString(nativeApplicationId);

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(ApplicationLauncher_HideApp).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationLauncher_HideApp(&endpoint, application, [](CHIP_ERROR err) {
        TvCastingAppJNIMgr().getMediaCommandResponseHandler(ApplicationLauncher_HideApp).Handle(err);
    });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.ApplicationLauncher_HideApp failed %" CHIP_ERROR_FORMAT, err.Format()));

    env->ReleaseStringUTFChars(applicationId, nativeApplicationId);
exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, targetNavigator_1navigateTarget)
(JNIEnv * env, jobject, jobject contentApp, jbyte target, jstring data, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD targetNavigator_navigateTarget called");

    const char * nativeData = env->GetStringUTFChars(data, 0);

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(TargetNavigator_NavigateTarget).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->TargetNavigator_NavigateTarget(
        &endpoint, static_cast<uint8_t>(target), chip::MakeOptional(CharSpan::fromCharString(nativeData)),
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(TargetNavigator_NavigateTarget).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.TargetNavigator_NavigateTarget failed %" CHIP_ERROR_FORMAT, err.Format()));

    env->ReleaseStringUTFChars(data, nativeData);
exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, targetNavigator_1subscribeToCurrentTarget)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD targetNavigator_1subscribeToCurrentTarget called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getCurrentTargetSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(TargetNavigator_CurrentTarget).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(TargetNavigator_CurrentTarget)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->TargetNavigator_SubscribeToCurrentTarget(
        &endpoint, nullptr,
        [](void * context,
           chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getCurrentTargetSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(TargetNavigator_CurrentTarget).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(TargetNavigator_CurrentTarget).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.targetNavigator_subscribeToCurrentTarget failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, targetNavigator_1subscribeToTargetList)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD targetNavigator_1subscribeToTargetList called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getTargetListSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(TargetNavigator_TargetList).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(TargetNavigator_TargetList)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->TargetNavigator_SubscribeToTargetList(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getTargetListSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(TargetNavigator_TargetList).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(TargetNavigator_TargetList).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.targetNavigator_subscribeToTargetList failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, keypadInput_1sendKey)
(JNIEnv * env, jobject, jobject contentApp, jbyte keyCode, jobject jResponseHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD keypadInput_sendKey called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getMediaCommandResponseHandler(KeypadInput_SendKey).SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->KeypadInput_SendKey(
        &endpoint, static_cast<chip::app::Clusters::KeypadInput::CecKeyCode>(keyCode),
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getMediaCommandResponseHandler(KeypadInput_SendKey).Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.KeypadInput_SendKey failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

// APPLICATION BASIC
JNI_METHOD(jboolean, applicationBasic_1subscribeToVendorName)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1subscribeToVendorName called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getVendorNameSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_VendorName).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(ApplicationBasic_VendorName)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToVendorName(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getVendorNameSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_VendorName).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(ApplicationBasic_VendorName).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.applicationBasic_subscribeToVendorName failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationBasic_1subscribeToVendorID)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1subscribeToVendorID called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getVendorIDSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_VendorID).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(ApplicationBasic_VendorID)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToVendorID(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getVendorIDSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_VendorID).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(ApplicationBasic_VendorID).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.applicationBasic_subscribeToVendorID failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationBasic_1subscribeToApplicationName)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1subscribeToApplicationName called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getApplicationNameSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_ApplicationName).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(ApplicationBasic_ApplicationName)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToApplicationName(
        &endpoint, nullptr,
        [](void * context,
           chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getApplicationNameSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_ApplicationName).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(ApplicationBasic_ApplicationName).Handle(); });

    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.applicationBasic_subscribeToApplicationName failed %" CHIP_ERROR_FORMAT,
                              err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationBasic_1subscribeToProductID)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1subscribeToProductID called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getProductIDSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_ProductID).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(ApplicationBasic_ProductID)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToProductID(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getProductIDSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_ProductID).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) { TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(ApplicationBasic_ProductID).Handle(); });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.applicationBasic_subscribeToProductID failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationBasic_1subscribeToApplicationVersion)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler, jint minInterval,
 jint maxInterval, jobject jSubscriptionEstablishedHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1subscribeToApplicationVersion called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getApplicationVersionSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err =
        TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_ApplicationVersion).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr()
              .getSubscriptionEstablishedHandler(ApplicationBasic_ApplicationVersion)
              .SetUp(env, jSubscriptionEstablishedHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "SubscriptionEstablishedHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToApplicationVersion(
        &endpoint, nullptr,
        [](void * context,
           chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getApplicationVersionSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getSubscriptionReadFailureHandler(ApplicationBasic_ApplicationVersion).Handle(err);
        },
        static_cast<uint16_t>(minInterval), static_cast<uint16_t>(maxInterval),
        [](void * context) {
            TvCastingAppJNIMgr().getSubscriptionEstablishedHandler(ApplicationBasic_ApplicationVersion).Handle();
        });

    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.applicationBasic_subscribeToApplicationVersion failed %" CHIP_ERROR_FORMAT,
                              err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationBasic_1readVendorName)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1readVendorName called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getReadVendorNameSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_VendorName).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "ReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_ReadVendorName(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getVendorNameSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_VendorName).Handle(err);
        });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.applicationBasic_1readVendorName failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationBasic_1readVendorID)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1readVendorID called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getVendorIDSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_VendorID).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "ReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_ReadVendorID(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getVendorIDSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) { TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_VendorID).Handle(err); });

    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.applicationBasic_ReadVendorID failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationBasic_1readApplicationName)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1readApplicationName called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getApplicationNameSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_ApplicationName).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "ReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_ReadApplicationName(
        &endpoint, nullptr,
        [](void * context,
           chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getApplicationNameSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_ApplicationName).Handle(err);
        });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.applicationBasic_ReadApplicationName failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationBasic_1readProductID)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1readProductID called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getProductIDSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_ProductID).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "ReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_ReadProductID(
        &endpoint, nullptr,
        [](void * context, chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getProductIDSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) { TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_ProductID).Handle(err); });

    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.applicationBasic_ReadProductID failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationBasic_1readApplicationVersion)
(JNIEnv * env, jobject, jobject contentApp, jobject jReadSuccessHandler, jobject jReadFailureHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD applicationBasic_1readApplicationVersion called");

    TargetEndpointInfo endpoint;
    CHIP_ERROR err = convertJContentAppToTargetEndpointInfo(contentApp, endpoint);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(AppServer, "Conversion from jobject contentApp to TargetEndpointInfo * failed: %" CHIP_ERROR_FORMAT,
                              err.Format()));

    err = TvCastingAppJNIMgr().getApplicationVersionSuccessHandler().SetUp(env, jReadSuccessHandler);
    VerifyOrExit(CHIP_NO_ERROR == err, ChipLogError(AppServer, "SuccessHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_ApplicationVersion).SetUp(env, jReadFailureHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "ReadFailureHandler.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationBasic_ReadApplicationVersion(
        &endpoint, nullptr,
        [](void * context,
           chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType responseData) {
            TvCastingAppJNIMgr().getApplicationVersionSuccessHandler().Handle(responseData);
        },
        [](void * context, CHIP_ERROR err) {
            TvCastingAppJNIMgr().getReadFailureHandler(ApplicationBasic_ApplicationVersion).Handle(err);
        });

    VerifyOrExit(
        CHIP_NO_ERROR == err,
        ChipLogError(AppServer, "CastingServer.applicationBasic_ReadApplicationVersion failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}
