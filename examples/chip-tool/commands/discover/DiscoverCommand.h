/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#include "../../config/PersistentStorage.h"
#include "../common/Command.h"
#include <controller/ExampleOperationalCredentialsIssuer.h>

class DiscoverCommand : public Command, public chip::Controller::DeviceAddressUpdateDelegate
{
public:
    DiscoverCommand(const char * commandName) : Command(commandName)
    {
        AddArgument("nodeid", 0, UINT64_MAX, &mNodeId);
        AddArgument("fabricid", 0, UINT64_MAX, &mFabricId);
    }

    /////////// DeviceAddressUpdateDelegate Interface /////////
    void OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR error) override{};

    /////////// Command Interface /////////
    CHIP_ERROR Run(NodeId localId, NodeId remoteId) override;

    virtual CHIP_ERROR RunCommand(NodeId remoteId, uint64_t fabricId) = 0;

private:
    chip::NodeId mNodeId;
    uint64_t mFabricId;
};
