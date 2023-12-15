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

#include "Cluster.h"
#include "Types.h"

#include "lib/support/logging/CHIPLogging.h"

namespace matter {
namespace casting {
namespace core {

enum ReadAttributeError
{
    READ_ATTRIBUTE_NO_ERROR
};

enum WriteAttributeError
{
    WRITE_ATTRIBUTE_NO_ERROR
};

template <typename ValueType>
using ReadAttributeCallback = std::function<void(Optional<ValueType> before, ValueType after, ReadAttributeError)>;

using WriteAttributeCallback = std::function<void(WriteAttributeError)>;

class BaseCluster;

template <typename ValueType>
class Attribute
{
private:
    memory::Weak<BaseCluster> cluster;
    ValueType value;

public:
    Attribute(memory::Weak<BaseCluster> cluster) { this->cluster = cluster; }

    ~Attribute() {}

    Attribute()                       = delete;
    Attribute(Attribute & other)      = delete;
    void operator=(const Attribute &) = delete;

protected:
    memory::Strong<BaseCluster> GetCluster() const { return cluster.lock(); }

public:
    ValueType GetValue();
    void Read(ReadAttributeCallback<ValueType> onRead);
    void Write(ValueType value, WriteAttributeCallback onWrite);
    bool SubscribeAttribute(AttributeId attributeId, ReadAttributeCallback<ValueType> callback);
    bool UnsubscribeAttribute(AttributeId attributeId, ReadAttributeCallback<ValueType> callback);
};

}; // namespace core
}; // namespace casting
}; // namespace matter
