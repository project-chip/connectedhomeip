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
#include <app/util/config.h>
#include <jni.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/jsontlv/TlvJson.h>
#include <platform/PlatformManager.h>
#include <zap-generated/endpoint_config.h>

namespace chip {
namespace AppPlatform {

using CommandHandlerInterface    = chip::app::CommandHandlerInterface;
using LaunchResponseType         = chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::Type;
using PlaybackResponseType       = chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type;
using NavigateTargetResponseType = chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Type;
using GetSetupPINResponseType    = chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Type;
using Status                     = chip::Protocols::InteractionModel::Status;

const std::string FAILURE_KEY        = "PlatformError";
const std::string FAILURE_STATUS_KEY = "Status";

void ContentAppCommandDelegate::InvokeCommand(CommandHandlerInterface::HandlerContext & handlerContext)
{
    if (handlerContext.mRequestPath.mEndpointId >= FIXED_ENDPOINT_COUNT)
    {
        DeviceLayer::StackUnlock unlock;
        TLV::TLVReader readerForJson;
        readerForJson.Init(handlerContext.mPayload);

        CHIP_ERROR err = CHIP_NO_ERROR;
        Json::Value json;
        err = TlvToJson(readerForJson, json);
        if (err != CHIP_NO_ERROR)
        {
            handlerContext.SetCommandHandled();
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                     chip::Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        JNIEnv * env        = JniReferences::GetInstance().GetEnvForCurrentThread();
        Json::Value value   = json["value"];
        std::string payload = JsonToString(value);
        UtfString jsonString(env, payload.c_str());

        if (!mContentAppEndpointManager.HasValidObjectRef())
        {
            ChipLogProgress(Zcl, "mContentAppEndpointManager is not valid");
            return;
        }

        jstring resp = static_cast<jstring>(env->CallObjectMethod(
            mContentAppEndpointManager.ObjectRef(), mSendCommandMethod, static_cast<jint>(handlerContext.mRequestPath.mEndpointId),
            static_cast<jlong>(handlerContext.mRequestPath.mClusterId), static_cast<jlong>(handlerContext.mRequestPath.mCommandId),
            jsonString.jniValue()));
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ContentAppCommandDelegate::sendCommand");
            env->ExceptionDescribe();
            env->ExceptionClear();
            FormatResponseData(handlerContext, "{\"value\":{}}");
        }
        else
        {
            JniUtfString respStr(env, resp);
            ChipLogProgress(Zcl, "ContentAppCommandDelegate::InvokeCommand got response %s", respStr.c_str());
            FormatResponseData(handlerContext, respStr.c_str());
        }
        env->DeleteLocalRef(resp);
    }
    else
    {
        handlerContext.SetCommandNotHandled();
    }
}

Status ContentAppCommandDelegate::InvokeCommand(EndpointId epId, ClusterId clusterId, CommandId commandId, std::string payload,
                                                bool & commandHandled, Json::Value & value)
{
    if (epId >= FIXED_ENDPOINT_COUNT)
    {
        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        UtfString jsonString(env, payload.c_str());

        ChipLogProgress(Zcl, "ContentAppCommandDelegate::InvokeCommand send command being called with payload %s", payload.c_str());

        if (!mContentAppEndpointManager.HasValidObjectRef())
        {
            return chip::Protocols::InteractionModel::Status::Failure;
        }

        jstring resp =
            (jstring) env->CallObjectMethod(mContentAppEndpointManager.ObjectRef(), mSendCommandMethod, static_cast<jint>(epId),
                                            static_cast<jlong>(clusterId), static_cast<jlong>(commandId), jsonString.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in ContentAppCommandDelegate::sendCommand");
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        else
        {
            JniUtfString respStr(env, resp);
            ChipLogProgress(Zcl, "ContentAppCommandDelegate::InvokeCommand got response %s", respStr.c_str());

            Json::Reader reader;
            if (!reader.parse(respStr.c_str(), value))
            {
                env->DeleteLocalRef(resp);
                return chip::Protocols::InteractionModel::Status::Failure;
            }
        }
        env->DeleteLocalRef(resp);

        // handle errors from platform-app
        if (!value[FAILURE_KEY].empty())
        {
            value = value[FAILURE_KEY];
            if (!value[FAILURE_STATUS_KEY].empty() && value[FAILURE_STATUS_KEY].isUInt())
            {
                return static_cast<Protocols::InteractionModel::Status>(value[FAILURE_STATUS_KEY].asUInt());
            }
            return chip::Protocols::InteractionModel::Status::Failure;
        }

        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    else
    {
        commandHandled = false;
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
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
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::Failure);
        return;
    }

    switch (handlerContext.mRequestPath.mClusterId)
    {
    case app::Clusters::ContentLauncher::Id: {
        Status status;
        LaunchResponseType launchResponse = FormatContentLauncherResponse(value, status);
        if (status != chip::Protocols::InteractionModel::Status::Success)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, status);
        }
        else
        {
            handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, launchResponse);
        }
        break;
    }

    case app::Clusters::TargetNavigator::Id: {
        Status status;
        NavigateTargetResponseType navigateTargetResponse = FormatNavigateTargetResponse(value, status);
        if (status != chip::Protocols::InteractionModel::Status::Success)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, status);
        }
        else
        {
            handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, navigateTargetResponse);
        }
        break;
    }

    case app::Clusters::MediaPlayback::Id: {
        Status status;
        PlaybackResponseType playbackResponse = FormatMediaPlaybackResponse(value, status);
        if (status != chip::Protocols::InteractionModel::Status::Success)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, status);
        }
        else
        {
            handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, playbackResponse);
        }
        break;
    }

    case app::Clusters::AccountLogin::Id: {
        if (app::Clusters::AccountLogin::Commands::GetSetupPIN::Id != handlerContext.mRequestPath.mCommandId)
        {
            // No response for other commands in this cluster
            break;
        }
        Status status;
        GetSetupPINResponseType getSetupPINresponse = FormatGetSetupPINResponse(value, status);
        if (status != chip::Protocols::InteractionModel::Status::Success)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, status);
        }
        else
        {
            handlerContext.mCommandHandler.AddResponseData(handlerContext.mRequestPath, getSetupPINresponse);
        }
        break;
    }
    default:
        handlerContext.SetCommandNotHandled();
    }
}

LaunchResponseType ContentAppCommandDelegate::FormatContentLauncherResponse(Json::Value value, Status & status)
{
    status = chip::Protocols::InteractionModel::Status::Success;
    LaunchResponseType launchResponse;
    std::string statusFieldId =
        std::to_string(to_underlying(app::Clusters::ContentLauncher::Commands::LauncherResponse::Fields::kStatus));
    if (value[statusFieldId].empty())
    {
        status = chip::Protocols::InteractionModel::Status::Failure;
        return launchResponse;
    }
    else
    {
        launchResponse.status = static_cast<app::Clusters::ContentLauncher::StatusEnum>(value[statusFieldId].asInt());
        std::string dataFieldId =
            std::to_string(to_underlying(app::Clusters::ContentLauncher::Commands::LauncherResponse::Fields::kData));
        if (!value[dataFieldId].empty())
        {
            launchResponse.data = chip::MakeOptional(CharSpan::fromCharString(value[dataFieldId].asCString()));
        }
    }
    return launchResponse;
}

NavigateTargetResponseType ContentAppCommandDelegate::FormatNavigateTargetResponse(Json::Value value, Status & status)
{
    status = chip::Protocols::InteractionModel::Status::Success;
    NavigateTargetResponseType navigateTargetResponse;
    std::string statusFieldId =
        std::to_string(to_underlying(app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Fields::kStatus));
    if (value[statusFieldId].empty())
    {
        status = chip::Protocols::InteractionModel::Status::Failure;
        return navigateTargetResponse;
    }
    else
    {
        navigateTargetResponse.status = static_cast<app::Clusters::TargetNavigator::StatusEnum>(value[statusFieldId].asInt());
        std::string dataFieldId =
            std::to_string(to_underlying(app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Fields::kData));
        if (!value[dataFieldId].empty())
        {
            navigateTargetResponse.data = chip::MakeOptional(CharSpan::fromCharString(value[dataFieldId].asCString()));
        }
    }
    return navigateTargetResponse;
}

PlaybackResponseType ContentAppCommandDelegate::FormatMediaPlaybackResponse(Json::Value value, Status & status)
{
    status = chip::Protocols::InteractionModel::Status::Success;
    PlaybackResponseType playbackResponse;
    std::string statusFieldId =
        std::to_string(to_underlying(app::Clusters::MediaPlayback::Commands::PlaybackResponse::Fields::kStatus));
    if (value[statusFieldId].empty())
    {
        status = chip::Protocols::InteractionModel::Status::Failure;
        return playbackResponse;
    }
    else
    {
        playbackResponse.status = static_cast<app::Clusters::MediaPlayback::StatusEnum>(value[statusFieldId].asInt());
        std::string dataFieldId =
            std::to_string(to_underlying(app::Clusters::MediaPlayback::Commands::PlaybackResponse::Fields::kData));
        if (!value[dataFieldId].empty())
        {
            playbackResponse.data = chip::MakeOptional(CharSpan::fromCharString(value[dataFieldId].asCString()));
        }
    }
    return playbackResponse;
}

GetSetupPINResponseType ContentAppCommandDelegate::FormatGetSetupPINResponse(Json::Value value, Status & status)
{
    status = chip::Protocols::InteractionModel::Status::Success;
    GetSetupPINResponseType getSetupPINresponse;
    std::string setupPINFieldId =
        std::to_string(to_underlying(app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Fields::kSetupPIN));
    if (!value[setupPINFieldId].empty())
    {
        getSetupPINresponse.setupPIN = CharSpan::fromCharString(value[setupPINFieldId].asCString());
    }
    else
    {
        getSetupPINresponse.setupPIN = "";
    }
    return getSetupPINresponse;
}

} // namespace AppPlatform
} // namespace chip
