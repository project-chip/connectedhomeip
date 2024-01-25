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

CHIP_ERROR DownloadLogCommand::RunCommand()
{
    ChipLogProgress(chipTool, "Downloading logs from node 0x" ChipLogFormatX64, ChipLogValueX64(mNodeId));

    MTRDeviceController * commissioner = CurrentCommissioner();
    auto * device = [MTRDevice deviceWithNodeID:@(mNodeId) controller:commissioner];

    auto logType = static_cast<MTRDiagnosticLogType>(mLogType);
    auto queue = dispatch_queue_create("com.chip.bdx.downloader", DISPATCH_QUEUE_SERIAL);

    auto * self = this;
    auto completion = ^(NSURL * url, NSError * error) {
        // A non-nil url indicates the presence of content, which can occur even in error scenarios like timeouts.
        if (nil != url) {
            NSError * readError = nil;
            auto * data = [NSData dataWithContentsOfURL:url options:NSDataReadingUncached error:&readError];
            VerifyOrReturn(nil == readError, self->SetCommandExitStatus(MTRErrorToCHIPErrorCode(readError)));

            auto * content = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            NSLog(@"Content: %@", content);
        }

        VerifyOrReturn(nil == error, self->SetCommandExitStatus(MTRErrorToCHIPErrorCode(error)));

        // The url is nil when there are no logs on the target device.
        if (nil == url) {
            NSLog(@"No logs has been found onto node 0x" ChipLogFormatX64, ChipLogValueX64(mNodeId));
        }
        self->SetCommandExitStatus(CHIP_NO_ERROR);
    };

    [device downloadLogOfType:logType timeout:mTimeout queue:queue completion:completion];
    return CHIP_NO_ERROR;
}
