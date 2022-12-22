/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#import "logging.h"

#include <cstdio>
#include <pthread.h>

namespace dft {
namespace logging {

    void Setup()
    {
        auto kLoggingColorError = @"\033[1;31m";
        auto kLoggingColorProgress = @"\033[0;32m";
        auto kLoggingColorDetail = @"\033[0;34m";
        auto kLoggingColorEnd = @"\033[0m";

        MTRSetLogCallback(MTRLogTypeDetail, ^(MTRLogType type, NSString * component, NSString * message) {
            NSString * loggingColor = nil;

            switch (type) {
            case MTRLogTypeError:
                loggingColor = kLoggingColorError;
                break;
            case MTRLogTypeProgress:
                loggingColor = kLoggingColorProgress;
                break;
            case MTRLogTypeDetail:
                loggingColor = kLoggingColorDetail;
                break;
            }

            auto formatter = [[NSDateFormatter alloc] init];
            formatter.dateFormat = @"yyyy-MM-dd HH:mm:ss.SSS";
            auto formattedDate = [formatter stringFromDate:[NSDate date]];

            int pid = [[NSProcessInfo processInfo] processIdentifier];

            auto tid = pthread_mach_thread_np(pthread_self());

            fprintf(stdout, "%s%s [%d:%u] [%s]: %s%s\n", loggingColor.UTF8String, formattedDate.UTF8String, pid, tid,
                component.UTF8String, message.UTF8String, kLoggingColorEnd.UTF8String);
        });
    }

}
}
