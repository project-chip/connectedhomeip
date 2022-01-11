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

#pragma once

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>

namespace chip {
namespace app {

template <typename CommandData>
class CommandResponseHelper
{
public:
    CommandResponseHelper(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath) :
        mCommand(command), mCommandPath(commandPath), responsed(false)
    {}

    CHIP_ERROR Success(const CommandData & response)
    {
        CHIP_ERROR err = mCommand->AddResponseData(mCommandPath, response);
        if (err == CHIP_NO_ERROR)
        {
            responsed = true;
        }
        return err;
    };

    void Response(EmberAfStatus status)
    {
        emberAfSendImmediateDefaultResponse(status);
        responsed = true;
    }

    bool IsResponsed() { return responsed; }

private:
    app::CommandHandler * mCommand;
    app::ConcreteCommandPath mCommandPath;
    bool responsed;
};

} // namespace app
} // namespace chip
