/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "SleepCommand.h"
#include <chrono>
#include <thread>

CHIP_ERROR SleepCommand::RunCommand()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(mDurationInMs));
    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}
