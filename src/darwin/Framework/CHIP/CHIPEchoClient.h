/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements describes a temporary helper class to implement an echo client for CHIP
 *      This may be removed from the framework soon.
 *
 *      Note: Do not use CHIPDeviceController directly if you use this class to initialize an EchoServer
 *
 *      Sample Usage:
 *        _echoClient = [[CHIPEchoClient alloc] initWithServerAddress:@"192.168.1.22" port:8000];
 *        if (_echoClient) {
 *            NSError * error;
 *            BOOL didStart = [_echoClient start:&error frequency:10 stopAfter:60];
 *            if (!didStart) {
 *                os_log(OS_LOG_DEFAULT, @"Failed to start echo client %@", error);
 *            }
 *         }
 *
 */

#ifndef ECHO_CLIENT_H
#define ECHO_CLIENT_H

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface CHIPEchoClient : NSObject


/// Initialize the CHIPEchoClient with a server address
/// @param ipAddress    A string representation of the server's IPAddress
/// @param port               The server's port
- (instancetype)initWithServerAddress:(NSString *)ipAddress port:(UInt16)port;

/// Start the Echo Client with the default configuration
/// By default, the echo client will send an echo to the server every 10 seconds indefinitely
/// @param error  Returns a CHIP Error Code if the client was unable to start
- (BOOL)start:(NSError * __autoreleasing *)error;

/// Start the Echo Client with a custom echo frequency
/// @param error               Returns a CHIP Error Code if the client was unable to start
/// @param frequency      The frequency, in seconds, of the client's echo messages
- (BOOL)start:(NSError * __autoreleasing *)error frequency:(UInt32)frequency;

/// Start the echo client with a custom echo frequency and automatically stop it after the specified time
/// @param error             Returns a CHIP Error Code if the client was unable to start
/// @param frequency    The frequency, in seconds, of the client's echo messages
/// @param stopAfter    The number of seconds after which the echo client will stop sending messages
- (BOOL)start:(NSError * __autoreleasing *)error frequency:(UInt32)frequency stopAfter:(UInt32)stopAfter;

/// Stop the echo client
- (void)stop;

@end

NS_ASSUME_NONNULL_END

#endif /* ECHO_CLIENT_H */
