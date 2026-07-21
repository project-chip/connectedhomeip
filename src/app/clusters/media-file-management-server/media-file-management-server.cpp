/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "media-file-management-server.h"
#include "media-file-management-delegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandResponseHelper.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaFileManagement;
using Protocols::InteractionModel::Status;

static constexpr size_t kMediaFileManagementDelegateTableSize =
    MATTER_DM_MEDIA_FILE_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kMediaFileManagementDelegateTableSize <= kEmberInvalidEndpointIndex, "MediaFileManagement Delegate table size error");

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::MediaFileManagement::Delegate;

namespace {

Delegate * gDelegateTable[kMediaFileManagementDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, MediaFileManagement::Id,
                                                       MATTER_DM_MEDIA_FILE_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kMediaFileManagementDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Media File Management has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, MediaFileManagement::Id,
                                                       MATTER_DM_MEDIA_FILE_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kMediaFileManagementDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class MediaFileManagementAttrAccess : public app::AttributeAccessInterface
{
public:
    MediaFileManagementAttrAccess() :
        app::AttributeAccessInterface(Optional<EndpointId>::Missing(), chip::app::Clusters::MediaFileManagement::Id)
    {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;
};

MediaFileManagementAttrAccess gMediaFileManagementAttrAccess;

CHIP_ERROR MediaFileManagementAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case Attributes::TotalStorage::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.Encode(static_cast<uint64_t>(0));
        }
        return aEncoder.Encode(delegate->HandleGetTotalStorage());
    }
    case Attributes::AvailableStorage::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.Encode(static_cast<uint64_t>(0));
        }
        return aEncoder.Encode(delegate->HandleGetAvailableStorage());
    }
    case Attributes::AvailableFiles::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }
        return delegate->HandleGetAvailableFiles(aEncoder);
    }
    case Attributes::SupportedMimeTypes::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }
        return delegate->HandleGetSupportedMimeTypes(aEncoder);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfMediaFileManagementClusterAddFileCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::AddFile::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    app::CommandResponseHelper<Commands::AddFileResponse::Type> responder(commandObj, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    if (isDelegateNull(delegate, endpoint))
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    delegate->HandleAddFile(responder, commandData.name, commandData.size, commandData.mimeType, commandData.imageUri);

    if (!responder.HasSentResponse())
    {
        commandObj->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaFileManagementClusterDeleteFileCallback(app::CommandHandler * commandObj,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::DeleteFile::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    Delegate * delegate = GetDelegate(endpoint);
    if (isDelegateNull(delegate, endpoint))
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    delegate->HandleDeleteFile(commandObj, commandPath, commandData.fileID);

    return true;
}

bool emberAfMediaFileManagementClusterRequestSharedFilesCallback(app::CommandHandler * commandObj,
                                                                 const app::ConcreteCommandPath & commandPath,
                                                                 const Commands::RequestSharedFiles::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    Delegate * delegate = GetDelegate(endpoint);
    if (isDelegateNull(delegate, endpoint))
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    delegate->HandleRequestSharedFiles(commandObj, commandPath, commandData.clientName, commandData.requestID,
                                       commandData.supportedMimeTypes);

    return true;
}

bool emberAfMediaFileManagementClusterGetSharedFileCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::GetSharedFile::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    app::CommandResponseHelper<Commands::GetSharedFileResponse::Type> responder(commandObj, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    if (isDelegateNull(delegate, endpoint))
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    delegate->HandleGetSharedFile(responder, commandData.responseID);

    if (!responder.HasSentResponse())
    {
        commandObj->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaFileManagementClusterOfferFileCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::OfferFile::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    Delegate * delegate = GetDelegate(endpoint);
    if (isDelegateNull(delegate, endpoint))
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    delegate->HandleOfferFile(commandObj, commandPath, commandData.clientName, commandData.name, commandData.size,
                              commandData.mimeType, commandData.imageUri);

    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterMediaFileManagementPluginServerInitCallback()
{
    app::AttributeAccessInterfaceRegistry::Instance().Register(&gMediaFileManagementAttrAccess);
}
