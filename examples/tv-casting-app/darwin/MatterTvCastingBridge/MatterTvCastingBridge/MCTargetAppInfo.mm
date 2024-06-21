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

#import "MCTargetAppInfo.h"

@implementation MCTargetAppInfo

- (instancetype)init
{
    self = [super init];
    if (self) {
        _vendorId = 0;
        _productId = 0;
    }
    return self;
}

- (instancetype)initWithVendorId:(uint16_t)vendorId
{
    return [self initWithVendorId:vendorId productId:0];
}

- (instancetype)initWithVendorId:(uint16_t)vendorId productId:(uint16_t)productId
{
    self = [super init];
    if (self) {
        _vendorId = vendorId;
        _productId = productId;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"MCTargetAppInfo: vendorId: %d, productId: %d", self.vendorId, self.productId];
}

@end
