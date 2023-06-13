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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/util/util.h>
#include <utility>
#include <vector>

using chip::Protocols::InteractionModel::Status;

template <typename T>
using List = chip::app::DataModel::List<T>;

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

class Delegate
{
public:
    explicit Delegate() {}

    virtual CHIP_ERROR Init() = 0;

    /**
     *
     * This function is to be overridden by a user implemented function that makes this decision based on the application logic.
     * @param mode
     */
    virtual Status HandleResetCondition();

    virtual ~Delegate() = default;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip