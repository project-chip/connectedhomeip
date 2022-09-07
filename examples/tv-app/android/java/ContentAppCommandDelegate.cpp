/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @brief Contains Implementation of the ContentAppCommandDelegate
 */

#include "ContentAppCommandDelegate.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/jsontlv/TlvJson.h>
#include <zap-generated/endpoint_config.h>

namespace chip {
namespace AppPlatform {

using CommandHandlerInterface = chip::app::CommandHandlerInterface;
using LaunchResponseType      = chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::Type;

void ContentAppCommandDelegate::InvokeCommand(CommandHandlerInterface::HandlerContext & handlerContext)
{
    if (handlerContext.mRequestPath.mEndpointId >= FIXED_ENDPOINT_COUNT)
    {
        TLV::TLVReader readerForJson;
        readerForJson.Init(handlerContext.mPayload);

        CHIP_ERROR err = CHIP_NO_ERROR;
        Json::Value json;
        err = TlvToJson(readerForJson, json);
        if (err != CHIP_NO_ERROR)
        {
            // TODO : Add an interface to let the apps know a message came but there was a serialization error.
            handlerContext.SetCommandNotHandled();
            return;
        }

        JNIEnv * env      = JniReferences::GetInstance().GetEnvForCurrentThread();
        Json::Value value = json["value"];
        UtfString jsonString(env, JsonToString(value).c_str());

        ChipLogProgress(Zcl, "ContentAppCommandDelegate::InvokeCommand send command being called with payload %s",
                        JsonToString(json).c_str());

        jstring resp = (jstring) env->CallObjectMethod(
            mContentAppEndpointManager, mSendCommandMethod, static_cast<jint>(handlerContext.mRequestPath.mEndpointId),
            static_cast<jint>(handlerContext.mRequestPath.mClusterId), static_cast<jint>(handlerContext.mRequestPath.mCommandId),
            jsonString.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ContentAppCommandDelegate::sendCommand");
            env->ExceptionDescribe();
            env->ExceptionClear();
            FormatResponseData(handlerContext, "{\"value\":{}}");
            return;
        }
        const char * respStr = env->GetStringUTFChars(resp, 0);
        ChipLogProgress(Zcl, "ContentAppCommandDelegate::InvokeCommand got response %s", respStr);
        FormatResponseData(handlerContext, respStr);
    }
    else
    {
        handlerContext.SetCommandNotHandled();
    }
}

void ContentAppCommandDelegate::FormatResponseData(CommandHandlerInterface::HandlerContext & handlerContext, const char * response)
{
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(response, value))
    {
        handlerContext.SetCommandNotHandled();
        return;
    }

    switch (handlerContext.mRequestPath.mClusterId)
    {
    case app::Clusters::ContentLauncher::Id: {
        LaunchResponseType launchResponse;
        if (value["0"].empty())
        {
            launchResponse.status = chip::app::Clusters::ContentLauncher::ContentLaunchStatusEnum::kAuthFailed;
        }
        else
        {
            launchResponse.status = static_cast<chip::app::Clusters::ContentLauncher::ContentLaunchStatusEnum>(value["0"].asInt());
            if (!value["1"].empty())
            {
                launchResponse.data = chip::MakeOptional(CharSpan::fromCharString(value["1"].asCString()));
            }
        }
        handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, launchResponse);
        handlerContext.SetCommandHandled();
        break;
    }

    // case app::Clusters::TargetNavigator::Id:
    //     break;

    // case app::Clusters::MediaPlayback::Id:
    //     break;

    // case app::Clusters::AccountLogin::Id:
    //     break;
    default:
        handlerContext.SetCommandNotHandled();
    }
}

} // namespace AppPlatform
} // namespace chip
