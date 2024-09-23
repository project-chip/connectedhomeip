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

#import "MTRTestCase+ServerAppRunner.h"

static unsigned sAppRunnerIndex = 1;

#if HAVE_NSTASK
// kBasePort gets the discriminator added to it to figure out the port the app
// should be using.  This ensures that apps with distinct discriminators use
// distinct ports.
static const uint16_t kMinDiscriminator = 1111;
static const uint16_t kBasePort = 5542 - kMinDiscriminator;
#endif // HAVE_NSTASK

@implementation MTRTestCase (ServerAppRunner)

#if HAVE_NSTASK
+ (NSTask *)doStartAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload
{
    __auto_type uniqueIndex = sAppRunnerIndex++;

    NSError * error;
    __auto_type * parsedPayload = [MTRSetupPayload setupPayloadWithOnboardingPayload:payload error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(parsedPayload);

    XCTAssertFalse(parsedPayload.hasShortDiscriminator);

    NSNumber * discriminator = parsedPayload.discriminator;
    XCTAssertGreaterThanOrEqual(discriminator.unsignedShortValue, kMinDiscriminator);

    NSNumber * passcode = parsedPayload.setupPasscode;

    __auto_type * executable = [NSString stringWithFormat:@"out/debug/%@-app/chip-%@-app", name, name];
    __auto_type * appTask = [self createTaskForPath:executable];

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
        [NSString stringWithFormat:@"/tmp/chip-%@-kvs%u", name, uniqueIndex],
        @"--product-id",
        [NSString stringWithFormat:@"%u", parsedPayload.productID.unsignedShortValue],
    ];

    __auto_type * allArguments = [forcedArguments arrayByAddingObjectsFromArray:arguments];
    [appTask setArguments:allArguments];

    NSString * outFile = [NSString stringWithFormat:@"/tmp/darwin/framework-tests/%@-app-%u.log", name, uniqueIndex];
    NSString * errorFile = [NSString stringWithFormat:@"/tmp/darwin/framework-tests/%@-app-err-%u.log", name, uniqueIndex];

    // Make sure the files exist.
    [[NSFileManager defaultManager] createFileAtPath:outFile contents:nil attributes:nil];
    [[NSFileManager defaultManager] createFileAtPath:errorFile contents:nil attributes:nil];

    appTask.standardOutput = [NSFileHandle fileHandleForWritingAtPath:outFile];
    appTask.standardError = [NSFileHandle fileHandleForWritingAtPath:errorFile];

    NSLog(@"Started chip-%@-app (%@) with arguments %@ stdout=%@ and stderr=%@", name, appTask, allArguments, outFile, errorFile);

    return appTask;
}
#endif // HAVE_NSTASK

- (BOOL)startAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload
{
#if !HAVE_NSTASK
    XCTFail("Unable to start server app when we do not have NSTask");
    return NO;
#else
    [self launchTask:[self.class doStartAppWithName:name arguments:arguments payload:payload]];
    return YES;
#endif // HAVE_NSTASK
}

+ (BOOL)startAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload
{
#if !HAVE_NSTASK
    XCTFail("Unable to start server app when we do not have NSTask");
    return NO;
#else
    [self launchTask:[self doStartAppWithName:name arguments:arguments payload:payload]];
    return YES;
#endif // HAVE_NSTASK
}

+ (unsigned)nextUniqueIndex
{
    return sAppRunnerIndex;
}

@end
