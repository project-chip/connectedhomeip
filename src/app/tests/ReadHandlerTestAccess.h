/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/ReadHandler.h>

using namespace chip::app;

namespace chip {
namespace Test {

/**
 * @brief Class acts as an accessor to private methods of the ReadHandler class without needing to give friend access to
 *        each individual test.
 *        This is not a Global API and should only be used for (Unit) Testing.
 */
class ReadHandlerTestAccess
{
public:
    ReadHandlerTestAccess(ReadHandler * aReadHandler) : pReadHandler(aReadHandler) {}

    void OnInitialRequest(System::PacketBufferHandle && aPayload)
    {
        if (pReadHandler != nullptr)
        {
            pReadHandler->OnInitialRequest(std::move(aPayload));
        }
    }
    ReadHandler * GetReadHandler() { return pReadHandler; }

private:
    ReadHandler * pReadHandler = nullptr;
};

} // namespace Test
} // namespace chip
