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
#include <lib/support/StringBuilder.h>

#include <app-common/zap-generated/tests/simulated-cluster-objects.h>

class SystemCommands
{
public:
    SystemCommands(){};
    virtual ~SystemCommands(){};

    virtual CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err) = 0;

    CHIP_ERROR Start(const char * identity, const chip::app::Clusters::SystemCommands::Commands::Start::Type & value);
    CHIP_ERROR Stop(const char * identity, const chip::app::Clusters::SystemCommands::Commands::Stop::Type & value);
    CHIP_ERROR Reboot(const char * identity, const chip::app::Clusters::SystemCommands::Commands::Reboot::Type & value);
    CHIP_ERROR FactoryReset(const char * identity, const chip::app::Clusters::SystemCommands::Commands::FactoryReset::Type & value);
    CHIP_ERROR CreateOtaImage(const char * identity,
                              const chip::app::Clusters::SystemCommands::Commands::CreateOtaImage::Type & value);
    CHIP_ERROR CompareFiles(const char * identity, const chip::app::Clusters::SystemCommands::Commands::CompareFiles::Type & value);

private:
    CHIP_ERROR RunInternal(const char * scriptName, const chip::Optional<chip::CharSpan> registerKey);
    CHIP_ERROR RunInternal(const char * command);
    CHIP_ERROR AddSystemCommandArgument(chip::StringBuilderBase & builder, const char * argName, const chip::CharSpan & argValue);
};
