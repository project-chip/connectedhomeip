/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/server/JointFabricDatastore.h>
#include <app/server/Server.h>
#include <lib/core/CHIPError.h>

namespace chip {

class JFADatastoreSync : public app::JointFabricDatastore::Delegate
{
public:
    JFADatastoreSync() {}

    CHIP_ERROR Init(Server & server);

    /* app::JointFabricDatastore::Delegate */
    CHIP_ERROR
    SyncNode(NodeId nodeId,
             const app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type & endpointGroupIDEntry,
             std::function<void()> onSuccess) override;
    CHIP_ERROR
    SyncNode(NodeId nodeId,
             const app::Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & nodeKeySetEntry,
             std::function<void()> onSuccess) override;
    CHIP_ERROR
    SyncNode(NodeId nodeId,
             const app::Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & bindingEntry,
             std::function<void()> onSuccess) override;
    CHIP_ERROR SyncNode(NodeId nodeId, const app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type & aclEntry,
                        std::function<void()> onSuccess) override;

private:
    friend JFADatastoreSync & JFADSync(void);

    static JFADatastoreSync sJFDS;

    Server * mServer = nullptr;
};

inline JFADatastoreSync & JFADSync(void)
{
    return JFADatastoreSync::sJFDS;
}

} // namespace chip
