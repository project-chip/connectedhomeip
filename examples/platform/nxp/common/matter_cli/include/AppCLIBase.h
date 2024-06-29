/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
namespace NXP {
namespace App {

class AppCLIBase
{
public:
    virtual ~AppCLIBase() = default;
    /**
     * \brief API allowing to initialize the CLI
     *
     * This API can also register some common commands
     */
    virtual CHIP_ERROR Init(void) = 0;

    /**
     * \brief Registers common CLI commands used by application
     */
    void RegisterDefaultCommands(void);

    /**
     * \brief Handles CLI Reset command
     *
     * Abstract function since implementation could be different for each child class.
     */
    virtual void ResetCmdHandle(void) = 0;
};
extern AppCLIBase & GetAppCLI();
} // namespace App
} // namespace NXP
} // namespace chip
