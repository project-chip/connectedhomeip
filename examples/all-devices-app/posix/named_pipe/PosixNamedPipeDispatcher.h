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

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <NamedPipeCommands.h>
#include <json/json.h>
#include <lib/core/CHIPError.h>
#include <oob-accessors/OOBAccessorRegistry.h>
#include <posix/named_pipe/NamedPipeCommandTranslator.h>

namespace chip::app {

class PosixNamedPipeDispatcher : public NamedPipeCommandDelegate
{
public:
    static PosixNamedPipeDispatcher & Instance();

    explicit PosixNamedPipeDispatcher(OOBAccessorRegistry & oobRegistry) : mOobRegistry(oobRegistry) {}
    ~PosixNamedPipeDispatcher() override;

    /**
     * @brief Starts listening on the named pipe FIFO.
     * @param fifoPath Path to the named pipe file.
     */
    CHIP_ERROR Start(const char * fifoPath);

    /**
     * @brief Stops listening on the named pipe and cleans up the FIFO file.
     */
    CHIP_ERROR Stop();

    /**
     * @brief Checks if a translator is registered for the specified action name.
     */
    bool HasTranslator(CharSpan actionName) const;

    /**
     * @brief Registers a command translator instance under the specified action name.
     */
    CHIP_ERROR RegisterTranslator(CharSpan actionName, std::shared_ptr<NamedPipeCommandTranslator> translator);

    /**
     * @brief Registers a translator if not already present, registering all action names exposed by TranslatorType.
     */
    template <typename TranslatorType>
    CHIP_ERROR EnsureTranslatorRegistered()
    {
        auto actionNames = TranslatorType::GetActionNames();
        if (actionNames.empty())
        {
            return CHIP_NO_ERROR;
        }
        bool anyMissing = false;
        for (const auto & name : actionNames)
        {
            if (!HasTranslator(name))
            {
                anyMissing = true;
                break;
            }
        }
        if (!anyMissing)
        {
            return CHIP_NO_ERROR;
        }
        auto translator = std::make_shared<TranslatorType>();
        for (const auto & name : actionNames)
        {
            if (!HasTranslator(name))
            {
                ReturnErrorOnFailure(RegisterTranslator(name, translator));
            }
        }
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Parses and dispatches a JSON command to the registered translator.
     */
    CHIP_ERROR DispatchJson(const Json::Value & json);

    // NamedPipeCommandDelegate implementation
    void OnEventCommandReceived(const char * json) override;

private:
    static void DispatchCommand(intptr_t context);

    OOBAccessorRegistry & mOobRegistry;
    NamedPipeCommands mNamedPipeCommands;
    std::unordered_map<std::string, std::shared_ptr<NamedPipeCommandTranslator>> mTranslators;
};

} // namespace chip::app
