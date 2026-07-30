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

#include <app/clusters/media-file-management-server/MediaFileManagementDelegate.h>

#include <cstdint>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

/**
 * Example file-backed Media File Management delegate for the Linux tv-app.
 *
 * Files and their metadata are persisted under a storage directory on disk
 * (default: /tmp/chip-media-files), similar to how the example KVS store keeps
 * its artifacts under /tmp. Each managed file is represented by:
 *   - a data blob `<storageDir>/<fileID>.bin` (created empty on AddFile; the
 *     actual bytes would arrive out-of-band via BDX in a full implementation),
 *   - a line in `<storageDir>/index.txt` holding the metadata record.
 *
 * The index is loaded into memory at construction and rewritten on every
 * mutation so state survives restarts.
 */
class MediaFileManagementManager : public Delegate
{
public:
    static constexpr const char * kDefaultStorageDir = "/tmp/chip-media-files";

    explicit MediaFileManagementManager(std::string storageDir = kDefaultStorageDir);
    ~MediaFileManagementManager() override = default;

    // --- Attribute data providers ---
    uint64_t GetTotalStorage() override;
    uint64_t GetAvailableStorage() override;
    CHIP_ERROR GetFileAtIndex(size_t index, Structs::FileDescriptionStruct::Type & file) override;
    CHIP_ERROR GetSupportedMimeTypeAtIndex(size_t index, MutableCharSpan & mimeType) override;

    // --- Command handlers ---
    Protocols::InteractionModel::Status HandleAddFile(const CharSpan & name, uint64_t size, const CharSpan & mimeType,
                                                      const CharSpan & imageUri,
                                                      Commands::AddFileResponse::Type & response) override;
    Protocols::InteractionModel::Status HandleDeleteFile(uint64_t fileID) override;
    Protocols::InteractionModel::Status
    HandleRequestSharedFiles(const CharSpan & clientName, uint16_t requestID,
                             const Optional<DataModel::Nullable<DataModel::DecodableList<CharSpan>>> & supportedMimeTypes) override;
    Protocols::InteractionModel::Status HandleGetSharedFile(uint16_t responseID,
                                                            Commands::GetSharedFileResponse::Type & response) override;
    Protocols::InteractionModel::Status HandleOfferFile(const CharSpan & clientName, const CharSpan & name, uint64_t size,
                                                        const CharSpan & mimeType, const CharSpan & imageUri) override;

private:
    // In-memory representation of a single managed file's metadata. Char span
    // members returned to the cluster point at these owned strings.
    struct FileEntry
    {
        uint64_t fileID = 0;
        std::string name;
        uint64_t size = 0;
        std::string mimeType;
        std::string imageUri;
    };

    std::string DataFilePath(uint64_t fileID) const;
    std::string IndexFilePath() const;

    // Ensure the storage directory exists; returns false on failure.
    bool EnsureStorageDir();

    // Load/save the metadata index from/to disk.
    void LoadIndex();
    void SaveIndex();

    std::string mStorageDir;
    std::vector<FileEntry> mFiles;
    uint64_t mNextFileID = 1;

    // Total capacity advertised for the (virtual) media store, in bytes.
    static constexpr uint64_t kTotalStorageBytes = 1024ull * 1024ull * 1024ull; // 1 GiB

    // Supported MIME types advertised via the SupportedMimeTypes attribute.
    static const std::vector<std::string> kSupportedMimeTypes;
};

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
