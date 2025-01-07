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

#include "LeakChecker.h"

#import <Foundation/Foundation.h>
#include <unistd.h> // For getpid()

@interface LeakChecker : NSObject
- (BOOL)hasMemoryLeaks;
@end

@implementation LeakChecker

- (BOOL)hasMemoryLeaks
{
#if TARGET_OS_OSX
    pid_t pid = getpid();
    auto * pidString = [NSString stringWithFormat:@"%d", pid];

    auto * task = [[NSTask alloc] init];
    task.launchPath = @"/usr/bin/leaks";
    task.arguments = @[ pidString ];

    auto * pipe = [NSPipe pipe];
    task.standardOutput = pipe;
    task.standardError = pipe;

    NSFileHandle * fileHandle = [pipe fileHandleForReading];
    [task launch];
    [task waitUntilExit];

    int exitCode = [task terminationStatus];
    if (exitCode) {
        NSData * data = [fileHandle readDataToEndOfFile];
        NSString * output = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        NSLog(@"%@", output);
        return YES;
    }
#else
    NSLog(@"Running leaks as a task is supported on this platform.");
#endif // TARGET_OS_OSX

    return NO;
}

@end

int ConditionalLeaksCheck(int exitCode)
{
#ifdef DFT_ENABLE_LEAK_CHECKING
    @autoreleasepool {
        auto * leakChecker = [[LeakChecker alloc] init];
        if ([leakChecker hasMemoryLeaks]) {
            return EXIT_FAILURE;
        }
    }
#endif // DFT_ENABLE_LEAK_CHECKING

    return exitCode;
}
