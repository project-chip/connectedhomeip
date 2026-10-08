/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/media-file-management-server/MediaFileManagementDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/MediaFileManagement/Attributes.h>
#include <clusters/MediaFileManagement/Enums.h>

namespace chip {
namespace app {
namespace Clusters {

class MediaFileManagementCluster : public DefaultServerCluster
{
public:
    MediaFileManagementCluster(EndpointId endpointId, MediaFileManagement::Delegate & delegate,
                               BitFlags<MediaFileManagement::Feature> features) :
        DefaultServerCluster({ endpointId, MediaFileManagement::Id }),
        mDelegate(delegate), mFeatures(features)
    {}

    // --- ServerClusterInterface overrides ---
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    /**
     * Public helper for the application to emit the SharedFilesAdded event.
     */
    void GenerateSharedFilesAddedEvent(uint16_t requestID, uint16_t responseID);

private:
    MediaFileManagement::Delegate & mDelegate;
    const BitFlags<MediaFileManagement::Feature> mFeatures;

    bool HasFeature(MediaFileManagement::Feature feature) const { return mFeatures.Has(feature); }

    CHIP_ERROR ReadAvailableFiles(const AttributeValueEncoder::ListEncodeHelper & encoder);
    CHIP_ERROR ReadSupportedMimeTypes(const AttributeValueEncoder::ListEncodeHelper & encoder);
};

} // namespace Clusters
} // namespace app
} // namespace chip
