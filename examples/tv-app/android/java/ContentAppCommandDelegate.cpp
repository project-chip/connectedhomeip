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

using CommandHandlerInterface    = chip::app::CommandHandlerInterface;
using LaunchResponseType         = chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::Type;
using PlaybackResponseType       = chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type;
using NavigateTargetResponseType = chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Type;

const std::string FAILURE_KEY        = "PlatformError";
const std::string FAILURE_STATUS_KEY = "Status";

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
            handlerContext.SetCommandHandled();
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                     Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        JNIEnv * env        = JniReferences::GetInstance().GetEnvForCurrentThread();
        Json::Value value   = json["value"];
        std::string payload = JsonToString(value);
        UtfString jsonString(env, payload.c_str());

        ChipLogProgress(Zcl, "ContentAppCommandDelegate::InvokeCommand send command being called with payload %s", payload.c_str());

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
        env->ReleaseStringUTFChars(resp, respStr);
        env->DeleteLocalRef(resp);
    }
    else
    {
        handlerContext.SetCommandNotHandled();
    }
}

void ContentAppCommandDelegate::FormatResponseData(CommandHandlerInterface::HandlerContext & handlerContext, const char * response)
{
    handlerContext.SetCommandHandled();
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(response, value))
    {
        return;
    }

    // handle errors from platform-app
    if (!value[FAILURE_KEY].empty())
    {
        value = value[FAILURE_KEY];
        if (!value[FAILURE_STATUS_KEY].empty() && value[FAILURE_STATUS_KEY].isUInt())
        {
            handlerContext.mCommandHandler.AddStatus(
                handlerContext.mRequestPath, static_cast<Protocols::InteractionModel::Status>(value[FAILURE_STATUS_KEY].asUInt()));
            return;
        }
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::Failure);
        return;
    }

    switch (handlerContext.mRequestPath.mClusterId)
    {
    case app::Clusters::ContentLauncher::Id: {
        LaunchResponseType launchResponse;
        std::string statusFieldId =
            std::to_string(to_underlying(app::Clusters::ContentLauncher::Commands::LaunchResponse::Fields::kStatus));
        if (value[statusFieldId].empty())
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::Failure);
            return;
        }
        else
        {
            launchResponse.status =
                static_cast<app::Clusters::ContentLauncher::ContentLaunchStatusEnum>(value[statusFieldId].asInt());
            std::string dataFieldId =
                std::to_string(to_underlying(app::Clusters::ContentLauncher::Commands::LaunchResponse::Fields::kData));
            if (!value[dataFieldId].empty())
            {
                launchResponse.data = chip::MakeOptional(CharSpan::fromCharString(value[dataFieldId].asCString()));
            }
        }
        handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, launchResponse);
        break;
    }

    case app::Clusters::TargetNavigator::Id: {
        NavigateTargetResponseType navigateTargetResponse;
        std::string statusFieldId =
            std::to_string(to_underlying(app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Fields::kStatus));
        if (value[statusFieldId].empty())
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::Failure);
            return;
        }
        else
        {
            navigateTargetResponse.status =
                static_cast<app::Clusters::TargetNavigator::TargetNavigatorStatusEnum>(value[statusFieldId].asInt());
            std::string dataFieldId =
                std::to_string(to_underlying(app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Fields::kData));
            if (!value[dataFieldId].empty())
            {
                navigateTargetResponse.data = chip::MakeOptional(CharSpan::fromCharString(value[dataFieldId].asCString()));
            }
        }
        handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, navigateTargetResponse);
        break;
    }

    case app::Clusters::MediaPlayback::Id: {
        PlaybackResponseType playbackResponse;
        std::string statusFieldId =
            std::to_string(to_underlying(app::Clusters::MediaPlayback::Commands::PlaybackResponse::Fields::kStatus));
        if (value[statusFieldId].empty())
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::Failure);
            return;
        }
        else
        {
            playbackResponse.status =
                static_cast<app::Clusters::MediaPlayback::MediaPlaybackStatusEnum>(value[statusFieldId].asInt());
            std::string dataFieldId =
                std::to_string(to_underlying(app::Clusters::MediaPlayback::Commands::PlaybackResponse::Fields::kData));
            if (!value[dataFieldId].empty())
            {
                playbackResponse.data = chip::MakeOptional(CharSpan::fromCharString(value[dataFieldId].asCString()));
            }
        }
        handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, playbackResponse);
        break;
    }

    // case app::Clusters::AccountLogin::Id:
    //     break;
    default:
        handlerContext.SetCommandNotHandled();
    }
}

} // namespace AppPlatform
} // namespace chip
