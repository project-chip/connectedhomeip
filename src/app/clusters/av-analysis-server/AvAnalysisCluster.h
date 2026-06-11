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

#include <app/clusters/av-analysis-server/AvAnalysisLogic.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <protocols/interaction_model/StatusCode.h>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

class AvAnalysisCluster;

/** @brief
 *  Defines interfaces for implementing application-specific logic for various aspects of the AvAnalysis
 * Cluster.
 */
class AvAnalysisDelegate
{
public:
    AvAnalysisDelegate()          = default;
    virtual ~AvAnalysisDelegate() = default;

    /**
     * Allows the delegate to perform any specific functions such as timer cancellation on a shutdown, this is invoked prior to
     * the destructor, it shall not be invoked as part of the destructor.
     */
    virtual void ShutdownApp() = 0;

    /**
     * Delegate command handlers
     */

    /**
     */
    virtual Protocols::InteractionModel::Status
    EnableContextTriggers() = 0;

    /**
     */
    virtual Protocols::InteractionModel::Status
    DisableContextTriggers() = 0;

    /**
     */
    virtual Protocols::InteractionModel::Status
    EstablishAnalysisStream() = 0;

    /**
     */
    virtual Protocols::InteractionModel::Status ActivateAnalysisStream() = 0;

    /**
     */
    virtual Protocols::InteractionModel::Status DeactivateAnalysisStream() = 0;

    /**
     */
    virtual Protocols::InteractionModel::Status RemoveAnalysisStream() = 0;
    
    /**
     * Delegate command helpers
     */
    virtual CHIP_ERROR VerifyZoneIDsAreValid(DataModel::DecodableList<uint16_t>) = 0;
    virtual bool CanAddContextTriggers() = 0;
    virtual void ActiveAmbientContextTriggersUpdated() = 0;

    

private:
    friend class AvAnalysisCluster;

    AvAnalysisCluster * mServer = nullptr;

    // Sets the Server pointer
    void SetServer(AvAnalysisCluster * aServer) { mServer = aServer; }

protected:
    AvAnalysisCluster * GetServer() const { return mServer; }
};

class AvAnalysisCluster : public DefaultServerCluster
{
public:
    /**
     * Creates a server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeatures         The bitflags value that identifies which features are supported by this instance.
     *
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    AvAnalysisCluster(EndpointId aEndpointId,
                      BitFlags<AvAnalysis::Feature> aFeatures, 
                      const std::vector<Descriptor::Structs::SemanticTagStruct::Type> & aSupportedAmbientContexts) :
        DefaultServerCluster({ aEndpointId, AvAnalysis::Id }),
        mLogic(aEndpointId, aFeatures, aSupportedAmbientContexts)
    {}

    AvAnalysisServerLogic & GetLogic() { return mLogic; }

    void MarkAttributeDirty(AttributeId attributeId) { NotifyAttributeChanged(attributeId); }

    void SetDelegate(AvAnalysisDelegate * delegate)
    {
        mLogic.SetDelegate(delegate);
        if (delegate != nullptr)
        {
            delegate->SetServer(this);
        }
    }

    CHIP_ERROR Init() { return mLogic.Init(); }

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    // Handle any dynamic cleanup required prior to the destructor being called on an app shutdown.  To be invoked by
    // an app as part of its own shutdown sequence and prior to the destruction of the app/delegate.
    void Shutdown(ClusterShutdownType shutdownType) override
    {
        DefaultServerCluster::Shutdown(shutdownType);
        mLogic.Shutdown();
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
                                                
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    // Attribute mutators
    CHIP_ERROR SetMaxAnalysisStreamCount(uint8_t aMaxAnalysisStreamCount);

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;


private:
    AvAnalysisServerLogic mLogic;
};

} // namespace Clusters
} // namespace app
} // namespace chip
