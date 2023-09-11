/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MediaBase.h"
#include "MediaCommandBase.h"

#include <controller/CHIPCluster.h>
#include <functional>

#include <app-common/zap-generated/cluster-objects.h>

// COMMAND CLASSES
class OnCommand : public MediaCommandBase<chip::app::Clusters::OnOff::Commands::On::Type, chip::app::DataModel::NullObjectType>
{
public:
    OnCommand() : MediaCommandBase(chip::app::Clusters::OnOff::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

class OffCommand : public MediaCommandBase<chip::app::Clusters::OnOff::Commands::Off::Type, chip::app::DataModel::NullObjectType>
{
public:
    OffCommand() : MediaCommandBase(chip::app::Clusters::OnOff::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

class ToggleCommand
    : public MediaCommandBase<chip::app::Clusters::OnOff::Commands::Toggle::Type, chip::app::DataModel::NullObjectType>
{
public:
    ToggleCommand() : MediaCommandBase(chip::app::Clusters::OnOff::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};
