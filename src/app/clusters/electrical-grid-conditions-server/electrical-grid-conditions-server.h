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
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalGridConditions {

// Spec-defined constraints
constexpr uint8_t kMaxForecastEntries = 56; // TODO not mentioned in the spec!

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    // For now this is a place holder and the Delegate could be removed
    // There is no delegated methods since these are largely implemented in the
    // commodity-price-server.cpp Instance class

protected:
    EndpointId mEndpointId = 0;
};

class Instance : public AttributeAccessInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), mDelegate(aDelegate), mFeature(aFeature)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
        mEndpointId = aEndpointId;
    }

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    // Set attribute methods
    CHIP_ERROR SetLocalGenerationAvailable(DataModel::Nullable<bool>);
    CHIP_ERROR SetCurrentConditions(DataModel::Nullable<Structs::ElectricalGridConditionsStruct::Type>);
    CHIP_ERROR SetForecastConditions(const DataModel::List<const Structs::ElectricalGridConditionsStruct::Type> &);

    // Send CurrentConditionsChanged events
    Protocols::InteractionModel::Status GenerateCurrentConditionsChangedEvent();

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;

    EndpointId mEndpointId;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    // NOTE there are no writable attributes

    // CommandHandlerInterface
    // NOTE there are no commands

    // Attribute storage
    DataModel::Nullable<bool> mLocalGenerationAvailable;
    DataModel::Nullable<Structs::ElectricalGridConditionsStruct::Type> mCurrentConditions;
    DataModel::List<const Structs::ElectricalGridConditionsStruct::Type> mForecastConditions;
};

} // namespace ElectricalGridConditions
} // namespace Clusters
} // namespace app
} // namespace chip
