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
#include "JNIDACProvider.h"

#include <app/server/Server.h>
#include <app/server/java/AndroidAppServerWrapper.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <inet/InetInterface.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
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

JNI_METHOD(void, setDACProvider)(JNIEnv *, jobject, jobject provider)
{
    if (!chip::Credentials::IsDeviceAttestationCredentialsProviderSet())
    {
        JNIDACProvider * p = new JNIDACProvider(provider);
        chip::Credentials::SetDeviceAttestationCredentialsProvider(p);
    }
}

JNI_METHOD(jboolean, openBasicCommissioningWindow)(JNIEnv * env, jobject, jint duration, jobject jCommissioningCompleteHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD openBasicCommissioningWindow called with duration %d", duration);
    CHIP_ERROR err = TvCastingAppJNIMgr().getCommissioningCompleteHandler().SetUp(env, jCommissioningCompleteHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI::SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->OpenBasicCommissioningWindow(
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getCommissioningCompleteHandler().Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer::OpenBasicCommissioningWindow failed: %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, sendUserDirectedCommissioningRequest)(JNIEnv * env, jobject, jstring addressJStr, jint port)
{
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

JNI_METHOD(jboolean, discoverCommissioners)(JNIEnv *, jobject)
{
    ChipLogProgress(AppServer, "JNI_METHOD discoverCommissioners called");
    CHIP_ERROR err = CastingServer::GetInstance()->DiscoverCommissioners();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "TVCastingApp-JNI::discoverCommissioners failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
}

JNI_METHOD(void, init)(JNIEnv *, jobject)
{
    ChipLogProgress(AppServer, "JNI_METHOD init called");
    CastingServer::GetInstance()->Init();
}

JNI_METHOD(jboolean, contentLauncherLaunchURL)
(JNIEnv * env, jobject, jstring contentUrl, jstring contentDisplayStr, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD contentLauncherLaunchURL called");
    const char * nativeContentUrl        = env->GetStringUTFChars(contentUrl, 0);
    const char * nativeContentDisplayStr = env->GetStringUTFChars(contentDisplayStr, 0);

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(ContentLauncher_LaunchURL);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI::SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ContentLauncherLaunchURL(nativeContentUrl, nativeContentDisplayStr, [](CHIP_ERROR err) {
        TvCastingAppJNIMgr().getMediaCommandResponseHandler(ContentLauncher_LaunchURL).Handle(err);
    });
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

JNI_METHOD(jboolean, levelControl_step)
(JNIEnv * env, jobject, jbyte stepMode, jbyte stepSize, jshort transitionTime, jbyte optionMask, jbyte optionOverride,
 jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD levelControl_step called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(LevelControl_Step);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->LevelControl_Step(static_cast<chip::app::Clusters::LevelControl::StepMode>(stepMode),
                                                          static_cast<uint8_t>(stepSize), static_cast<uint16_t>(transitionTime),
                                                          static_cast<uint8_t>(optionMask), static_cast<uint8_t>(optionOverride),
                                                          [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.LevelControl_Step failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, levelControl_moveToLevel)
(JNIEnv * env, jobject, jbyte level, jshort transitionTime, jbyte optionMask, jbyte optionOverride, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD levelControl_moveToLevel called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(LevelControl_MoveToLevel);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->LevelControl_MoveToLevel(
        static_cast<uint8_t>(level), static_cast<uint16_t>(transitionTime), static_cast<uint8_t>(optionMask),
        static_cast<uint8_t>(optionOverride), [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.LevelControl_MoveToLevel failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_play)
(JNIEnv * env, jobject, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_play called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Play);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_Play([&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_Play failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_pause)
(JNIEnv * env, jobject, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_pause called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Pause);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_Pause([&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_Pause failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_stopPlayback)
(JNIEnv * env, jobject, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_stopPlayback called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_StopPlayback);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_StopPlayback(
        [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_StopPlayback failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_next)
(JNIEnv * env, jobject, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_next called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Next);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_Next([&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_Next failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_seek)
(JNIEnv * env, jobject, jlong position, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_seek called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_Seek);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_Seek(static_cast<uint64_t>(position),
                                                           [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_Seek failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_skipForward)
(JNIEnv * env, jobject, jlong deltaPositionMilliseconds, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_skipForward called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_SkipForward);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SkipForward(
        static_cast<uint64_t>(deltaPositionMilliseconds), [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_SkipForward failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, mediaPlayback_skipBackward)
(JNIEnv * env, jobject, jlong deltaPositionMilliseconds, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD mediaPlayback_skipBackward called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(MediaPlayback_SkipBackward);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->MediaPlayback_SkipBackward(
        static_cast<uint64_t>(deltaPositionMilliseconds), [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.MediaPlayback_SkipBackward failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, applicationLauncher_launchApp)
(JNIEnv * env, jobject, jshort catalogVendorId, jstring applicationId, jbyteArray data, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD applicationLauncher_launchApp called");

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId      = static_cast<uint16_t>(catalogVendorId);
    const char * nativeApplicationId = env->GetStringUTFChars(applicationId, 0);
    application.applicationId        = CharSpan::fromCharString(nativeApplicationId);
    JniByteArray dataByteArray(env, data);

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(ApplicationLauncher_LaunchApp);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationLauncher_LaunchApp(
        application, chip::MakeOptional(dataByteArray.byteSpan()),
        [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
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

JNI_METHOD(jboolean, applicationLauncher_stopApp)
(JNIEnv * env, jobject, jshort catalogVendorId, jstring applicationId, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD applicationLauncher_stopApp called");

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId      = static_cast<uint16_t>(catalogVendorId);
    const char * nativeApplicationId = env->GetStringUTFChars(applicationId, 0);
    application.applicationId        = CharSpan::fromCharString(nativeApplicationId);

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(ApplicationLauncher_StopApp);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationLauncher_StopApp(
        application, [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
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

JNI_METHOD(jboolean, applicationLauncher_hideApp)
(JNIEnv * env, jobject, jshort catalogVendorId, jstring applicationId, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD applicationLauncher_hideApp called");

    chip::app::Clusters::ApplicationLauncher::Structs::Application::Type application;
    application.catalogVendorId      = static_cast<uint16_t>(catalogVendorId);
    const char * nativeApplicationId = env->GetStringUTFChars(applicationId, 0);
    application.applicationId        = CharSpan::fromCharString(nativeApplicationId);

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(ApplicationLauncher_HideApp);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ApplicationLauncher_HideApp(
        application, [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
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

JNI_METHOD(jboolean, targetNavigator_navigateTarget)
(JNIEnv * env, jobject, jbyte target, jstring data, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD targetNavigator_navigateTarget called");

    const char * nativeData = env->GetStringUTFChars(data, 0);

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(TargetNavigator_NavigateTarget);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->TargetNavigator_NavigateTarget(
        static_cast<uint8_t>(target), chip::MakeOptional(CharSpan::fromCharString(nativeData)),
        [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
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

JNI_METHOD(jboolean, keypadInput_sendKey)
(JNIEnv * env, jobject, jbyte keyCode, jobject jResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD keypadInput_sendKey called");

    MatterCallbackHandlerJNI responseHandler = TvCastingAppJNIMgr().getMediaCommandResponseHandler(KeypadInput_SendKey);
    CHIP_ERROR err                           = responseHandler.SetUp(env, jResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI.SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->KeypadInput_SendKey(static_cast<chip::app::Clusters::KeypadInput::CecKeyCode>(keyCode),
                                                            [&responseHandler](CHIP_ERROR err) { responseHandler.Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer.KeypadInput_SendKey failed %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}
