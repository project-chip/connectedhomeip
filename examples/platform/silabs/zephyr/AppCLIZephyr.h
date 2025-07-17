/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/CHIPError.h>

namespace chip {
namespace Zephyr {
namespace App {

class AppCLIZephyr
{
public:
    virtual ~AppCLIZephyr() = default;

    virtual CHIP_ERROR Init(void);

    virtual void ResetCmdHandle(void);

    /**
     * \brief Registers common CLI commands used by application
     */
    void RegisterDefaultCommands();

    static AppCLIZephyr & GetDefaultInstance();

private:
    bool isShellInitialized = false;
};
AppCLIZephyr & GetAppCLI();
} // namespace App
} // namespace Zephyr
} // namespace chip
