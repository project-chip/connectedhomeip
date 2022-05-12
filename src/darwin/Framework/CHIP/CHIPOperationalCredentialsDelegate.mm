/**
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <algorithm>

#import "CHIPOperationalCredentialsDelegate.h"

#import <Security/Security.h>

#include <Security/SecKey.h>

#import "CHIPLogging.h"

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/Optional.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TimeUtils.h>

constexpr const char kOperationalCredentialsRootCertificateStorage[] = "MatterCARootCert";

using namespace chip;
using namespace TLV;
using namespace Credentials;
using namespace Crypto;

CHIP_ERROR CHIPOperationalCredentialsDelegate::init(
    CHIPPersistentStorageDelegateBridge * storage, ChipP256KeypairPtr nocSigner, NSData * ipk)
{
    if (storage == nil || nocSigner == nullptr || ipk == nil) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mStorage = storage;

    mIssuerKey = std::move(nocSigner);

    if ([ipk length] != mIPK.Length()) {
        CHIP_LOG_ERROR("CHIPOperationalCredentialsDelegate::init provided IPK is wrong size");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    memcpy(mIPK.Bytes(), [ipk bytes], [ipk length]);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::GenerateNOCChainAfterValidation(NodeId nodeId, FabricId fabricId,
    const chip::CATValues & cats, const Crypto::P256PublicKey & pubkey, MutableByteSpan & rcac, MutableByteSpan & icac,
    MutableByteSpan & noc)
{
    uint32_t validityStart, validityEnd;
    bool haveRootCert = false;

    if (!ToChipEpochTime(0, validityStart)) {
        NSLog(@"Failed in computing certificate validity start date");
        return CHIP_ERROR_INTERNAL;
    }

    if (!ToChipEpochTime(kCertificateValiditySecs, validityEnd)) {
        NSLog(@"Failed in computing certificate validity end date");
        return CHIP_ERROR_INTERNAL;
    }

    ChipDN rcac_dn;
    uint16_t rcacBufLen = static_cast<uint16_t>(std::min(rcac.size(), static_cast<size_t>(UINT16_MAX)));
    PERSISTENT_KEY_OP(fabricId, kOperationalCredentialsRootCertificateStorage, key,
        haveRootCert = (mStorage->SyncGetKeyValue(key, rcac.data(), rcacBufLen) == CHIP_NO_ERROR));
    if (haveRootCert) {
        uint64_t rcacId;
        rcac.reduce_size(rcacBufLen);
        ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(rcac, rcac_dn));
        ReturnErrorOnFailure(rcac_dn.GetCertChipId(rcacId));
        VerifyOrReturnError(rcacId == mIssuerId, CHIP_ERROR_INTERNAL);
    } else {
        ReturnErrorOnFailure(rcac_dn.AddAttribute_MatterRCACId(mIssuerId));
        ReturnErrorOnFailure(rcac_dn.AddAttribute_MatterFabricId(fabricId));

        NSLog(@"Generating RCAC");
        X509CertRequestParams rcac_request = { 0, validityStart, validityEnd, rcac_dn, rcac_dn };
        ReturnErrorOnFailure(NewRootX509Cert(rcac_request, *mIssuerKey, rcac));

        VerifyOrReturnError(CanCastTo<uint16_t>(rcac.size()), CHIP_ERROR_INTERNAL);
        PERSISTENT_KEY_OP(fabricId, kOperationalCredentialsRootCertificateStorage, key,
            ReturnErrorOnFailure(mStorage->SyncSetKeyValue(key, rcac.data(), static_cast<uint16_t>(rcac.size()))));
    }

    icac.reduce_size(0);

    ChipDN noc_dn;
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterFabricId(fabricId));
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterNodeId(nodeId));
    ReturnErrorOnFailure(noc_dn.AddCATs(cats));

    X509CertRequestParams noc_request = { 1, validityStart, validityEnd, noc_dn, rcac_dn };
    return NewNodeOperationalX509Cert(noc_request, pubkey, *mIssuerKey, noc);
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::GenerateNOCChain(const chip::ByteSpan & csrElements, const chip::ByteSpan & csrNonce,
    const chip::ByteSpan & attestationSignature, const chip::ByteSpan & attestationChallenge, const chip::ByteSpan & DAC,
    const chip::ByteSpan & PAI, chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion)
{
    chip::NodeId assignedId;
    if (mNodeIdRequested) {
        assignedId = mNextRequestedNodeId;
        mNodeIdRequested = false;
    } else {
        if (mDeviceBeingPaired == chip::kUndefinedNodeId) {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        assignedId = mDeviceBeingPaired;
    }

    TLVReader reader;
    reader.Init(csrElements);

    if (reader.GetType() == kTLVType_NotSpecified) {
        ReturnErrorOnFailure(reader.Next());
    }

    VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == AnonymousTag(), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    reader.ExitContainer(containerType);

    chip::Crypto::P256PublicKey pubkey;
    ReturnErrorOnFailure(chip::Crypto::VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    NSMutableData * nocBuffer = [[NSMutableData alloc] initWithLength:chip::Controller::kMaxCHIPDERCertLength];
    MutableByteSpan noc((uint8_t *) [nocBuffer mutableBytes], chip::Controller::kMaxCHIPDERCertLength);

    NSMutableData * rcacBuffer = [[NSMutableData alloc] initWithLength:chip::Controller::kMaxCHIPDERCertLength];
    MutableByteSpan rcac((uint8_t *) [rcacBuffer mutableBytes], chip::Controller::kMaxCHIPDERCertLength);

    MutableByteSpan icac;

    ReturnErrorOnFailure(GenerateNOCChainAfterValidation(assignedId, mNextFabricId, chip::kUndefinedCATs, pubkey, rcac, icac, noc));

    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, noc, icac, rcac, MakeOptional(GetIPK()), Optional<NodeId>());

    return CHIP_NO_ERROR;
}

bool CHIPOperationalCredentialsDelegate::ToChipEpochTime(uint32_t offset, uint32_t & epoch)
{
    NSDate * date = [NSDate dateWithTimeIntervalSinceNow:offset];
    unsigned units = NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute
        | NSCalendarUnitSecond;
    NSCalendar * calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    NSDateComponents * components = [calendar components:units fromDate:date];

    uint16_t year = static_cast<uint16_t>([components year]);
    uint8_t month = static_cast<uint8_t>([components month]);
    uint8_t day = static_cast<uint8_t>([components day]);
    uint8_t hour = static_cast<uint8_t>([components hour]);
    uint8_t minute = static_cast<uint8_t>([components minute]);
    uint8_t second = static_cast<uint8_t>([components second]);
    return chip::CalendarToChipEpochTime(year, month, day, hour, minute, second, epoch);
}
