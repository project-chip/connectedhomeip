/**
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

#import "MTRCertificateInfo.h"

#import "MTRConversion.h"
#import "MTRDefines_Internal.h"

#include <credentials/CHIPCert.h>

NS_ASSUME_NONNULL_BEGIN

using namespace chip;
using namespace chip::Credentials;
using namespace chip::ASN1;

@interface MTRCertificateInfo ()
- (nullable instancetype)initWithTLVBytes:(MTRCertificateTLVBytes)bytes NS_DESIGNATED_INITIALIZER;
@end

@interface MTRDistinguishedNameInfo ()
- (instancetype)initWithDN:(const ChipDN &)dn;
@end

MTR_DIRECT_MEMBERS
@implementation MTRCertificateInfo {
    NSData * _bytes; // needs to be kept around, _data may contain pointers into the backing buffer
    struct ChipCertificateData _data;
}

- (nullable instancetype)initWithTLVBytes:(MTRCertificateTLVBytes)bytes
{
    if (self = [super init]) {
        VerifyOrReturnValue(_bytes = [bytes copy], nil);
        VerifyOrReturnValue(DecodeChipCert(AsByteSpan(_bytes), _data) == CHIP_NO_ERROR, nil);
    }
    return self;
}

- (MTRDistinguishedNameInfo *)issuer
{
    return [[MTRDistinguishedNameInfo alloc] initWithDN:_data.mIssuerDN];
}

- (MTRDistinguishedNameInfo *)subject
{
    return [[MTRDistinguishedNameInfo alloc] initWithDN:_data.mSubjectDN];
}

- (NSDate *)notBefore
{
    return MatterEpochSecondsAsDate(_data.mNotBeforeTime);
}

- (NSDate *)notAfter
{
    // "no expiry" is encoded as kNullCertTime (see ChipEpochToASN1Time)
    return (_data.mNotAfterTime != kNullCertTime) ? MatterEpochSecondsAsDate(_data.mNotAfterTime) : NSDate.distantFuture;
}

- (id)copyWithZone:(nullable NSZone *)zone
{
    return self; // immutable, no need to copy
}

- (BOOL)isEqual:(id)object
{
    if (self == object) {
        return YES;
    }
    if ([object class] != MTRCertificateInfo.class) {
        return NO;
    }
    auto other = (MTRCertificateInfo *) object;
    return [_bytes isEqual:other->_bytes];
}

- (NSUInteger)hash
{
    return _bytes.hash;
}

@end

MTR_DIRECT_MEMBERS
@implementation MTRDistinguishedNameInfo {
    ChipDN _dn;
}

- (instancetype)initWithDN:(const ChipDN &)dn
{
    if (self = [super init]) {
        _dn = dn;
    }
    return self;
}

- (nullable NSNumber *)nodeID
{
    return [self identifierWithUniqueOID:kOID_AttributeType_MatterNodeId];
}

- (nullable NSNumber *)fabricID
{
    return [self identifierWithUniqueOID:kOID_AttributeType_MatterFabricId];
}

- (nullable NSNumber *)rootCACertificateID
{
    return [self identifierWithUniqueOID:kOID_AttributeType_MatterRCACId];
}

- (nullable NSNumber *)intermediateCACertificateID
{
    return [self identifierWithUniqueOID:kOID_AttributeType_MatterICACId];
}

- (nullable NSNumber *)identifierWithUniqueOID:(OID)oid
{
    NSAssert(IsChipDNAttr(oid), @"Invalid OID");

    ChipRDN const * match = nullptr;
    for (auto const & rdn : _dn.rdn) {
        if (rdn.IsEmpty()) {
            break;
        } else if (rdn.mAttrOID == oid) {
            VerifyOrReturnValue(!match, nil); // invalid, there should be only one matching RDN
            match = &rdn;
        }
    }

    return (match) ? @(match->mChipVal) : nil;
}

- (NSSet<NSNumber *> *)caseAuthenticatedTags
{
    NSMutableSet<NSNumber *> * result;
    for (auto const & rdn : _dn.rdn) {
        if (rdn.IsEmpty()) {
            break;
        } else if (rdn.mAttrOID == kOID_AttributeType_MatterCASEAuthTag) {
            NSNumber * tag = @(rdn.mChipVal);
            if (!result) {
                result = [NSMutableSet setWithObject:tag];
            } else {
                [result addObject:tag];
            }
        }
    }
    return (result) ? [result copy] : [NSSet set];
}

- (id)copyWithZone:(nullable NSZone *)zone
{
    return self; // immutable, no need to copy
}

- (BOOL)isEqual:(id)object
{
    if (self == object) {
        return YES;
    }
    if ([object class] != MTRDistinguishedNameInfo.class) {
        return NO;
    }
    auto other = (MTRDistinguishedNameInfo *) object;
    return _dn.IsEqual(other->_dn);
}

@end

NS_ASSUME_NONNULL_END
