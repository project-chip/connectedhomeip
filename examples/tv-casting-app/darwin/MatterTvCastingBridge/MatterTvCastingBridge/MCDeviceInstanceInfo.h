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

#import <Foundation/Foundation.h>

#ifndef MCDeviceInstanceInfo_h
#define MCDeviceInstanceInfo_h

@interface MCDeviceInstanceInfo : NSObject

@property (nonatomic, strong, readonly) NSString * _Nullable vendorName;

@property (nonatomic, strong, readonly) NSString * _Nullable productName;

@property (nonatomic, strong, readonly) NSNumber * _Nullable vendorId;

@property (nonatomic, strong, readonly) NSNumber * _Nullable productId;

@property (nonatomic, strong, readonly) NSString * _Nullable serialNumber;

@property (nonatomic, strong, readonly) NSNumber * _Nullable hardwareVersion;

@property (nonatomic, strong, readonly) NSString * _Nullable hardwareVersionString;

- (instancetype _Nullable)initWithVendorName:(NSString * _Nullable)vendorName
                                 productName:(NSString * _Nullable)productName
                                    vendorId:(NSNumber * _Nullable)vendorId
                                   productId:(NSNumber * _Nullable)productId
                                serialNumber:(NSString * _Nullable)serialNumber
                             hardwareVersion:(NSNumber * _Nullable)hardwareVersion
                       hardwareVersionString:(NSString * _Nullable)hardwareVersionString;

@end

#endif /* MCDeviceInstanceInfo_h */
