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

#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {

class AttributeDelegate
{
public:
    AttributeDelegate (ClusterId clusterId) : mClusterId(clusterId) {}
    /**
     * xxxx 
     */
    virtual void PostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size, uint8_t * value) {}
    /**
     * xxxx 
     */
    virtual Protocols::InteractionModel::Status ExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer) {return Protocols::InteractionModel::Status::Success;}
    /**
     * xxxx 
     */
    virtual Protocols::InteractionModel::Status ExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer, uint16_t maxReadLength) {return Protocols::InteractionModel::Status::Success;} 

    virtual ~AttributeDelegate() {}

private:
    ClusterId mClusterId;
};

void RegisterApplicationAttributeDelegate(ClusterId clusterId, AttributeDelegate * delegate);

} // namespace app
} // namespace chip



//class AllClustersCommandDelegate : public NamedPipeCommandDelegate
//{
//public:
//    void OnEventCommandReceived(const char * json) override;
//};
