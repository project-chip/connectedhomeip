/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "InteractiveServer.h"

#include <json/json.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::DeviceLayer;

namespace {
constexpr const char * kClusterIdKey                = "clusterId";
constexpr const char * kEndpointIdKey               = "endpointId";
constexpr const char * kAttributeIdKey              = "attributeId";
constexpr const char * kWaitTypeKey                 = "waitType";
constexpr const char * kAttributeWriteKey           = "writeAttribute";
constexpr const char * kAttributeReadKey            = "readAttribute";
constexpr const char * kCommandIdKey                = "commandId";
constexpr const char * kWaitForCommissioningCommand = "WaitForCommissioning";

std::string JsonToString(Json::Value & json)
{
    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    return writer.write(json);
}

void OnPlatformEvent(const ChipDeviceEvent * event, intptr_t arg);

void OnCommissioningComplete(intptr_t context)
{
    PlatformMgr().RemoveEventHandler(OnPlatformEvent);
    InteractiveServer::GetInstance().CommissioningComplete();
}

void OnPlatformEvent(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kCommissioningComplete:
        PlatformMgr().ScheduleWork(OnCommissioningComplete, arg);
        break;
    }
}
} // namespace

InteractiveServer * InteractiveServer::instance = nullptr;
InteractiveServer & InteractiveServer::GetInstance()
{
    if (instance == nullptr)
    {
        instance = new InteractiveServer();
    }
    return *instance;
}

void InteractiveServer::Run(const chip::Optional<uint16_t> port)
{
    mIsReady = false;
    wsThread = std::thread(&WebSocketServer::Run, &mWebSocketServer, port, this);
}

bool InteractiveServer::OnWebSocketMessageReceived(char * msg)
{
    ChipLogError(chipTool, "Receive message: %s", msg);
    if (strcmp(msg, kWaitForCommissioningCommand) == 0)
    {
        mIsReady = false;
        PlatformMgr().AddEventHandler(OnPlatformEvent);
    }
    else
    {
        mIsReady = true;
    }
    return true;
}

bool InteractiveServer::Command(const chip::app::ConcreteCommandPath & path)
{
    VerifyOrReturnValue(mIsReady, false);

    Json::Value value;
    value[kClusterIdKey]  = path.mClusterId;
    value[kEndpointIdKey] = path.mEndpointId;
    value[kCommandIdKey]  = path.mCommandId;

    auto valueStr = JsonToString(value);
    LogErrorOnFailure(mWebSocketServer.Send(valueStr.c_str()));
    return mIsReady;
}

bool InteractiveServer::ReadAttribute(const chip::app::ConcreteAttributePath & path)
{
    VerifyOrReturnValue(mIsReady, false);

    Json::Value value;
    value[kClusterIdKey]   = path.mClusterId;
    value[kEndpointIdKey]  = path.mEndpointId;
    value[kAttributeIdKey] = path.mAttributeId;
    value[kWaitTypeKey]    = kAttributeReadKey;

    auto valueStr = JsonToString(value);
    LogErrorOnFailure(mWebSocketServer.Send(valueStr.c_str()));
    return mIsReady;
}

bool InteractiveServer::WriteAttribute(const chip::app::ConcreteAttributePath & path)
{
    VerifyOrReturnValue(mIsReady, false);

    Json::Value value;
    value[kClusterIdKey]   = path.mClusterId;
    value[kEndpointIdKey]  = path.mEndpointId;
    value[kAttributeIdKey] = path.mAttributeId;
    value[kWaitTypeKey]    = kAttributeWriteKey;

    auto valueStr = JsonToString(value);
    LogErrorOnFailure(mWebSocketServer.Send(valueStr.c_str()));
    return mIsReady;
}

void InteractiveServer::CommissioningComplete()
{
    VerifyOrReturn(!mIsReady);
    mIsReady = true;

    Json::Value value = Json::objectValue;
    auto valueStr     = JsonToString(value);
    LogErrorOnFailure(mWebSocketServer.Send(valueStr.c_str()));
}
