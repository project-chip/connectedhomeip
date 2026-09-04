/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <posix/named_pipe/Dispatcher.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

namespace chip::app::NamedPipe {

namespace {

struct CommandContext
{
    Json::Value json;
    Dispatcher * dispatcher;
};

} // namespace

Dispatcher & Dispatcher::Instance()
{
    static Dispatcher instance(OOBAccessorRegistry::Instance());
    return instance;
}

Dispatcher::~Dispatcher()
{
    LogErrorOnFailure(Stop());
}

CHIP_ERROR Dispatcher::Start(const char * fifoPath)
{
    VerifyOrReturnError(fifoPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return mNamedPipeCommands.Start(fifoPath, this);
}

CHIP_ERROR Dispatcher::Stop()
{
    return mNamedPipeCommands.Stop();
}

bool Dispatcher::HasTranslator(CharSpan actionName) const
{
    return mTranslators.find(std::string(actionName.data(), actionName.size())) != mTranslators.end();
}

CHIP_ERROR Dispatcher::RegisterTranslator(CharSpan actionName, std::shared_ptr<CommandTranslator> translator)
{
    VerifyOrReturnError(translator != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mTranslators[std::string(actionName.data(), actionName.size())] = std::move(translator);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Dispatcher::DispatchJson(const Json::Value & json)
{
    if (!json.isObject() || !json.isMember("Name") || !json["Name"].isString())
    {
        ChipLogError(AppServer, "NamedPipe::Dispatcher: Missing or invalid command Name");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    EndpointId targetEndpoint = kRootEndpointId;
    if (json.isMember("EndpointId"))
    {
        auto endpointId = CommandTranslator::ExtractUInt<EndpointId>(json, "EndpointId");
        if (!endpointId.has_value())
        {
            ChipLogError(AppServer, "NamedPipe::Dispatcher: Invalid EndpointId format");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        targetEndpoint = *endpointId;
    }

    std::string actionName = json["Name"].asString();
    auto it                = mTranslators.find(actionName);
    if (it == mTranslators.end())
    {
        ChipLogError(AppServer, "NamedPipe::Dispatcher: Unhandled action: %s", actionName.c_str());
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR err = it->second->TranslateAndExecute(targetEndpoint, json, mOobRegistry);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "NamedPipe::Dispatcher: Failed to execute action %s on endpoint %u: %" CHIP_ERROR_FORMAT,
                     actionName.c_str(), targetEndpoint, err.Format());
    }
    return err;
}

void Dispatcher::OnEventCommandReceived(const char * json)
{
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(json, value))
    {
        ChipLogError(AppServer, "NamedPipe::Dispatcher: Failed to parse JSON command: %s",
                     reader.getFormattedErrorMessages().c_str());
        return;
    }

    auto * context = Platform::New<CommandContext>();
    if (context == nullptr)
    {
        ChipLogError(AppServer, "NamedPipe::Dispatcher: Memory allocation failed for command context");
        return;
    }
    context->json       = std::move(value);
    context->dispatcher = this;

    CHIP_ERROR err = DeviceLayer::PlatformMgr().ScheduleWork(DispatchCommand, reinterpret_cast<intptr_t>(context));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "NamedPipe::Dispatcher: Failed to schedule work: %" CHIP_ERROR_FORMAT, err.Format());
        Platform::Delete(context);
    }
}

void Dispatcher::DispatchCommand(intptr_t context)
{
    auto * cmd = reinterpret_cast<CommandContext *>(context);
    if (cmd != nullptr)
    {
        LogErrorOnFailure(cmd->dispatcher->DispatchJson(cmd->json));
        Platform::Delete(cmd);
    }
}

} // namespace chip::app::NamedPipe
