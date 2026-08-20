/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <protocols/interaction_model/Constants.h>
#include <string>

namespace chip::app::Clusters {

class BridgedDeviceBasicInformationDelegate
{
public:
    virtual ~BridgedDeviceBasicInformationDelegate() = default;

    /// Called when the NodeLabel attribute is about to be changed.
    ///
    /// If a non-success status is returned, the attribute write will fail.
    /// Note: This is NOT called for no-op writes where the value has not changed.
    virtual Protocols::InteractionModel::Status OnNodeLabelChanged(const std::string & newNodeLabel)
    {
        return Protocols::InteractionModel::Status::Success;
    }
};

} // namespace chip::app::Clusters
