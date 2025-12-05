/*
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

#include <app/server-cluster/ServerClusterExtension.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

CHIP_ERROR ServerClusterExtension::Startup(ServerClusterContext & context)
{
    mContext      = &context;
    mVersionDelta = 0;
    return mUnderlying.Startup(context);
}

void ServerClusterExtension::Shutdown()
{
    mUnderlying.Shutdown();
    mContext = nullptr;
}

void ServerClusterExtension::NotifyAttributeChanged(AttributeId id)
{
    VerifyOrReturn(mContext != nullptr);
    mVersionDelta++;
    mContext->interactionContext.dataModelChangeListener.MarkDirty({ mClusterPath.mEndpointId, mClusterPath.mClusterId, id });
}

Span<const ConcreteClusterPath> ServerClusterExtension::GetPaths() const
{
    return mUnderlying.GetPaths();
}

DataVersion ServerClusterExtension::GetDataVersion(const ConcreteClusterPath & path) const
{
    DataVersion underlying_version = mUnderlying.GetDataVersion(path);
    return underlying_version + (path == mClusterPath ? mVersionDelta : 0);
}

BitFlags<DataModel::ClusterQualityFlags> ServerClusterExtension::GetClusterFlags(const ConcreteClusterPath & path) const
{
    return mUnderlying.GetClusterFlags(path);
}

DataModel::ActionReturnStatus ServerClusterExtension::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                     AttributeValueDecoder & decoder)
{
    return mUnderlying.WriteAttribute(request, decoder);
}

void ServerClusterExtension::ListAttributeWriteNotification(const ConcreteAttributePath & path,
                                                            DataModel::ListWriteOperation opType, FabricIndex accessingFabric)
{
    mUnderlying.ListAttributeWriteNotification(path, opType, accessingFabric);
}

DataModel::ActionReturnStatus ServerClusterExtension::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                    AttributeValueEncoder & encoder)
{
    return mUnderlying.ReadAttribute(request, encoder);
}

CHIP_ERROR ServerClusterExtension::Attributes(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    return mUnderlying.Attributes(path, builder);
}

CHIP_ERROR ServerClusterExtension::EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo)
{
    return mUnderlying.EventInfo(path, eventInfo);
}

std::optional<DataModel::ActionReturnStatus> ServerClusterExtension::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                   chip::TLV::TLVReader & input_arguments,
                                                                                   CommandHandler * handler)
{
    return mUnderlying.InvokeCommand(request, input_arguments, handler);
}

CHIP_ERROR ServerClusterExtension::AcceptedCommands(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return mUnderlying.AcceptedCommands(path, builder);
}

CHIP_ERROR ServerClusterExtension::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    return mUnderlying.GeneratedCommands(path, builder);
}

} // namespace app
} // namespace chip
