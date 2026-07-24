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

Status MediaFileManagementManager::HandleAddFile(const CharSpan & name, uint64_t size, const CharSpan & mimeType,
                                                 const CharSpan & imageUri, Commands::AddFileResponse::Type & response)
{
    // Reject files that would exceed the advertised capacity.
    if (size > GetAvailableStorage())
    {
        response.status = FileStatusEnum::kInsufficientStorage;
        return Status::Success;
    }

    FileEntry entry;
    entry.fileID   = mNextFileID++;
    entry.name     = Sanitize(name);
    entry.size     = size;
    entry.mimeType = Sanitize(mimeType);
    entry.imageUri = Sanitize(imageUri);

    // Create an (empty) data blob for the file. Actual bytes would be delivered
    // out-of-band (BDX) in a full implementation.
    std::ofstream data(DataFilePath(entry.fileID), std::ios::trunc | std::ios::binary);
    if (!data.is_open())
    {
        ChipLogError(Zcl, "MediaFileManagementManager: failed to create data file for id %llu",
                     static_cast<unsigned long long>(entry.fileID));
        response.status = FileStatusEnum::kInsufficientStorage;
        return Status::Success;
    }
    data.close();

    const uint64_t assignedID = entry.fileID;
    mFiles.push_back(std::move(entry));
    SaveIndex();

    ChipLogProgress(Zcl, "MediaFileManagementManager: added file id %llu (%llu bytes)", static_cast<unsigned long long>(assignedID),
                    static_cast<unsigned long long>(size));

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
    const CharSpan & clientName, uint16_t requestID,
    const Optional<DataModel::Nullable<DataModel::DecodableList<CharSpan>>> & supportedMimeTypes)
{
    // Example app does not implement cross-device sharing; accept and report no
    // shared files. A full implementation would emit SharedFilesAdded once
    // matching files are located.
    ChipLogProgress(Zcl, "MediaFileManagementManager: RequestSharedFiles requestID=%u", requestID);
    return Status::Success;
}

Status MediaFileManagementManager::HandleGetSharedFile(uint16_t responseID, Commands::GetSharedFileResponse::Type & response)
{
    // No shared files are offered by the example app.
    ChipLogProgress(Zcl, "MediaFileManagementManager: GetSharedFile responseID=%u", responseID);
    response.status = FileStatusEnum::kFileNotAvailable;
    return Status::Success;
}

Status MediaFileManagementManager::HandleOfferFile(const CharSpan & clientName, const CharSpan & name, uint64_t size,
                                                   const CharSpan & mimeType, const CharSpan & imageUri)
{
    ChipLogProgress(Zcl, "MediaFileManagementManager: OfferFile from client, size %llu", static_cast<unsigned long long>(size));
    return Status::Success;
}

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
