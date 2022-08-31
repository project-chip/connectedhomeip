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

#include <stdbool.h>
#include <stdint.h>

#include <functional>

// This is the interface to cluster implementations, providing access to manipulate attributes.
class ClusterInterface
{
public:
    virtual ~ClusterInterface() = default;

    // Set the endpoint for this cluster.
    virtual void SetEndpointId(chip::EndpointId id)                          = 0;
    // Get the ID for this cluster.
    virtual chip::ClusterId GetClusterId()                                   = 0;
    // Gets the list of enabled attributes for this cluster.
    virtual chip::Span<const EmberAfAttributeMetadata> GetAllAttributes()    = 0;
    // Write the given TLV encoded value to the given attribute. Can write
    // normally read-only values. Nofitied matter that the value has changed.
    virtual bool Push(chip::AttributeId attr, chip::TLV::TLVReader & reader) = 0;
};
