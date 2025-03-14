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

#import "XPCServerRegistry.h"

#import "XPCServerProtocols.h"
#import <objc/runtime.h>

@interface XPCServerRegistry ()
@property (strong, nonatomic) NSMutableArray<id<XPCServerBaseProtocol>> * servers;
@end

@implementation XPCServerRegistry
- (instancetype)init
{
    if ((self = [super init])) {
        _servers = [NSMutableArray new];
    }

    return self;
}

+ (XPCServerRegistry *)sharedInstance
{
    static XPCServerRegistry * registry = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        registry = [[XPCServerRegistry alloc] init];
    });
    return registry;
}

- (void)start
{
    if (@available(macOS 13.0, *)) {
        objc_enumerateClasses(nil, nil, @protocol(XPCServerBaseProtocol), nil, ^(Class cls, BOOL * stop) {
            id server = [[cls alloc] init];
            [_servers addObject:server];
        });
    } else {
        int numClasses = objc_getClassList(NULL, 0);
        if (numClasses > 0) {
            __auto_type * classes = (__unsafe_unretained Class *) malloc(sizeof(Class) * static_cast<size_t>(numClasses));
            numClasses = objc_getClassList(classes, numClasses);

            for (int i = 0; i < numClasses; i++) {
                id cls = classes[i];
                if (class_conformsToProtocol(cls, @protocol(XPCServerBaseProtocol))) {
                    id server = [[cls alloc] init];
                    [_servers addObject:server];
                }
            }
            free(classes); // Free the allocated memory
        }
    }

    for (id<XPCServerBaseProtocol> server in _servers) {
        [server start];
    }
}

- (void)stop
{
    for (id<XPCServerBaseProtocol> server in _servers) {
        [server stop];
    }
}

- (MTRDeviceController *)createController:(NSString * _Nullable)controllerId serviceName:(NSString * _Nullable)serviceName params:(id)params error:(NSError * __autoreleasing *)error
{
    BOOL isExternalCertificateParameters = [params isKindOfClass:[MTRDeviceControllerExternalCertificateParameters class]];
    BOOL isStartupParameters = [params isKindOfClass:[MTRDeviceControllerStartupParams class]];
    for (id server in _servers) {
        if (controllerId && serviceName) {
            if ([server conformsToProtocol:@protocol(XPCServerExternalCertificateParametersWithServiceNameProtocol)] && isExternalCertificateParameters) {
                return [server createController:controllerId serviceName:serviceName error:error];
            }

            if ([server conformsToProtocol:@protocol(XPCServerStartupParametersWithServiceNameProtocol)] && isStartupParameters) {
                return [server createController:controllerId serviceName:serviceName error:error];
            }
        } else {
            if ([server conformsToProtocol:@protocol(XPCServerExternalCertificateParametersProtocol)] && isExternalCertificateParameters) {
                return [server createController:params error:error];
            }

            if ([server conformsToProtocol:@protocol(XPCServerStartupParametersProtocol)] && isStartupParameters) {
                return [server createController:params error:error];
            }
        }
    }

    __auto_type * userInfo = @{ NSLocalizedDescriptionKey : @"No XPC servers support this configuration." };
    *error = [NSError errorWithDomain:@"Error" code:0 userInfo:userInfo];
    return nil;
}

@end
