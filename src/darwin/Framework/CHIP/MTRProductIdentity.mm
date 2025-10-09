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
@implementation MTRProductIdentity {
    uint16_t _vendorID;
    uint16_t _productID;
}

- (instancetype)initWithVendorID:(NSNumber *)vendorID productID:(NSNumber *)productID
{
    self = [super init];
    VerifyOrReturnValue(vendorID != nil && productID != nil, nil);
    _vendorID = vendorID.unsignedShortValue;
    _productID = productID.unsignedShortValue;
    return self;
}

static NSString * const sVendorIDKey = @"v";
static NSString * const sProductIDKey = @"p";

- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    _vendorID = static_cast<uint16_t>([coder decodeIntForKey:sVendorIDKey]);
    _productID = static_cast<uint16_t>([coder decodeIntForKey:sProductIDKey]);
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeInt:_vendorID forKey:sVendorIDKey];
    [coder encodeInt:_productID forKey:sProductIDKey];
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (id)copyWithZone:(NSZone *)zone
{
    return self; // immutable
}

- (NSUInteger)hash
{
    return (_vendorID << 16) | _productID;
}

- (BOOL)isEqual:(id)object
{
    VerifyOrReturnValue([object class] == [self class], NO);
    MTRProductIdentity * other = object;
    VerifyOrReturnValue(_vendorID == other->_vendorID, NO);
    VerifyOrReturnValue(_productID == other->_productID, NO);
    return YES;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: vid 0x%x pid 0x%x>", self.class, _vendorID, _productID];
}

- (NSNumber *)vendorID
{
    return @(_vendorID);
}

- (NSNumber *)productID
{
    return @(_productID);
}

@end
