/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>

#import "MTRDevice.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * This class handles downloading a log file requested by the MTRDevice. It creates a new
 * MTRDiagnosticsLogTransferHandler to prepare for the BDX transfer session and handle the BDX
 * messages and transfer events and downloads the file.
 *
 */
MTR_HIDDEN
@interface MTRDiagnosticsLogDownloadTask : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithDevice:(MTRDevice *)device;

- (void)downloadLogOfType:(MTRDiagnosticLogType)type
                  timeout:(NSTimeInterval)timeout
                    queue:(dispatch_queue_t)queue
               completion:(void (^)(NSURL * _Nullable logResult, NSError * error))completion;
@end

NS_ASSUME_NONNULL_END
