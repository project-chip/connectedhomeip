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

#include <clusters/MediaFileManagement/Commands.h>
#include <clusters/MediaFileManagement/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

/**
 * Application-specific logic for the Media File Management cluster.
 *
 * The delegate provides file storage information and handles the cluster
 * commands. The cluster core (MediaFileManagementCluster) owns feature-map,
 * cluster-revision and all attribute-list metadata; the delegate only supplies
 * the underlying application data and command behavior.
 */
class Delegate
{
public:
    virtual ~Delegate() = default;

    // --- Attribute data providers ---

    /// Total storage capacity for media files, in bytes.
    virtual uint64_t GetTotalStorage() = 0;

    /// Currently available storage capacity for media files, in bytes.
    virtual uint64_t GetAvailableStorage() = 0;

    /**
     * Return the file at the Nth index of the AvailableFiles list.
     * @return CHIP_NO_ERROR when the file was populated,
     *         CHIP_ERROR_PROVIDER_LIST_EXHAUSTED when index is past the end.
     */
    virtual CHIP_ERROR GetFileAtIndex(size_t index, Structs::FileDescriptionStruct::Type & file) = 0;

    /**
     * Return the MIME type string at the Nth index of the SupportedMimeTypes list.
     * @return CHIP_NO_ERROR when the entry was populated,
     *         CHIP_ERROR_PROVIDER_LIST_EXHAUSTED when index is past the end.
     */
    virtual CHIP_ERROR GetSupportedMimeTypeAtIndex(size_t index, MutableCharSpan & mimeType) = 0;

    // --- Command handlers ---

    /**
     * Handle the AddFile command. Populate `response` with the resulting status
     * and (on success) the newly assigned FileID.
     */
    virtual Protocols::InteractionModel::Status HandleAddFile(const CharSpan & name, uint64_t size, const CharSpan & mimeType,
                                                              const CharSpan & imageUri,
                                                              Commands::AddFileResponse::Type & response) = 0;

    /// Handle the DeleteFile command.
    virtual Protocols::InteractionModel::Status HandleDeleteFile(uint64_t fileID) = 0;

    /// Handle the RequestSharedFiles command (MediaSharing feature).
    virtual Protocols::InteractionModel::Status
    HandleRequestSharedFiles(const CharSpan & clientName, uint16_t requestID,
                             const Optional<DataModel::Nullable<DataModel::DecodableList<CharSpan>>> & supportedMimeTypes) = 0;

    /**
     * Handle the GetSharedFile command (MediaSharing feature). Populate
     * `response` with the resulting status and (on success) the file description.
     */
    virtual Protocols::InteractionModel::Status HandleGetSharedFile(uint16_t responseID,
                                                                    Commands::GetSharedFileResponse::Type & response) = 0;

    /// Handle the OfferFile command (MediaSharing feature).
    virtual Protocols::InteractionModel::Status HandleOfferFile(const CharSpan & clientName, const CharSpan & name, uint64_t size,
                                                                const CharSpan & mimeType, const CharSpan & imageUri) = 0;
};

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
