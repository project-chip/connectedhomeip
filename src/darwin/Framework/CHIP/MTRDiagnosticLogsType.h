/**
 *
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

#import <Matter/MTRDefines.h>

/**
 * This enum is used to specify the type of log requested from this device.
 *
 * The log types are : End User Support, Network Diagnostics and Crash logs.
 */
typedef NS_ENUM(NSInteger, MTRDiagnosticLogType) {
    MTRDiagnosticLogTypeEndUserSupport = 0, // End user support log is requested
    MTRDiagnosticLogTypeNetworkDiagnostics = 1, // Network Diagnostics log is requested
    MTRDiagnosticLogTypeCrash = 2 // Crash log is requested
} MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));
