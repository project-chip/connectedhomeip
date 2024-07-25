/*
 *    Copyright (c) 2021-2024 Project CHIP Authors
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
#include <app/CommandHandlerInterface.h>
#include <app/util/af-types.h>

namespace chip {
namespace app {
namespace Compatibility {

// This reader should never actually be registered; we do manual dispatch to it
// for the one attribute it handles.
class MandatoryGlobalAttributeReader : public AttributeAccessInterface
{
public:
    MandatoryGlobalAttributeReader(const EmberAfCluster * aCluster) :
        AttributeAccessInterface(MakeOptional(kInvalidEndpointId), kInvalidClusterId), mCluster(aCluster)
    {}

protected:
    const EmberAfCluster * mCluster;
};

class GlobalAttributeReader : public MandatoryGlobalAttributeReader
{
public:
    GlobalAttributeReader(const EmberAfCluster * aCluster) : MandatoryGlobalAttributeReader(aCluster) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    typedef CHIP_ERROR (CommandHandlerInterface::*CommandListEnumerator)(const ConcreteClusterPath & cluster,
                                                                         CommandHandlerInterface::CommandIdCallback callback,
                                                                         void * context);
    static CHIP_ERROR EncodeCommandList(const ConcreteClusterPath & aClusterPath, AttributeValueEncoder & aEncoder,
                                        CommandListEnumerator aEnumerator, const CommandId * aClusterCommandList);
};

} // namespace Compatibility
} // namespace app
} // namespace chip
