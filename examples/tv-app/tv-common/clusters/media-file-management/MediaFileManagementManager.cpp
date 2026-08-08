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

#include "MediaFileManagementManager.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

using Protocols::InteractionModel::Status;

const std::vector<std::string> MediaFileManagementManager::kSupportedMimeTypes = {
    "video/mp4",
    "audio/mpeg",
    "image/jpeg",
};

namespace {

// Upper bound for a bdx:// thumbnail URI: "bdx://" (6) + 16 hex node-id chars +
// '/' + a file-designator (bounded by the 128-char Name field) + NUL.
constexpr size_t kMaxBdxUriLength = 160;

// The index file stores one record per line, tab-separated, in the form:
//   <fileID>\t<size>\t<mimeType>\t<imageUri>\t<name>
// `name` is last because it is the only field permitted to contain spaces (but
// not tabs or newlines, which we sanitize on write).
constexpr char kFieldSep = '\t';

std::string Sanitize(const CharSpan & span)
{
    // An empty CharSpan may carry a null data pointer; constructing a
    // std::string from (nullptr, 0) is undefined behavior.
    if (span.empty())
    {
        return "";
    }
    std::string out(span.data(), span.size());
    for (char & c : out)
    {
        if (c == '\t' || c == '\n' || c == '\r')
        {
            c = ' ';
        }
    }
    return out;
}

} // namespace

MediaFileManagementManager::MediaFileManagementManager(std::string storageDir) : mStorageDir(std::move(storageDir))
{
    if (!EnsureStorageDir())
    {
        ChipLogError(Zcl, "MediaFileManagementManager: failed to create storage dir %s", mStorageDir.c_str());
        return;
    }
    LoadIndex();
}

std::string MediaFileManagementManager::DataFilePath(uint64_t fileID) const
{
    return mStorageDir + "/" + std::to_string(fileID) + ".bin";
}

std::string MediaFileManagementManager::IndexFilePath() const
{
    return mStorageDir + "/index.txt";
}

bool MediaFileManagementManager::EnsureStorageDir()
{
    struct stat st;
    if (stat(mStorageDir.c_str(), &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }
    // 0755: owner rwx, group/other rx.
    return mkdir(mStorageDir.c_str(), 0755) == 0;
}

void MediaFileManagementManager::LoadIndex()
{
    mFiles.clear();
    mNextFileID = 1;

    std::ifstream in(IndexFilePath());
    if (!in.is_open())
    {
        return; // No index yet; start empty.
    }

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty())
        {
            continue;
        }

        // Parse the first four tab-delimited fields; the remainder is the name.
        FileEntry entry;
        size_t p0 = line.find(kFieldSep);
        size_t p1 = (p0 == std::string::npos) ? std::string::npos : line.find(kFieldSep, p0 + 1);
        size_t p2 = (p1 == std::string::npos) ? std::string::npos : line.find(kFieldSep, p1 + 1);
        size_t p3 = (p2 == std::string::npos) ? std::string::npos : line.find(kFieldSep, p2 + 1);
        if (p3 == std::string::npos)
        {
            ChipLogError(Zcl, "MediaFileManagementManager: skipping malformed index line");
            continue;
        }

        entry.fileID   = strtoull(line.substr(0, p0).c_str(), nullptr, 10);
        entry.size     = strtoull(line.substr(p0 + 1, p1 - p0 - 1).c_str(), nullptr, 10);
        entry.mimeType = line.substr(p1 + 1, p2 - p1 - 1);
        entry.imageUri = line.substr(p2 + 1, p3 - p2 - 1);
        entry.name     = line.substr(p3 + 1);

        if (entry.fileID >= mNextFileID)
        {
            mNextFileID = entry.fileID + 1;
        }
        mFiles.push_back(std::move(entry));
    }
    ChipLogProgress(Zcl, "MediaFileManagementManager: loaded %u file(s) from %s", static_cast<unsigned>(mFiles.size()),
                    IndexFilePath().c_str());
}

void MediaFileManagementManager::SaveIndex()
{
    std::ofstream out(IndexFilePath(), std::ios::trunc);
    if (!out.is_open())
    {
        ChipLogError(Zcl, "MediaFileManagementManager: failed to write index %s", IndexFilePath().c_str());
        return;
    }
    for (const FileEntry & entry : mFiles)
    {
        out << entry.fileID << kFieldSep << entry.size << kFieldSep << entry.mimeType << kFieldSep << entry.imageUri << kFieldSep
            << entry.name << '\n';
    }
}

uint64_t MediaFileManagementManager::GetTotalStorage()
{
    return kTotalStorageBytes;
}

uint64_t MediaFileManagementManager::GetAvailableStorage()
{
    uint64_t used = 0;
    for (const FileEntry & entry : mFiles)
    {
        used += entry.size;
    }
    return (used >= kTotalStorageBytes) ? 0 : (kTotalStorageBytes - used);
}

CHIP_ERROR MediaFileManagementManager::GetFileAtIndex(size_t index, Structs::FileDescriptionStruct::Type & file)
{
    VerifyOrReturnError(index < mFiles.size(), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

    const FileEntry & entry = mFiles[index];
    file.fileID             = entry.fileID;
    file.name               = CharSpan(entry.name.data(), entry.name.size());
    file.size               = entry.size;
    file.mimeType           = CharSpan(entry.mimeType.data(), entry.mimeType.size());
    file.imageUri           = CharSpan(entry.imageUri.data(), entry.imageUri.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR MediaFileManagementManager::GetSupportedMimeTypeAtIndex(size_t index, MutableCharSpan & mimeType)
{
    VerifyOrReturnError(index < kSupportedMimeTypes.size(), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(CharSpan(kSupportedMimeTypes[index].data(), kSupportedMimeTypes[index].size()), mimeType);
}

uint64_t MediaFileManagementManager::AppendEntry(CharSpan name, uint64_t size, CharSpan mimeType, CharSpan imageUri)
{
    FileEntry entry;
    entry.fileID   = mNextFileID++;
    entry.name     = Sanitize(name);
    entry.size     = size;
    entry.mimeType = Sanitize(mimeType);
    entry.imageUri = Sanitize(imageUri);

    // Create an (empty) data blob for the file. The bytes themselves are
    // delivered out-of-band over BDX by the coordinator (if any).
    std::ofstream data(DataFilePath(entry.fileID), std::ios::trunc | std::ios::binary);
    if (!data.is_open())
    {
        ChipLogError(Zcl, "MediaFileManagementManager: failed to create data file for id %llu",
                     static_cast<unsigned long long>(entry.fileID));
        // Roll back the id allocation so it can be reused.
        mNextFileID--;
        return 0;
    }
    data.close();

    const uint64_t assignedID = entry.fileID;
    mFiles.push_back(std::move(entry));
    SaveIndex();
    return assignedID;
}

uint64_t MediaFileManagementManager::ReserveFile(CharSpan name, uint64_t size, CharSpan mimeType, CharSpan imageUri)
{
    return AppendEntry(name, size, mimeType, imageUri);
}

uint64_t MediaFileManagementManager::AddReceivedFile(CharSpan name, uint64_t size, CharSpan mimeType, CharSpan imageUri)
{
    // Bytes have already been written to the data path by the BDX layer; this
    // just records the metadata so the file appears in AvailableFiles.
    return AppendEntry(name, size, mimeType, imageUri);
}

bool MediaFileManagementManager::GetFileById(uint64_t fileID, Structs::FileDescriptionStruct::Type & file) const
{
    for (const FileEntry & entry : mFiles)
    {
        if (entry.fileID == fileID)
        {
            file.fileID   = entry.fileID;
            file.name     = CharSpan(entry.name.data(), entry.name.size());
            file.size     = entry.size;
            file.mimeType = CharSpan(entry.mimeType.data(), entry.mimeType.size());
            file.imageUri = CharSpan(entry.imageUri.data(), entry.imageUri.size());
            return true;
        }
    }
    return false;
}

Status MediaFileManagementManager::HandleAddFile(ScopedNodeId peer, const CharSpan & name, uint64_t size, const CharSpan & mimeType,
                                                 const CharSpan & imageUri, Commands::AddFileResponse::Type & response)
{
    // Reject files that would exceed the advertised capacity.
    if (size > GetAvailableStorage())
    {
        response.status = FileStatusEnum::kInsufficientStorage;
        return Status::Success;
    }

    const uint64_t assignedID = AppendEntry(name, size, mimeType, imageUri);
    if (assignedID == 0)
    {
        response.status = FileStatusEnum::kInsufficientStorage;
        return Status::Success;
    }

    ChipLogProgress(Zcl, "MediaFileManagementManager: added file id %llu (%llu bytes)", static_cast<unsigned long long>(assignedID),
                    static_cast<unsigned long long>(size));

    // Kick off the out-of-band download of the file (and thumbnail) from the
    // client. The metadata entry already exists; bytes land asynchronously.
    if (mBdxCoordinator != nullptr)
    {
        CHIP_ERROR err = mBdxCoordinator->StartIncomingFileTransfer(peer, assignedID, name, size, imageUri);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "MediaFileManagementManager: AddFile BDX start failed: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    response.status = FileStatusEnum::kSuccess;
    response.fileID.SetNonNull(assignedID);
    return Status::Success;
}

Status MediaFileManagementManager::HandleDeleteFile(uint64_t fileID)
{
    for (auto it = mFiles.begin(); it != mFiles.end(); ++it)
    {
        if (it->fileID == fileID)
        {
            std::remove(DataFilePath(fileID).c_str());
            mFiles.erase(it);
            SaveIndex();
            ChipLogProgress(Zcl, "MediaFileManagementManager: deleted file id %llu", static_cast<unsigned long long>(fileID));
            return Status::Success;
        }
    }
    ChipLogProgress(Zcl, "MediaFileManagementManager: DeleteFile - id %llu not found", static_cast<unsigned long long>(fileID));
    return Status::NotFound;
}

Status MediaFileManagementManager::HandleRequestSharedFiles(
    ScopedNodeId peer, const CharSpan & clientName, uint16_t requestID,
    const Optional<DataModel::Nullable<DataModel::DecodableList<CharSpan>>> & supportedMimeTypes)
{
    ChipLogProgress(Zcl, "MediaFileManagementManager: RequestSharedFiles requestID=%u", requestID);

    // Without a BDX coordinator this example cannot actually serve bytes, so it
    // accepts the request but shares nothing (no SharedFilesAdded events).
    VerifyOrReturnValue(mBdxCoordinator != nullptr, Status::Success);

    // Share every locally stored file with the requester. Each ShareFileWithClient
    // arms the BDX sender and emits a SharedFilesAdded event for that file.
    for (const FileEntry & entry : mFiles)
    {
        CHIP_ERROR err =
            mBdxCoordinator->ShareFileWithClient(peer, requestID, entry.fileID, CharSpan(entry.name.data(), entry.name.size()));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "MediaFileManagementManager: ShareFileWithClient failed for id %llu: %" CHIP_ERROR_FORMAT,
                         static_cast<unsigned long long>(entry.fileID), err.Format());
        }
    }
    return Status::Success;
}

Status MediaFileManagementManager::HandleGetSharedFile(ScopedNodeId peer, uint16_t responseID,
                                                       Commands::GetSharedFileResponse::Type & response)
{
    ChipLogProgress(Zcl, "MediaFileManagementManager: GetSharedFile responseID=%u", responseID);

    uint64_t fileID = 0;
    if (mBdxCoordinator == nullptr || !mBdxCoordinator->LookupSharedFile(peer, responseID, fileID))
    {
        response.status = FileStatusEnum::kFileNotAvailable;
        return Status::Success;
    }

    Structs::FileDescriptionStruct::Type file;
    if (!GetFileById(fileID, file))
    {
        response.status = FileStatusEnum::kFileNotAvailable;
        return Status::Success;
    }

    // Rewrite the ImageUri to a bdx:// URI pointing at this device so the client
    // can fetch the file/thumbnail from us. The response's CharSpan is encoded by
    // the cluster after this handler returns, so the URI must be backed by
    // storage that outlives this call (a member string), not a stack buffer.
    // Fits "bdx://" + 16-hex node id + '/' + a file-designator name.
    char uriBuffer[kMaxBdxUriLength] = {};
    MutableCharSpan uri(uriBuffer);
    if (mBdxCoordinator->MakeSelfBdxUri(fileID, file.name, uri) == CHIP_NO_ERROR)
    {
        mSharedFileUri.assign(uri.data(), uri.size());
        file.imageUri = CharSpan(mSharedFileUri.data(), mSharedFileUri.size());
    }

    response.status = FileStatusEnum::kSuccess;
    response.fileDescription.SetValue(DataModel::MakeNullable(file));
    return Status::Success;
}

Status MediaFileManagementManager::HandleOfferFile(ScopedNodeId peer, const CharSpan & clientName, const CharSpan & name,
                                                   uint64_t size, const CharSpan & mimeType, const CharSpan & imageUri)
{
    ChipLogProgress(Zcl, "MediaFileManagementManager: OfferFile from client, size %llu", static_cast<unsigned long long>(size));

    // Reject files that would exceed the advertised capacity.
    VerifyOrReturnValue(size <= GetAvailableStorage(), Status::ResourceExhausted);

    // Without a coordinator we cannot fetch the bytes; accept the metadata only.
    VerifyOrReturnValue(mBdxCoordinator != nullptr, Status::Success);

    const uint64_t assignedID = AppendEntry(name, size, mimeType, imageUri);
    VerifyOrReturnValue(assignedID != 0, Status::Failure);

    CHIP_ERROR err = mBdxCoordinator->StartIncomingFileTransfer(peer, assignedID, name, size, imageUri);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MediaFileManagementManager: OfferFile BDX start failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return Status::Success;
}

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
