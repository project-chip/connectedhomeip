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

#include "Endpoint.h"
#include "Types.h"

#include <controller/CHIPCluster.h>

#include <iostream>
#include <map>
#include <memory>
#include <type_traits>

namespace matter {
namespace casting {
namespace core {

template <typename T>
class Attribute;
class Endpoint;

// Base cluster class
class BaseCluster
{
public:
    BaseCluster(memory::Weak<Endpoint> endpoint) { this->mEndpoint = endpoint; }

    virtual ~BaseCluster() {}

    BaseCluster()                       = delete;
    BaseCluster(BaseCluster & other)    = delete;
    void operator=(const BaseCluster &) = delete;

    memory::Weak<Endpoint> GetEndpoint() const { return mEndpoint.lock(); }

    /**
     * @brief Registers Commands and Attributes to this cluster
     */
    virtual void SetUp() {}

    /**
     * @return Pointer to the Attribute registered in this cluster, corresponding to attributeId
     */
    void * GetAttribute(const chip::AttributeId attributeId) { return mAttributes[attributeId]; }

    /**
     * @return Pointer to the Command registered in this cluster, corresponding to commandId
     */
    void * GetCommand(const chip::CommandId commandId) { return mCommands[commandId]; }

protected:
    /**
     * @brief Registers the attribute (expected to be of type Attribute *) against the attributeId in this cluster
     */
    void RegisterAttribute(const chip::AttributeId attributeId, void * attribute) { mAttributes[attributeId] = attribute; }

    /**
     * @brief Registers the command (expected to be of type Command *) against the commandId in this cluster
     */
    void RegisterCommand(const chip::CommandId commandId, void * command) { mCommands[commandId] = command; }

    memory::Weak<Endpoint> mEndpoint;

private:
    std::map<chip::CommandId, void *> mCommands;
    std::map<chip::AttributeId, void *> mAttributes;
};

/**
 * @brief MediaClusterBase is used to invoke controller/CHIPCluster.h#ReadAttribute() API calls
 */
class MediaClusterBase : public chip::Controller::ClusterBase
{
public:
    MediaClusterBase(chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
                     chip::EndpointId endpoint) :
        ClusterBase(exchangeManager, session, endpoint)
    {}
};

}; // namespace core
}; // namespace casting
}; // namespace matter
