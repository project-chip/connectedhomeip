/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import <Matter/Matter.h>

#import "MTRTestCase.h"
#import "MTRTestServerAppRunner.h"

static unsigned sAppRunnerIndex = 1;

// kBasePort gets the discriminator added to it to figure out the port the app
// should be using.  This ensures that apps with distinct discriminators use
// distinct ports.
static const uint16_t kMinDiscriminator = 1111;
static const uint16_t kBasePort = 5542 - kMinDiscriminator;

@implementation MTRTestServerAppRunner {
    unsigned _uniqueIndex;
#if HAVE_NSTASK
    NSTask * _appTask;
#endif
}

- (instancetype)initWithAppName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload testcase:(MTRTestCase *)testcase
{
#if !HAVE_NSTASK
    XCTFail("Unable to start server app when we do not have NSTask");
    return nil;
#else // HAVE_NSTASK
    if (!(self = [super init])) {
        return nil;
    }

    _uniqueIndex = sAppRunnerIndex++;

    NSError * error;
    __auto_type * parsedPayload = [MTRSetupPayload setupPayloadWithOnboardingPayload:payload error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(parsedPayload);

    XCTAssertFalse(parsedPayload.hasShortDiscriminator);

    NSNumber * discriminator = parsedPayload.discriminator;
    XCTAssertGreaterThanOrEqual(discriminator.unsignedShortValue, kMinDiscriminator);

    NSNumber * passcode = parsedPayload.setupPasscode;

    __auto_type * executable = [NSString stringWithFormat:@"out/debug/%@-app/chip-%@-app", name, name];
    _appTask = [testcase createTaskForPath:executable];

    __auto_type * forcedArguments = @[
        // Make sure we only advertise on the local interface.
        @"--interface-id",
        @"-1",
        @"--secured-device-port",
        [NSString stringWithFormat:@"%u", kBasePort + discriminator.unsignedShortValue],
        @"--discriminator",
        [NSString stringWithFormat:@"%u", discriminator.unsignedShortValue],
        @"--passcode",
        [NSString stringWithFormat:@"%llu", passcode.unsignedLongLongValue],
        @"--KVS",
        [NSString stringWithFormat:@"/tmp/chip-%@-kvs%u", name, _uniqueIndex],
    ];

    __auto_type * allArguments = [forcedArguments arrayByAddingObjectsFromArray:arguments];
    [_appTask setArguments:allArguments];

    NSString * outFile = [NSString stringWithFormat:@"/tmp/darwin/framework-tests/%@-app-%u.log", name, _uniqueIndex];
    NSString * errorFile = [NSString stringWithFormat:@"/tmp/darwin/framework-tests/%@-app-err-%u.log", name, _uniqueIndex];

    // Make sure the files exist.
    [[NSFileManager defaultManager] createFileAtPath:outFile contents:nil attributes:nil];
    [[NSFileManager defaultManager] createFileAtPath:errorFile contents:nil attributes:nil];

    _appTask.standardOutput = [NSFileHandle fileHandleForWritingAtPath:outFile];
    _appTask.standardError = [NSFileHandle fileHandleForWritingAtPath:errorFile];

    [testcase launchTask:_appTask];

    NSLog(@"Started chip-%@-app with arguments %@ stdout=%@ and stderr=%@", name, allArguments, outFile, errorFile);

    return self;
#endif // HAVE_NSTASK
}

+ (unsigned)nextUniqueIndex
{
    return sAppRunnerIndex;
}

@end
