/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/support/Span.h>

#include <stdbool.h>
#include <stdint.h>

#include <functional>

#include "GeneratedClusters.h"
#include "data-model/Attribute.h"

// This is the interface to cluster implementations, providing access to manipulate attributes.
class ClusterInterface
{
public:
    virtual ~ClusterInterface() = default;

    // Set the endpoint for this cluster.
    virtual void SetEndpointId(chip::EndpointId id) = 0;
    // Get the ID for this cluster.
    virtual chip::ClusterId GetClusterId() = 0;
    // Gets the list of enabled attributes for this cluster.
    virtual chip::Span<AttributeInterface *> GetAllAttributes() = 0;
    // Find an attribute of this cluster, if present.
    virtual AttributeInterface * FindAttribute(chip::AttributeId id) = 0;
    // Find an attribute of this cluster, if present, given its name.
    virtual AttributeInterface * FindAttributeByName(chip::CharSpan name) = 0;
    // Write the given TLV encoded value to the given attribute. Can write
    // normally read-only values. Nofitied matter that the value has changed.
    virtual bool Write(chip::AttributeId attr, chip::TLV::TLVReader & reader) = 0;
};

struct CommonCluster;
typedef std::function<CHIP_ERROR(CommonCluster *, const chip::app::ConcreteDataAttributePath &, chip::app::AttributeValueDecoder &)>
    PropagateWriteCB;

// This provides the backend access to implementation details.
struct CommonCluster : public ClusterInterface
{
    CommonCluster(std::unique_ptr<GeneratedCluster> cluster);
    ~CommonCluster();

    void SetEndpointId(chip::EndpointId id) override;
    chip::EndpointId GetEndpointId() const;

    chip::ClusterId GetClusterId() override;

    void SetCallback(PropagateWriteCB * cb);

    bool active() const;

    // Implemented by generated code. Valid writes should be forwarded to ForwardWriteToBridge.
    virtual CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath,
                                       chip::app::AttributeValueDecoder & aDecoder);

    // Calls the registered callback if one exists, otherwise triggers a write of the actual
    // stored value.
    CHIP_ERROR ForwardWriteToBridge(const chip::app::ConcreteDataAttributePath & aPath,
                                    chip::app::AttributeValueDecoder & aDecoder);

    chip::Span<AttributeInterface *> GetAllAttributes() override;
    AttributeInterface * FindAttribute(chip::AttributeId id) override;
    AttributeInterface * FindAttributeByName(chip::CharSpan name) override;

    // Force a write of a given attribute. Can write normally read-only values.
    // Calls OnUpdated after the write finishes.
    bool Write(chip::AttributeId attr, chip::TLV::TLVReader & reader) override;

    // Push an attribute update notification to the matter stack.
    void OnUpdated(chip::AttributeId attr);

    template <typename T>
    void OnUpdated(const T & attr)
    {
        OnUpdated(attr.GetId());
    }

protected:
    chip::EndpointId mEndpoint   = 0xFFFF;
    PropagateWriteCB * mCallback = nullptr;
    std::vector<AttributeInterface *> mAttributes;
    std::unique_ptr<GeneratedCluster> mStorage;
};
