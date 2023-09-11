/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
