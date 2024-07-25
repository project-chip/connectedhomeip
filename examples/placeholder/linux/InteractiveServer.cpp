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
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/Base64.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/logging/LogV.h>

#include <string>
#include <vector>

using namespace chip::DeviceLayer;

namespace {
constexpr char kClusterIdKey[]                = "clusterId";
constexpr char kEndpointIdKey[]               = "endpointId";
constexpr char kAttributeIdKey[]              = "attributeId";
constexpr char kWaitTypeKey[]                 = "waitType";
constexpr char kAttributeWriteKey[]           = "writeAttribute";
constexpr char kAttributeReadKey[]            = "readAttribute";
constexpr char kCommandIdKey[]                = "commandId";
constexpr char kWaitForCommissioningCommand[] = "WaitForCommissioning";
constexpr char kCategoryError[]               = "Error";
constexpr char kCategoryProgress[]            = "Info";
constexpr char kCategoryDetail[]              = "Debug";
constexpr char kCategoryAutomation[]          = "Automation";

struct InteractiveServerResultLog
{
    std::string module;
    std::string message;
    std::string messageType;
};

struct InteractiveServerResult
{
    bool mEnabled = false;
    std::vector<std::string> mResults;
    std::vector<InteractiveServerResultLog> mLogs;

    void Setup() { mEnabled = true; }

    void Reset()
    {
        mEnabled = false;
        mResults.clear();
        mLogs.clear();
    }

    void MaybeAddLog(const char * module, uint8_t category, const char * base64Message)
    {
        VerifyOrReturn(mEnabled);

        const char * messageType = nullptr;
        switch (category)
        {
        case chip::Logging::kLogCategory_Error:
            messageType = kCategoryError;
            break;
        case chip::Logging::kLogCategory_Progress:
            messageType = kCategoryProgress;
            break;
        case chip::Logging::kLogCategory_Detail:
            messageType = kCategoryDetail;
            break;
        case chip::Logging::kLogCategory_Automation:
            messageType = kCategoryAutomation;
            break;
        default:
            chipDie();
            break;
        }

        mLogs.push_back(InteractiveServerResultLog({ module, base64Message, messageType }));
    }

    void MaybeAddResult(const char * result)
    {
        VerifyOrReturn(mEnabled);
        mResults.push_back(result);
    }

    std::string AsJsonString() const
    {
        std::string resultsStr;
        if (mResults.size())
        {
            for (const auto & result : mResults)
            {
                resultsStr = resultsStr + result + ",";
            }

            // Remove last comma.
            resultsStr.pop_back();
        }

        std::string logsStr;
        if (mLogs.size())
        {
            // Log messages are encoded in base64 already, so it is safe to append the message
            // between double quotes, even if the original log message contains some.
            for (const auto & log : mLogs)
            {
                logsStr = logsStr + "{";
                logsStr = logsStr + "  \"module\": \"" + log.module + "\",";
                logsStr = logsStr + "  \"category\": \"" + log.messageType + "\",";
                logsStr = logsStr + "  \"message\": \"" + log.message + "\"";
                logsStr = logsStr + "},";
            }

            // Remove last comma.
            logsStr.pop_back();
        }

        std::string jsonLog;
        jsonLog = jsonLog + "{";
        jsonLog = jsonLog + "  \"results\": [" + resultsStr + "],";
        jsonLog = jsonLog + "  \"logs\": [" + logsStr + "]";
        jsonLog = jsonLog + "}";

        return jsonLog;
    }
};

InteractiveServerResult gInteractiveServerResult;

void ENFORCE_FORMAT(3, 0) InteractiveServerLoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    chip::Logging::Platform::LogV(module, category, msg, args);

    char message[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    vsnprintf(message, sizeof(message), msg, args_copy);
    va_end(args_copy);

    char base64Message[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE * 2] = {};
    chip::Base64Encode(chip::Uint8::from_char(message), static_cast<uint16_t>(strlen(message)), base64Message);

    gInteractiveServerResult.MaybeAddLog(module, category, base64Message);
}

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

    chip::Logging::SetLogRedirectCallback(InteractiveServerLoggingCallback);
}

bool InteractiveServer::OnWebSocketMessageReceived(char * msg)
{
    ChipLogError(chipTool, "Receive message: %s", msg);
    gInteractiveServerResult.Setup();
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
    gInteractiveServerResult.MaybeAddResult(valueStr.c_str());
    mWebSocketServer.Send(gInteractiveServerResult.AsJsonString().c_str());
    gInteractiveServerResult.Reset();
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
    gInteractiveServerResult.MaybeAddResult(valueStr.c_str());
    mWebSocketServer.Send(gInteractiveServerResult.AsJsonString().c_str());
    gInteractiveServerResult.Reset();
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
    gInteractiveServerResult.MaybeAddResult(valueStr.c_str());
    mWebSocketServer.Send(gInteractiveServerResult.AsJsonString().c_str());
    gInteractiveServerResult.Reset();
    return mIsReady;
}

void InteractiveServer::CommissioningComplete()
{
    VerifyOrReturn(!mIsReady);
    mIsReady = true;

    Json::Value value = Json::objectValue;
    auto valueStr     = JsonToString(value);
    gInteractiveServerResult.MaybeAddResult(valueStr.c_str());
    mWebSocketServer.Send(gInteractiveServerResult.AsJsonString().c_str());
    gInteractiveServerResult.Reset();
}
