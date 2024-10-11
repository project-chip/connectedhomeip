/*
 *   Copyright (c) 2024 Project CHIP Authors
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

#import <Matter/Matter.h>

#import "MTRError_Utils.h"

#include "DownloadLogCommand.h"
#include "RemoteDataModelLogger.h"

CHIP_ERROR DownloadLogCommand::RunCommand()
{
    ChipLogProgress(chipTool, "Downloading logs from node 0x" ChipLogFormatX64, ChipLogValueX64(mNodeId));

    MTRDeviceController * commissioner = CurrentCommissioner();
    auto * device = [MTRDevice deviceWithNodeID:@(mNodeId) controller:commissioner];

    auto logType = static_cast<MTRDiagnosticLogType>(mLogType);
    auto queue = dispatch_queue_create("com.chip.bdx.downloader", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);

    bool shouldWaitForDownload = !mIsAsyncCommand.ValueOr(false);
    mIsAsyncCommand.ClearValue();

    bool dumpToFile = mFilePath.HasValue();
    auto * dumpFilePath = dumpToFile ? [NSString stringWithUTF8String:mFilePath.Value()] : nil;
    mFilePath.ClearValue();

    auto * self = this;
    auto completion = ^(NSURL * url, NSError * error) {
        // A non-nil url indicates the presence of content, which can occur even in error scenarios like timeouts.
        NSString * logContent = nil;
        if (nil != url) {
            NSError * readError = nil;
            auto * data = [NSData dataWithContentsOfURL:url options:NSDataReadingUncached error:&readError];
            if (nil != readError) {
                if (shouldWaitForDownload) {
                    self->SetCommandExitStatus(MTRErrorToCHIPErrorCode(readError));
                }
                return;
            }

            logContent = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            NSLog(@"Content: %@", logContent);

            if (dumpToFile) {
                NSError * writeError = nil;
                auto * fileManager = [NSFileManager defaultManager];
                [fileManager copyItemAtPath:[url path] toPath:dumpFilePath error:&writeError];
                if (nil != writeError) {
                    if (shouldWaitForDownload) {
                        self->SetCommandExitStatus(MTRErrorToCHIPErrorCode(readError));
                    }
                    return;
                }
            }
        }

        ChipLogProgress(chipTool, "Diagnostic logs transfer: %s", error ? "Error" : "Success");
        auto err = RemoteDataModelLogger::LogBdxDownload(logContent, error);

        if (CHIP_NO_ERROR != err) {
            if (shouldWaitForDownload) {
                self->SetCommandExitStatus(err);
            }
            return;
        }

        if (shouldWaitForDownload) {
            self->SetCommandExitStatus(MTRErrorToCHIPErrorCode(error));
        }
    };

    [device downloadLogOfType:logType timeout:mTimeout queue:queue completion:completion];

    if (!shouldWaitForDownload) {
        SetCommandExitStatus(CHIP_NO_ERROR);
    }
    return CHIP_NO_ERROR;
}
