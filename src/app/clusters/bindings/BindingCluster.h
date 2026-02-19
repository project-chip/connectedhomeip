/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/Binding/Attributes.h>
#include <clusters/Binding/Structs.h>

namespace chip {
namespace app {
namespace Clusters {

using TargetStructType         = Binding::Structs::TargetStruct::Type;
using DecodableBindingListType = Binding::Attributes::Binding::TypeInfo::DecodableType;

class BindingCluster : public DefaultServerCluster
{
public:
    /// Injected dependencies for this cluster
    struct Context
    {
        Binding::Table & bindingTable;
        Binding::Manager & bindingManager;
        DeviceLayer::PlatformManager & platformManager;
    };

    constexpr BindingCluster(Context && context, EndpointId endpointId) :
        DefaultServerCluster(ConcreteClusterPath::ConstExpr(endpointId, Binding::Id)), mClusterContext(std::move(context))
    {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    void ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType,
                                        FabricIndex accessingFabric) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

private:
    bool IsValidBinding(const EndpointId localEndpoint, const TargetStructType & entry);

    CHIP_ERROR CheckValidBindingList(const EndpointId localEndpoint, const DecodableBindingListType & bindingList,
                                     FabricIndex accessingFabricIndex);

    CHIP_ERROR NotifyBindingsChanged(FabricIndex accessingFabricIndex);

    /**
     * @brief appends a binding to the list of bindings
     *        This function is to be used when a device wants to add a binding to its own table
     *        If entry is a unicast binding, BindingManager will be notified and will establish a case session with the peer device
     *        Entry will be added to the binding table and persisted into storage
     *        BindingManager will be notified and the binding added callback will be called if it has been set
     *
     * @param entry binding to add
     */
    CHIP_ERROR CreateBindingEntry(const TargetStructType & entry, EndpointId localEndpoint);

    Context mClusterContext;
};

} // namespace Clusters
} // namespace app
} // namespace chip
