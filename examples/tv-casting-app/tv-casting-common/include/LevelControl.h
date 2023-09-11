/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MediaCommandBase.h"
#include "MediaSubscriptionBase.h"

#include <controller/CHIPCluster.h>
#include <functional>

#include <app-common/zap-generated/cluster-objects.h>

// COMMAND CLASSES
class StepCommand
    : public MediaCommandBase<chip::app::Clusters::LevelControl::Commands::Step::Type, chip::app::DataModel::NullObjectType>
{
public:
    StepCommand() : MediaCommandBase(chip::app::Clusters::LevelControl::Id) {}

    CHIP_ERROR Invoke(chip::app::Clusters::LevelControl::StepMode stepMode, uint8_t stepSize,
                      chip::app::DataModel::Nullable<uint16_t> transitionTime,
                      chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions> optionMask,
                      chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions> optionOverride,
                      std::function<void(CHIP_ERROR)> responseCallback);
};

class MoveToLevelCommand
    : public MediaCommandBase<chip::app::Clusters::LevelControl::Commands::MoveToLevel::Type, chip::app::DataModel::NullObjectType>
{
public:
    MoveToLevelCommand() : MediaCommandBase(chip::app::Clusters::LevelControl::Id) {}

    CHIP_ERROR Invoke(uint8_t level, chip::app::DataModel::Nullable<uint16_t> transitionTime,
                      chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions> optionMask,
                      chip::BitMask<chip::app::Clusters::LevelControl::LevelControlOptions> optionOverride,
                      std::function<void(CHIP_ERROR)> responseCallback);
};

// SUBSCRIBER CLASSES
class CurrentLevelSubscriber : public MediaSubscriptionBase<chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo>
{
public:
    CurrentLevelSubscriber() : MediaSubscriptionBase(chip::app::Clusters::LevelControl::Id) {}
};

class MinLevelSubscriber : public MediaSubscriptionBase<chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo>
{
public:
    MinLevelSubscriber() : MediaSubscriptionBase(chip::app::Clusters::LevelControl::Id) {}
};

class MaxLevelSubscriber : public MediaSubscriptionBase<chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo>
{
public:
    MaxLevelSubscriber() : MediaSubscriptionBase(chip::app::Clusters::LevelControl::Id) {}
};
