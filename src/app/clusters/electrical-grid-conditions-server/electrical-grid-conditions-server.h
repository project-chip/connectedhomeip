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
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalGridConditions {

// Spec-defined constraints
constexpr uint8_t kMaxForecastEntries = 56;

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    // For now this is a placeholder and the Delegate could be removed
    // There are no delegated methods since these are largely implemented in the
    // electrical-grid-conditions-server.cpp Instance class

protected:
    EndpointId mEndpointId = 0;
};

class Instance : public AttributeAccessInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeatures) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), mDelegate(aDelegate), mFeatures(aFeatures)
    {
        /* set the base class delegate's endpointId */
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

    // Generate CurrentConditionsChanged events
    Protocols::InteractionModel::Status GenerateCurrentConditionsChangedEvent();

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeatures;

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
