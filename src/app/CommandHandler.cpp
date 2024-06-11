/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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
#include <app/CommandHandler.h>

namespace chip {
namespace app {

void CommandHandler::Handle::Init(CommandHandler * handler)
{
    if (handler != nullptr)
    {
        handler->IncrementHoldOff(this);
        mpHandler = handler;
    }
}

CommandHandler * CommandHandler::Handle::Get()
{
    // Not safe to work with CommandHandlerImpl in parallel with other Matter work.
    assertChipStackLockedByCurrentThread();

    return mpHandler;
}

void CommandHandler::Handle::Release()
{
    if (mpHandler != nullptr)
    {
        mpHandler->DecrementHoldOff(this);
        Invalidate();
    }
}

CommandHandler::Handle::Handle(CommandHandler * handler)
{
    Init(handler);
}

} // namespace app
} // namespace chip
