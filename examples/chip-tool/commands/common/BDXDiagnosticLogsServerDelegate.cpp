/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "BDXDiagnosticLogsServerDelegate.h"

#include <string>
#include <unistd.h>

constexpr const char kTmpDir[]          = "/tmp/";
constexpr uint8_t kMaxFileDesignatorLen = 32;
constexpr uint16_t kMaxFilePathLen      = kMaxFileDesignatorLen + sizeof(kTmpDir) + 1;

// For testing a few file names trigger an error depending on the current 'phase'.
constexpr char kErrorOnTransferBegin[]            = "Error:OnTransferBegin";
constexpr char kErrorOnTransferData[]             = "Error:OnTransferData";
constexpr char kErrorOnTransferEnd[]              = "Error:OnTransferEnd";
constexpr char kErrorTransferMethodNotSupported[] = "TransferMethodNotSupported.txt";

BDXDiagnosticLogsServerDelegate BDXDiagnosticLogsServerDelegate::sInstance;

CHIP_ERROR CheckForErrorRequested(const chip::CharSpan & phaseErrorTarget, const chip::CharSpan & fileDesignator)
{
    VerifyOrReturnError(!phaseErrorTarget.data_equal(fileDesignator), CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckFileDesignatorAllowed(const std::map<chip::app::CommandSender *, std::string> & fileDesignators,
                                      const chip::CharSpan & fileDesignator)
{
    std::string targetFileDesignator(fileDesignator.data(), fileDesignator.size());

    for (auto it = fileDesignators.begin(); it != fileDesignators.end(); it++)
    {
        auto expectedFileDesignator = it->second;
        if (targetFileDesignator.compare(expectedFileDesignator) == 0)
        {
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR GetFilePath(const chip::CharSpan & fileDesignator, chip::MutableCharSpan & outFilePath)
{
    VerifyOrReturnError(fileDesignator.size() <= kMaxFileDesignatorLen, CHIP_ERROR_INVALID_STRING_LENGTH);
    // sizeof(kTmpDir) includes the trailing null.
    VerifyOrReturnError(outFilePath.size() >= sizeof(kTmpDir) - 1 + fileDesignator.size(), CHIP_ERROR_INTERNAL);

    memcpy(outFilePath.data(), kTmpDir, sizeof(kTmpDir) - 1);
    memcpy(outFilePath.data() + sizeof(kTmpDir) - 1, fileDesignator.data(), fileDesignator.size());
    outFilePath.reduce_size(sizeof(kTmpDir) - 1 + fileDesignator.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckFileExists(const char * filePath)
{
    if (access(filePath, F_OK) != 0)
    {
        ChipLogError(chipTool, "The file '%s' for dumping the logs does not exist.", filePath);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckFileDoesNotExist(const char * filePath)
{
    if (access(filePath, F_OK) == 0)
    {
        ChipLogError(chipTool, "The file '%s' for dumping the logs already exists.", filePath);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

void LogFileDesignator(const char * prefix, const chip::CharSpan & fileDesignator, CHIP_ERROR error = CHIP_NO_ERROR)
{
#if CHIP_PROGRESS_LOGGING
    auto size = static_cast<uint16_t>(fileDesignator.size());
    auto data = fileDesignator.data();
    ChipLogProgress(chipTool, "%s (%u): %.*s", prefix, size, size, data);
#endif // CHIP_PROGRESS_LOGGING

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(chipTool, "%s: %s", prefix, ErrorStr(error));
    }
}

CHIP_ERROR CreateFile(const char * filePath)
{
    VerifyOrReturnError(nullptr != filePath, CHIP_ERROR_INVALID_ARGUMENT);

    auto fd = fopen(filePath, "w+");
    VerifyOrReturnError(nullptr != fd, CHIP_ERROR_WRITE_FAILED);

    auto rv = fclose(fd);
    VerifyOrReturnError(EOF != rv, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppendToFile(const char * filePath, const chip::ByteSpan & data)
{
    VerifyOrReturnError(nullptr != filePath, CHIP_ERROR_INVALID_ARGUMENT);

    auto fd = fopen(filePath, "a");
    VerifyOrReturnError(nullptr != fd, CHIP_ERROR_WRITE_FAILED);

    fwrite(data.data(), data.size(), 1, fd);

    auto rv = fclose(fd);
    VerifyOrReturnError(EOF != rv, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BDXDiagnosticLogsServerDelegate::OnTransferBegin(chip::bdx::BDXTransferProxy * transfer)
{
    auto fileDesignator = transfer->GetFileDesignator();
    LogFileDesignator("OnTransferBegin", fileDesignator);

    VerifyOrReturnError(fileDesignator.size() != 0, CHIP_ERROR_UNKNOWN_RESOURCE_ID);

    chip::CharSpan phaseErrorTarget(kErrorOnTransferBegin, sizeof(kErrorOnTransferBegin) - 1);
    ReturnErrorOnFailure(CheckForErrorRequested(phaseErrorTarget, fileDesignator));

    chip::CharSpan transferErrorTarget(kErrorTransferMethodNotSupported, sizeof(kErrorTransferMethodNotSupported) - 1);
    VerifyOrReturnError(!transferErrorTarget.data_equal(fileDesignator), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ReturnErrorOnFailure(CheckFileDesignatorAllowed(mFileDesignators, fileDesignator));

    char outputFilePath[kMaxFilePathLen] = { 0 };
    chip::MutableCharSpan outputFilePathSpan(outputFilePath);
    ReturnErrorOnFailure(GetFilePath(fileDesignator, outputFilePathSpan));

    // Ensure null-termination of the filename, since we will be passing it around as a char *.
    VerifyOrReturnError(outputFilePathSpan.size() < MATTER_ARRAY_SIZE(outputFilePath), CHIP_ERROR_INTERNAL);
    outputFilePath[outputFilePathSpan.size()] = '\0';

    ReturnErrorOnFailure(CheckFileDoesNotExist(outputFilePath));
    ReturnErrorOnFailure(CreateFile(outputFilePath));

    return transfer->Accept();
}

CHIP_ERROR BDXDiagnosticLogsServerDelegate::OnTransferEnd(chip::bdx::BDXTransferProxy * transfer, CHIP_ERROR error)
{
    auto fileDesignator = transfer->GetFileDesignator();
    LogFileDesignator("OnTransferEnd", fileDesignator, error);

    chip::CharSpan phaseErrorTarget(kErrorOnTransferEnd, sizeof(kErrorOnTransferEnd) - 1);
    ReturnErrorOnFailure(CheckForErrorRequested(phaseErrorTarget, fileDesignator));

    char outputFilePath[kMaxFilePathLen] = { 0 };
    chip::MutableCharSpan outputFilePathSpan(outputFilePath);
    ReturnErrorOnFailure(GetFilePath(fileDesignator, outputFilePathSpan));
    ReturnErrorOnFailure(CheckFileExists(outputFilePath));

    return CHIP_NO_ERROR;
}

CHIP_ERROR BDXDiagnosticLogsServerDelegate::OnTransferData(chip::bdx::BDXTransferProxy * transfer, const chip::ByteSpan & data)
{
    auto fileDesignator = transfer->GetFileDesignator();
    LogFileDesignator("OnTransferData", fileDesignator);

    chip::CharSpan phaseErrorTarget(kErrorOnTransferData, sizeof(kErrorOnTransferData) - 1);
    ReturnErrorOnFailure(CheckForErrorRequested(phaseErrorTarget, fileDesignator));

    char outputFilePath[kMaxFilePathLen] = { 0 };
    chip::MutableCharSpan outputFilePathSpan(outputFilePath);
    ReturnErrorOnFailure(GetFilePath(fileDesignator, outputFilePathSpan));
    ReturnErrorOnFailure(CheckFileExists(outputFilePath));
    ReturnErrorOnFailure(AppendToFile(outputFilePath, data));

    return transfer->Continue();
}

void BDXDiagnosticLogsServerDelegate::AddFileDesignator(chip::app::CommandSender * sender, const chip::CharSpan & fileDesignator)
{
    std::string entry(fileDesignator.data(), fileDesignator.size());
    mFileDesignators[sender] = entry;
}

void BDXDiagnosticLogsServerDelegate::RemoveFileDesignator(chip::app::CommandSender * sender)
{
    mFileDesignators.erase(sender);
}
