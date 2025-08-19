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

NS_ASSUME_NONNULL_BEGIN

@protocol XPCServerBaseProtocol <NSObject>
- (void)start;
- (void)stop;
@end

@protocol XPCServerExternalCertificateParametersProtocol <XPCServerBaseProtocol, NSXPCListenerDelegate>
- (MTRDeviceController *)createController:(MTRDeviceControllerExternalCertificateParameters *)params error:(NSError * __autoreleasing *)error;
@end

@protocol XPCServerStartupParametersProtocol <XPCServerBaseProtocol, NSXPCListenerDelegate>
- (MTRDeviceController *)createController:(MTRDeviceControllerStartupParams *)params error:(NSError * __autoreleasing *)error;
@end

@protocol XPCServerExternalCertificateParametersWithServiceNameProtocol <XPCServerBaseProtocol>
- (MTRDeviceController *)createController:(NSString *)controllerID serviceName:(NSString *)serviceName error:(NSError * __autoreleasing *)error;
@end

@protocol XPCServerStartupParametersWithServiceNameProtocol <XPCServerBaseProtocol>
- (MTRDeviceController *)createController:(NSString *)controllerID serviceName:(NSString *)serviceName error:(NSError * __autoreleasing *)error;
@end

NS_ASSUME_NONNULL_END
