/*
*
*    Copyright (c) 2023 Project CHIP Authors
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


#include <app/clusters/mode-select-server/mode-select-delegate.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

/// This is an application level delegate to handle ModeSelect commands according to the specific business logic.
class ModeSelectDelegate : public Delegate
{
private:
    CHIP_ERROR Init() override;
    Status HandleChangeToMode(uint8_t mode) override;
    void HandleChangeToModeWitheStatus(uint8_t mode, ModeSelect::Commands::ChangeToModeResponse::Type &response) override;

public:
    explicit ModeSelectDelegate(std::vector<chip::app::Clusters::ModeSelect::Structs::ModeOptionStruct::Type> modes) :
        Delegate(std::move(modes)) {}

    ~ModeSelectDelegate() override = default;
};

/// This is an application level delegate to handle RvcRun commands according to the specific business logic.
class RvcRunDelegate : public Delegate
{
private:
    CHIP_ERROR Init() override;
    Status HandleChangeToMode(uint8_t mode) override;
    void HandleChangeToModeWitheStatus(uint8_t mode, ModeSelect::Commands::ChangeToModeResponse::Type &response) override;

public:
    explicit RvcRunDelegate(std::vector<chip::app::Clusters::ModeSelect::Structs::ModeOptionStruct::Type> modes) :
        Delegate(std::move(modes)) {}

    ~RvcRunDelegate() override = default;
};

/// This is an application level delegate to handle RvcClean commands according to the specific business logic.
class RvcCleanDelegate : public Delegate
{
private:
    CHIP_ERROR Init() override;
    Status HandleChangeToMode(uint8_t mode) override;
    void HandleChangeToModeWitheStatus(uint8_t mode, ModeSelect::Commands::ChangeToModeResponse::Type &response) override;

public:
    explicit RvcCleanDelegate(std::vector<chip::app::Clusters::ModeSelect::Structs::ModeOptionStruct::Type> modes) :
        Delegate(std::move(modes)) {}

    ~RvcCleanDelegate() override = default;
};

/// This is an application level delegate to handle DishwasherControl commands according to the specific business logic.
class DishwasherControlDelegate : public Delegate
{
private:
    CHIP_ERROR Init() override;
    Status HandleChangeToMode(uint8_t mode) override;
    void HandleChangeToModeWitheStatus(uint8_t mode, ModeSelect::Commands::ChangeToModeResponse::Type &response) override;

public:
    explicit DishwasherControlDelegate(std::vector<chip::app::Clusters::ModeSelect::Structs::ModeOptionStruct::Type> modes) :
        Delegate(std::move(modes)) {}

    ~DishwasherControlDelegate() override = default;
};

/// This is an application level delegate to handle LaundryWasher commands according to the specific business logic.
class LaundryWasherDelegate : public Delegate
{
private:
    CHIP_ERROR Init() override;
    Status HandleChangeToMode(uint8_t mode) override;
    void HandleChangeToModeWitheStatus(uint8_t mode, ModeSelect::Commands::ChangeToModeResponse::Type &response) override;

public:
    explicit LaundryWasherDelegate(std::vector<chip::app::Clusters::ModeSelect::Structs::ModeOptionStruct::Type> modes) :
        Delegate(std::move(modes)) {}

    ~LaundryWasherDelegate() override = default;
};

/// This is an application level delegate to handle Refrigerator And Temperature Controlled Cabinet (TCC) commands
/// according to the specific business logic.
class TccDelegate : public Delegate
{
private:
    CHIP_ERROR Init() override;
    Status HandleChangeToMode(uint8_t mode) override;
    void HandleChangeToModeWitheStatus(uint8_t mode, ModeSelect::Commands::ChangeToModeResponse::Type &response) override;

public:
    explicit TccDelegate(std::vector<chip::app::Clusters::ModeSelect::Structs::ModeOptionStruct::Type> modes) :
        Delegate(std::move(modes)) {}

    ~TccDelegate() override = default;
};

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
