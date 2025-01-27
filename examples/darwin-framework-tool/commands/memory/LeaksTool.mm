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

#include "LeaksTool.h"

#import <Foundation/Foundation.h>
#include <unistd.h> // For getpid()

constexpr const char * kDefaultOutputGraphPath = "darwin-framework-tool.memgraph";

@interface LeaksTool : NSObject
- (BOOL)runWithArguments:(NSArray<NSString *> * _Nullable)arguments;
@end

@implementation LeaksTool

- (BOOL)runWithArguments:(NSArray<NSString *> * _Nullable)arguments
{
#if TARGET_OS_OSX
    pid_t pid = getpid();
    __auto_type * pidString = [NSString stringWithFormat:@"%d", pid];

    __auto_type * task = [[NSTask alloc] init];
    task.launchPath = @"/usr/bin/leaks";
    task.arguments = @[ pidString ];
    if (arguments) {
        task.arguments = [task.arguments arrayByAddingObjectsFromArray:arguments];
    }

    __auto_type * pipe = [NSPipe pipe];
    task.standardOutput = pipe;
    task.standardError = pipe;

    __auto_type * fileHandle = [pipe fileHandleForReading];
    [task launch];
    [task waitUntilExit];

    int exitCode = [task terminationStatus];
    if (exitCode != EXIT_SUCCESS) {
        return NO;
    }

    __auto_type * data = [fileHandle readDataToEndOfFile];
    __auto_type * output = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    NSLog(@"%@", output);

    return YES;
#else
    NSLog(@"Running leaks as a task is supported on this platform.");
    return NO;
#endif // TARGET_OS_OSX
}

@end

CHIP_ERROR DumpMemoryGraph(chip::Optional<char *> filePath)
{
    NSMutableString * outputGraphArgument = [NSMutableString stringWithFormat:@"--outputGraph=%s", kDefaultOutputGraphPath];
    if (filePath.HasValue()) {
        outputGraphArgument = [NSMutableString stringWithFormat:@"--outputGraph=%s", filePath.Value()];
    }

    __auto_type * leaksTool = [[LeaksTool alloc] init];
    if (![leaksTool runWithArguments:@[ outputGraphArgument ]]) {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}
