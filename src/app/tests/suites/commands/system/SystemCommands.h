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

#pragma once

#include <lib/support/CodeUtils.h>

class SystemCommands
{
public:
    SystemCommands(){};
    virtual ~SystemCommands(){};

    virtual CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err) = 0;

    CHIP_ERROR Start(uint16_t discriminator = 0xFFFF, uint16_t port = CHIP_PORT, const char * kvs = nullptr);
    CHIP_ERROR Stop();
    CHIP_ERROR Reboot(uint16_t discriminator = 0xFFFF, uint16_t port = CHIP_PORT, const char * kvs = nullptr);
    CHIP_ERROR FactoryReset();

private:
    CHIP_ERROR RunInternal(const char * command);
    CHIP_ERROR CreateCommonCommandArgs(char * commandBuffer, size_t commandBufferSize, const char * scriptDir,
                                       const char * scriptName, uint16_t discriminator, uint16_t port, const char * kvs);
};
