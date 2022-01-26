/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

namespace chip {
namespace Shell {

class OnOffCommands
{
public:
    // delete the copy constructor
    OnOffCommands(const OnOffCommands &) = delete;
    // delete the move constructor
    OnOffCommands(OnOffCommands &&) = delete;
    // delete the assignment operator
    OnOffCommands & operator=(const OnOffCommands &) = delete;

    static OnOffCommands & GetInstance()
    {
        static OnOffCommands instance;
        return instance;
    }

    // Register the OTA provider commands
    void Register();

private:
    OnOffCommands() {}
};

} // namespace Shell
} // namespace chip
