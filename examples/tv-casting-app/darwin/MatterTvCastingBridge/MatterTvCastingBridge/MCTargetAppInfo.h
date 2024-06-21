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

#ifndef MCTargetAppInfo_h
#define MCTargetAppInfo_h

/**
 * @brief Feature: Target Content Application - An entry in the TargetAppList which contains
 * a Target vendorId and an optional Target productId.
 */
@interface MCTargetAppInfo : NSObject

/** Target Target Content Application Vendor ID, 0 means unspecified */
@property (nonatomic, readonly) uint16_t vendorId;

/** Target Target Content Application Product ID, 0 means unspecified */
@property (nonatomic, readonly) uint16_t productId;

/** Initialize with vendorId and productId set to 0 */
- (instancetype)init;

/** Initialize with vendorId, productId defaults to 0 */
- (instancetype _Nonnull)initWithVendorId:(uint16_t)vendorId;

/** Initialize with vendorId and productId */
- (instancetype _Nonnull)initWithVendorId:(uint16_t)vendorId productId:(uint16_t)productId NS_DESIGNATED_INITIALIZER;

/** Description method */
- (NSString * _Nonnull)description;

@end

#endif /* MCTargetAppInfo_h */
