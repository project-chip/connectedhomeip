/*
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

#include <pw_unit_test/framework.h>

#include "../MediaFileManagementManager.h"

#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>

#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaFileManagement;
using Protocols::InteractionModel::Status;

// A unique per-test storage directory under /tmp so tests never collide with a
// running app's real store and are independent of one another.
std::string MakeTempDir(const char * suffix)
{
    std::string dir = std::string("/tmp/chip-mfm-test-") + suffix + "-" + std::to_string(getpid());
    // Best-effort clean slate.
    std::string index = dir + "/index.txt";
    std::remove(index.c_str());
    rmdir(dir.c_str());
    return dir;
}

Commands::AddFileResponse::Type AddFile(MediaFileManagementManager & mgr, const char * name, uint64_t size, const char * mime,
                                        const char * uri)
{
    Commands::AddFileResponse::Type response;
    Status status = mgr.HandleAddFile(CharSpan::fromCharString(name), size, CharSpan::fromCharString(mime),
                                      CharSpan::fromCharString(uri), response);
    EXPECT_EQ(status, Status::Success);
    return response;
}

struct TestMediaFileManagementManager : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestMediaFileManagementManager, StartsEmpty)
{
    MediaFileManagementManager mgr(MakeTempDir("empty"));

    Structs::FileDescriptionStruct::Type file;
    EXPECT_EQ(mgr.GetFileAtIndex(0, file), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    EXPECT_EQ(mgr.GetAvailableStorage(), mgr.GetTotalStorage());
}

TEST_F(TestMediaFileManagementManager, AddFileAssignsIncrementingIdsAndAppearsInList)
{
    MediaFileManagementManager mgr(MakeTempDir("add"));

    auto r1 = AddFile(mgr, "Song.mp3", 3500, "audio/mpeg", "bdx://n/1.jpg");
    ASSERT_EQ(r1.status, FileStatusEnum::kSuccess);
    ASSERT_FALSE(r1.fileID.IsNull());

    auto r2 = AddFile(mgr, "Movie.mp4", 9000, "video/mp4", "bdx://n/2.jpg");
    ASSERT_EQ(r2.status, FileStatusEnum::kSuccess);
    ASSERT_FALSE(r2.fileID.IsNull());

    // IDs are unique and monotonically increasing.
    EXPECT_NE(r1.fileID.Value(), r2.fileID.Value());
    EXPECT_LT(r1.fileID.Value(), r2.fileID.Value());

    // Both files are enumerable.
    Structs::FileDescriptionStruct::Type file0;
    Structs::FileDescriptionStruct::Type file1;
    ASSERT_EQ(mgr.GetFileAtIndex(0, file0), CHIP_NO_ERROR);
    ASSERT_EQ(mgr.GetFileAtIndex(1, file1), CHIP_NO_ERROR);
    EXPECT_EQ(mgr.GetFileAtIndex(2, file0), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);

    EXPECT_EQ(file0.fileID, r1.fileID.Value());
    EXPECT_TRUE(file0.name.data_equal(CharSpan::fromCharString("Song.mp3")));
    EXPECT_EQ(file0.size, 3500u);
    EXPECT_TRUE(file0.mimeType.data_equal(CharSpan::fromCharString("audio/mpeg")));
}

TEST_F(TestMediaFileManagementManager, AvailableStorageTracksAddedFiles)
{
    MediaFileManagementManager mgr(MakeTempDir("storage"));

    const uint64_t total = mgr.GetTotalStorage();
    AddFile(mgr, "a", 1000, "video/mp4", "u");
    EXPECT_EQ(mgr.GetAvailableStorage(), total - 1000);
    AddFile(mgr, "b", 2500, "video/mp4", "u");
    EXPECT_EQ(mgr.GetAvailableStorage(), total - 3500);
}

TEST_F(TestMediaFileManagementManager, AddFileRejectedWhenExceedingCapacity)
{
    MediaFileManagementManager mgr(MakeTempDir("capacity"));

    Commands::AddFileResponse::Type response;
    Status status = mgr.HandleAddFile(CharSpan::fromCharString("huge"), mgr.GetTotalStorage() + 1,
                                      CharSpan::fromCharString("video/mp4"), CharSpan::fromCharString("u"), response);
    EXPECT_EQ(status, Status::Success);
    EXPECT_EQ(response.status, FileStatusEnum::kInsufficientStorage);
    EXPECT_TRUE(response.fileID.IsNull());
}

TEST_F(TestMediaFileManagementManager, DeleteFileRemovesEntry)
{
    MediaFileManagementManager mgr(MakeTempDir("delete"));

    auto r1 = AddFile(mgr, "a", 1000, "video/mp4", "u");
    AddFile(mgr, "b", 2000, "video/mp4", "u");

    EXPECT_EQ(mgr.HandleDeleteFile(r1.fileID.Value()), Status::Success);

    // Only one file remains.
    Structs::FileDescriptionStruct::Type file;
    ASSERT_EQ(mgr.GetFileAtIndex(0, file), CHIP_NO_ERROR);
    EXPECT_EQ(mgr.GetFileAtIndex(1, file), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    EXPECT_TRUE(file.name.data_equal(CharSpan::fromCharString("b")));
}

TEST_F(TestMediaFileManagementManager, DeleteUnknownFileReturnsNotFound)
{
    MediaFileManagementManager mgr(MakeTempDir("delete-unknown"));
    EXPECT_EQ(mgr.HandleDeleteFile(9999), Status::NotFound);
}

TEST_F(TestMediaFileManagementManager, StatePersistsAcrossInstances)
{
    const std::string dir = MakeTempDir("persist");

    uint64_t savedID = 0;
    {
        MediaFileManagementManager mgr(dir);
        auto r  = AddFile(mgr, "persisted.mp4", 4096, "video/mp4", "bdx://n/p.jpg");
        savedID = r.fileID.Value();
    }

    // A fresh manager over the same directory reloads the persisted file.
    MediaFileManagementManager reopened(dir);
    Structs::FileDescriptionStruct::Type file;
    ASSERT_EQ(reopened.GetFileAtIndex(0, file), CHIP_NO_ERROR);
    EXPECT_EQ(file.fileID, savedID);
    EXPECT_TRUE(file.name.data_equal(CharSpan::fromCharString("persisted.mp4")));
    EXPECT_EQ(file.size, 4096u);

    // A newly added file must not reuse the persisted ID.
    auto r2 = AddFile(reopened, "next.mp4", 100, "video/mp4", "u");
    EXPECT_GT(r2.fileID.Value(), savedID);
}

TEST_F(TestMediaFileManagementManager, SupportedMimeTypesEnumerable)
{
    MediaFileManagementManager mgr(MakeTempDir("mime"));

    char buffer[64];
    size_t count = 0;
    while (true)
    {
        MutableCharSpan span(buffer);
        CHIP_ERROR err = mgr.GetSupportedMimeTypeAtIndex(count, span);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        ASSERT_EQ(err, CHIP_NO_ERROR);
        EXPECT_GT(span.size(), 0u);
        count++;
    }
    EXPECT_GT(count, 0u);
}

TEST_F(TestMediaFileManagementManager, NameWithTabIsSanitizedAndSurvivesReload)
{
    const std::string dir = MakeTempDir("sanitize");
    {
        MediaFileManagementManager mgr(dir);
        // A tab in the name would corrupt the tab-delimited index if not sanitized.
        AddFile(mgr, "bad\tname", 10, "video/mp4", "u");
    }
    MediaFileManagementManager reopened(dir);
    Structs::FileDescriptionStruct::Type file;
    ASSERT_EQ(reopened.GetFileAtIndex(0, file), CHIP_NO_ERROR);
    // The tab was replaced with a space, and the record round-tripped intact.
    EXPECT_TRUE(file.name.data_equal(CharSpan::fromCharString("bad name")));
    EXPECT_EQ(file.size, 10u);
}

} // namespace
