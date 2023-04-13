/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/util/af-enums.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/basic-types.h>
#include <protocols/interaction_model/StatusCode.h>
#include <app/CommandHandlerInterface.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

/**
 * Interface to help manage the supported modes of the Mode Select Cluster.
 */
// class SupportedModesManager
// {

//     using ModeOptionStructType = Structs::ModeOptionStruct::Type;

// public:
//     /**
//      * A class that can return the supported ModeOptions for a specific endpoint.
//      */
//     struct ModeOptionsProvider
//     {
//         using pointer = const ModeOptionStructType *;

//         /**
//          * Returns the ModeOptionStructIterator to the first option.
//          */
//         inline pointer begin() const { return mBegin; }

//         /**
//          * Returns the ModeOptionStructIterator to an element after the last option.
//          */
//         inline pointer end() const { return mEnd; }

//         ModeOptionsProvider(const pointer aBegin, const pointer aEnd) : mBegin(aBegin), mEnd(aEnd) {}

//         pointer mBegin;
//         pointer mEnd;
//     };

//     /**
//      * Given the endpointId, returns all its supported modes options.
//      * @param endpointId
//      * @return The mode options provider for the endpoint.
//      */
//     virtual ModeOptionsProvider getModeOptionsProvider(EndpointId endpointId) const = 0;

//     /**
//      * Given the endpointId and a mode value, find the ModeOptionStruct that matches the mode.
//      * @param endpointId The endpoint for which to search the ModeOptionStruct.
//      * @param mode The mode for which to search the ModeOptionStruct.
//      * @param dataPtr The pointer to receive the ModeOptionStruct, if one is found.
//      * @return Status::Success if successfully found the option. Otherwise, returns appropriate status code (found in
//      * <app/util/af-enums.h>)
//      */
//     virtual Protocols::InteractionModel::Status getModeOptionByMode(EndpointId endpointId, uint8_t mode,
//                                                                     const ModeOptionStructType ** dataPtr) const = 0;

//     virtual ~SupportedModesManager() {}
// };

// const SupportedModesManager * getSupportedModesManager();

// --- New way

class Delegate {
public:
    virtual CHIP_ERROR Init() = 0;
    // virtual getSupportedModes();
    // Protocols::InteractionModel::Status getModeOptionByMode(EndpointId, uint8_t, const **ModeSelect::Structs::ModeOptionStruct::Type);
    virtual ~Delegate() = default;
};

class Instance : public CommandHandlerInterface,
                 public AttributeAccessInterface
{
public:
    CHIP_ERROR Init();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    // CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;
    // CHIP_ERROR EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    // // AttributeAccessInterface
    // CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    // CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    Delegate * msDelegate;

public:
    Instance(EndpointId aEndpointId, ClusterId aClusterId, Delegate * aDelegate) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId)
    {
        msDelegate = aDelegate;
        this->Init();
    }

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
};


} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
