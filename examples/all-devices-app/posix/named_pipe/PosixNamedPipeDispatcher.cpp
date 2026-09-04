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

#include <posix/named_pipe/PosixNamedPipeDispatcher.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

namespace chip::app {

namespace {

struct CommandContext
{
    Json::Value json;
    PosixNamedPipeDispatcher * dispatcher;
};

} // namespace

PosixNamedPipeDispatcher & PosixNamedPipeDispatcher::Instance()
{
    static PosixNamedPipeDispatcher instance(OOBAccessorRegistry::Instance());
    return instance;
}

PosixNamedPipeDispatcher::~PosixNamedPipeDispatcher()
{
    LogErrorOnFailure(Stop());
}

CHIP_ERROR PosixNamedPipeDispatcher::Start(const char * fifoPath)
{
    VerifyOrReturnError(fifoPath != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return mNamedPipeCommands.Start(fifoPath, this);
}

CHIP_ERROR PosixNamedPipeDispatcher::Stop()
{
    return mNamedPipeCommands.Stop();
}

bool PosixNamedPipeDispatcher::HasTranslator(CharSpan actionName) const
{
    return mTranslators.find(std::string(actionName.data(), actionName.size())) != mTranslators.end();
}

CHIP_ERROR PosixNamedPipeDispatcher::RegisterTranslator(CharSpan actionName,
                                                        std::shared_ptr<NamedPipeCommandTranslator> translator)
{
    VerifyOrReturnError(translator != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mTranslators[std::string(actionName.data(), actionName.size())] = std::move(translator);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PosixNamedPipeDispatcher::DispatchJson(const Json::Value & json)
{
    if (!json.isMember("Name") || !json["Name"].isString())
    {
        ChipLogError(AppServer, "PosixNamedPipeDispatcher: Missing or invalid command Name");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto endpointId = NamedPipeCommandTranslator::ExtractUInt<EndpointId>(json, "EndpointId");
    if (!endpointId.has_value())
    {
        ChipLogError(AppServer, "PosixNamedPipeDispatcher: Missing or invalid EndpointId");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::string actionName = json["Name"].asString();
    auto it                = mTranslators.find(actionName);
    if (it == mTranslators.end())
    {
        ChipLogError(AppServer, "PosixNamedPipeDispatcher: Unhandled action: %s", actionName.c_str());
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR err = it->second->TranslateAndExecute(*endpointId, json, mOobRegistry);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "PosixNamedPipeDispatcher: Failed to execute action %s on endpoint %u: %" CHIP_ERROR_FORMAT,
                     actionName.c_str(), *endpointId, err.Format());
    }
    return err;
}

void PosixNamedPipeDispatcher::OnEventCommandReceived(const char * json)
{
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(json, value))
    {
        ChipLogError(AppServer, "PosixNamedPipeDispatcher: Failed to parse JSON command: %s",
                     reader.getFormattedErrorMessages().c_str());
        return;
    }

    auto * context = Platform::New<CommandContext>();
    if (context == nullptr)
    {
        ChipLogError(AppServer, "PosixNamedPipeDispatcher: Memory allocation failed for command context");
        return;
    }
    context->json       = std::move(value);
    context->dispatcher = this;

    CHIP_ERROR err = DeviceLayer::PlatformMgr().ScheduleWork(DispatchCommand, reinterpret_cast<intptr_t>(context));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "PosixNamedPipeDispatcher: Failed to schedule work: %" CHIP_ERROR_FORMAT, err.Format());
        Platform::Delete(context);
    }
}

void PosixNamedPipeDispatcher::DispatchCommand(intptr_t context)
{
    auto * cmd = reinterpret_cast<CommandContext *>(context);
    if (cmd != nullptr)
    {
        LogErrorOnFailure(cmd->dispatcher->DispatchJson(cmd->json));
        Platform::Delete(cmd);
    }
}


} // namespace chip::app
