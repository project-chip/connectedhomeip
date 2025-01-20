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

#import "MTRProductIdentity.h"

#import "MTRDefines_Internal.h"
#import "MTRUtilities.h"

#include <lib/support/CodeUtils.h>

MTR_DIRECT_MEMBERS
@implementation MTRProductIdentity

- (instancetype)initWithVendorID:(NSNumber *)vendorID productID:(NSNumber *)productID
{
    self = [super init];
    VerifyOrReturnValue(vendorID != nil && productID != nil, nil);
    _vendorID = vendorID;
    _productID = productID;
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    return self; // immutable
}

- (NSUInteger)hash
{
    return _vendorID.hash ^ _productID.hash;
}

- (BOOL)isEqual:(id)object
{
    VerifyOrReturnValue([object class] == [self class], NO);
    MTRProductIdentity * other = object;
    return MTREqualObjects(_vendorID, other.vendorID) && MTREqualObjects(_productID, other.productID);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: vid 0x%x pid 0x%x>", self.class, _vendorID.unsignedIntValue, _productID.unsignedIntValue];
}

@end
