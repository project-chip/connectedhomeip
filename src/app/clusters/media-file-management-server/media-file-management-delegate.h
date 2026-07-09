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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandResponseHelper.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

using FileDescriptionStruct     = Structs::FileDescriptionStruct::Type;
using AddFileResponseType       = Commands::AddFileResponse::Type;
using GetSharedFileResponseType = Commands::GetSharedFileResponse::Type;

class Delegate
{
public:
    virtual void HandleAddFile(CommandResponseHelper<AddFileResponseType> & helper, const CharSpan & name, uint64_t size,
                               const CharSpan & mimeType, const CharSpan & imageUri) = 0;

    virtual void HandleDeleteFile(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, uint64_t fileID) = 0;

    virtual void
    HandleRequestSharedFiles(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, const CharSpan & clientName,
                             uint16_t requestID,
                             const Optional<DataModel::Nullable<DataModel::DecodableList<CharSpan>>> & supportedMimeTypes) = 0;

    virtual void HandleGetSharedFile(CommandResponseHelper<GetSharedFileResponseType> & helper, uint16_t responseID) = 0;

    virtual void HandleOfferFile(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, const CharSpan & clientName,
                                 const CharSpan & name, uint64_t size, const CharSpan & mimeType, const CharSpan & imageUri) = 0;

    virtual CHIP_ERROR HandleGetAvailableFiles(app::AttributeValueEncoder & aEncoder)     = 0;
    virtual CHIP_ERROR HandleGetSupportedMimeTypes(app::AttributeValueEncoder & aEncoder) = 0;
    virtual uint64_t HandleGetTotalStorage()                                              = 0;
    virtual uint64_t HandleGetAvailableStorage()                                          = 0;

    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint)      = 0;
    virtual uint16_t GetClusterRevision(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
