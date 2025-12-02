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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-delegate.h>
#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-logic.h>
#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-constants.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <protocols/interaction_model/StatusCode.h>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

class PhysicalPTZCallback
{
public:
    PhysicalPTZCallback()                                                               = default;
    virtual ~PhysicalPTZCallback()                                                      = default;
    virtual void OnPhysicalMovementComplete(Protocols::InteractionModel::Status status) = 0;
};

class CameraAvSettingsUserLevelMgmtServer : public DefaultServerCluster,
                                            public PhysicalPTZCallback
{
public:
    /**
     * Creates a server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeatures         The bitflags value that identifies which features are supported by this instance.

     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    CameraAvSettingsUserLevelMgmtServer(EndpointId aEndpointId, BitFlags<CameraAvSettingsUserLevelManagement::Feature> aFeatures) :
        DefaultServerCluster({ aEndpointId, CameraAvSettingsUserLevelManagement::Id }), mLogic(aEndpointId, aFeatures)
    {}

    CameraAvSettingsUserLevelMgmtServerLogic & GetLogic() { return mLogic; }

    void SetDelegate(CameraAvSettingsUserLevelmanagementDelegate * delegate)
    {
        mLogic.SetDelegate(delegate);
        if (delegate != nullptr) 
        {
            delegate->SetServer(this);
        }

    }

    CHIP_ERROR Init() { return mLogic.Init(); }

    // Handle any dynamic cleanup required prior to the destructor being called on an app shutdown.  To be invoked by
    // an app as part of its own shutdown sequence and prior to the destruction of the app/delegate.
    void Shutdown() override
    {
        DefaultServerCluster::Shutdown();
        mLogic.Shutdown();
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    class PhysicalPTZCallback
{
public:
    PhysicalPTZCallback()                                                               = default;
    virtual ~PhysicalPTZCallback()                                                      = default;
    virtual void OnPhysicalMovementComplete(Protocols::InteractionModel::Status status) = 0;
};

private:
    CameraAvSettingsUserLevelMgmtServerLogic mLogic;

    // Helper Read functions for complex attribute types
    CHIP_ERROR ReadAndEncodeMPTZPresets(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadAndEncodeDPTZStreams(AttributeValueEncoder & encoder); 
};

} // namespace Clusters
} // namespace app
} // namespace chip
