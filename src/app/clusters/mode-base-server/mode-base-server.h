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

#include <app/CommandHandlerInterface.h>
#include <app/AttributeAccessInterface.h>
#include "mode-base-delegate.h"
#include <app/util/af.h>
#include <map>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

class Instance : public CommandHandlerInterface, public AttributeAccessInterface
{

public:
    // This map holds pointers to all initialised ModeBase instances. It provides a way to access all ModeBase clusters.
    static std::map<uint32_t, Instance*> ModeBaseAliasesInstanceMap;

    CHIP_ERROR Init();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Generic accessor functions
    bool HasFeature(Feature feature) const;

    void UpdateStartUpMode(DataModel::Nullable<uint8_t> aNewStartUpMode);
    void UpdateOnMode(DataModel::Nullable<uint8_t> aNewOnMode);
    void UpdateCurrentMode(uint8_t aNewMode);

    DataModel::Nullable<uint8_t> GetStartUpMode();
    DataModel::Nullable<uint8_t> GetOnMode();
    uint8_t GetCurrentMode();

    EndpointId GetEndpointId() const {return mEndpointId;}

//    struct Type;

private:
    EndpointId mEndpointId{};
    ClusterId mClusterId{};
    Delegate * mDelegate;

    uint32_t mFeature;

    // Attribute data store
    uint8_t mCurrentMode;
    DataModel::Nullable<uint8_t> mStartUpMode;
    DataModel::Nullable<uint8_t> mOnMode;

    /**
     * This checks to see if this clusters instance is a valid ModeBase aliased cluster based on the AliasedClusters list.
     * @return true if the clusterId of this instance is a valid ModeBase cluster.
     */
    bool isAliasCluster() const;
    void HandleChangeToMode(HandlerContext & ctx, const Commands::ChangeToMode::DecodableType & req);

public:
    /**
     * Creates a mode base cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ModeBase aliased cluster to be instantiated.
     * @param aDelegate A pointer to a delegate that will handle application layer logic.
     */
    Instance(EndpointId aEndpointId, ClusterId aClusterId, Delegate * aDelegate, uint32_t aFeature) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        mDelegate(aDelegate)
    {
        mEndpointId = aEndpointId;
        mClusterId  = aClusterId;
        mFeature = aFeature;
    }

    ~Instance() override
    {
        ModeBaseAliasesInstanceMap.erase(mClusterId);
    }

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);
};

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip




