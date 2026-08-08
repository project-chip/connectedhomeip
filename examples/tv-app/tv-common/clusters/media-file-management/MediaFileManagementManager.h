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
#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>

#include <cstdint>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

/**
 * Hook implemented by the tv-app's BDX layer to move file bytes out-of-band.
 *
 * The Media File Management cluster commands only carry metadata; the actual
 * file and thumbnail bytes travel over BDX (protocol 0x0002) in a separate
 * exchange. The manager owns the on-disk metadata/index and calls into this
 * coordinator to drive the transfers. It is optional: when no coordinator is
 * set (e.g. in unit tests) the manager keeps metadata-only behavior.
 */
class BdxCoordinator
{
public:
    virtual ~BdxCoordinator() = default;

    /**
     * Download the bytes for a freshly added/offered file from `peer` into the
     * file's on-disk data path, using `fileName` as the BDX file designator.
     * `thumbnailUri` is the `bdx://` URI for the preview image (may be empty).
     * Invoked for AddFile and OfferFile, where the tv-app is the BDX receiver.
     */
    virtual CHIP_ERROR StartIncomingFileTransfer(ScopedNodeId peer, uint64_t fileID, CharSpan fileName, uint64_t size,
                                                 CharSpan thumbnailUri) = 0;

    /**
     * Arm the BDX sender to serve `fileID` (designator `fileName`) to `peer`,
     * and emit a SharedFilesAdded event correlating `requestID` with a freshly
     * generated ResponseID. Invoked for RequestSharedFiles.
     */
    virtual CHIP_ERROR ShareFileWithClient(ScopedNodeId peer, uint16_t requestID, uint64_t fileID, CharSpan fileName) = 0;

    /**
     * Resolve a ResponseID previously handed out by ShareFileWithClient to the
     * shared FileID, verifying it was shared with `peer`. Invoked for
     * GetSharedFile. Returns false if unknown or not shared with this peer.
     */
    virtual bool LookupSharedFile(ScopedNodeId peer, uint16_t responseID, uint64_t & fileID) = 0;

    /**
     * Build a `bdx://<own-node-id>/<designator>` URI (into `out`) that a client
     * uses to retrieve the given file's thumbnail from this device via BDX.
     */
    virtual CHIP_ERROR MakeSelfBdxUri(uint64_t fileID, CharSpan designator, MutableCharSpan & out) = 0;
};

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

    /// Attach the BDX coordinator used to move file bytes out-of-band. Optional.
    void SetBdxCoordinator(BdxCoordinator * coordinator) { mBdxCoordinator = coordinator; }

    /// Resolve the on-disk data-blob path for a file id (used by the BDX layer).
    std::string DataFilePathForFile(uint64_t fileID) const { return DataFilePath(fileID); }

    /**
     * Record the metadata for a fully received file (bytes already written to
     * its data path by the BDX layer) so it appears in AvailableFiles. Used by
     * the coordinator on completion of an AddFile/OfferFile download.
     */
    uint64_t AddReceivedFile(CharSpan name, uint64_t size, CharSpan mimeType, CharSpan imageUri);

    /// Reserve the next file id and its (empty) data blob; returns the id.
    uint64_t ReserveFile(CharSpan name, uint64_t size, CharSpan mimeType, CharSpan imageUri);

    /// Look up a file's metadata by id. Returns false if not present.
    bool GetFileById(uint64_t fileID, Structs::FileDescriptionStruct::Type & file) const;

    // --- Attribute data providers ---
    uint64_t GetTotalStorage() override;
    uint64_t GetAvailableStorage() override;
    CHIP_ERROR GetFileAtIndex(size_t index, Structs::FileDescriptionStruct::Type & file) override;
    CHIP_ERROR GetSupportedMimeTypeAtIndex(size_t index, MutableCharSpan & mimeType) override;

    // --- Command handlers ---
    Protocols::InteractionModel::Status HandleAddFile(ScopedNodeId peer, const CharSpan & name, uint64_t size,
                                                      const CharSpan & mimeType, const CharSpan & imageUri,
                                                      Commands::AddFileResponse::Type & response) override;
    Protocols::InteractionModel::Status HandleDeleteFile(uint64_t fileID) override;
    Protocols::InteractionModel::Status
    HandleRequestSharedFiles(ScopedNodeId peer, const CharSpan & clientName, uint16_t requestID,
                             const Optional<DataModel::Nullable<DataModel::DecodableList<CharSpan>>> & supportedMimeTypes) override;
    Protocols::InteractionModel::Status HandleGetSharedFile(ScopedNodeId peer, uint16_t responseID,
                                                            Commands::GetSharedFileResponse::Type & response) override;
    Protocols::InteractionModel::Status HandleOfferFile(ScopedNodeId peer, const CharSpan & clientName, const CharSpan & name,
                                                        uint64_t size, const CharSpan & mimeType,
                                                        const CharSpan & imageUri) override;

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

    // Append a new file entry (metadata) and create an empty data blob for it.
    // Returns the assigned file id, or 0 on failure.
    uint64_t AppendEntry(CharSpan name, uint64_t size, CharSpan mimeType, CharSpan imageUri);

    // Ensure the storage directory exists; returns false on failure.
    bool EnsureStorageDir();

    // Load/save the metadata index from/to disk.
    void LoadIndex();
    void SaveIndex();

    std::string mStorageDir;
    std::vector<FileEntry> mFiles;
    uint64_t mNextFileID = 1;

    // Backing storage for the bdx:// ImageUri returned by HandleGetSharedFile.
    // The response's CharSpan must stay valid until the cluster TLV-encodes it
    // after the handler returns, so it cannot live on the handler's stack.
    std::string mSharedFileUri;

    // Optional out-of-band BDX byte-transfer coordinator. Not owned.
    BdxCoordinator * mBdxCoordinator = nullptr;

    // Total capacity advertised for the (virtual) media store, in bytes.
    static constexpr uint64_t kTotalStorageBytes = 1024ull * 1024ull * 1024ull; // 1 GiB

    // Supported MIME types advertised via the SupportedMimeTypes attribute.
    static const std::vector<std::string> kSupportedMimeTypes;
};

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
