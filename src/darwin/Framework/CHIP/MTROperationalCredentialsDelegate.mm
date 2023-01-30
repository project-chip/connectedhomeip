/**
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#import "MTROperationalCredentialsDelegate.h"

#import <Security/Security.h>

#import "MTRCertificates.h"
#import "MTRDeviceController_Internal.h"
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"

#include <controller/CommissioningDelegate.h>
#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationVendorReserved.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TimeUtils.h>
#include <platform/LockTracker.h>

using namespace chip;
using namespace TLV;
using namespace Credentials;
using namespace Crypto;

MTROperationalCredentialsDelegate::MTROperationalCredentialsDelegate(MTRDeviceController * deviceController)
    : mWeakController(deviceController)
{
}

CHIP_ERROR MTROperationalCredentialsDelegate::Init(MTRPersistentStorageDelegateBridge * storage, ChipP256KeypairPtr nocSigner,
    NSData * ipk, NSData * rootCert, NSData * _Nullable icaCert)
{
    if (storage == nil || ipk == nil || rootCert == nil) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mStorage = storage;

    mIssuerKey = nocSigner;

    if ([ipk length] != mIPK.Length()) {
        MTR_LOG_ERROR("MTROperationalCredentialsDelegate::init provided IPK is wrong size");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    memcpy(mIPK.Bytes(), [ipk bytes], [ipk length]);

    // Make copies of the certificates, just in case the API consumer
    // has them as MutableData.
    mRootCert = [NSData dataWithData:rootCert];
    if (mRootCert == nil) {
        return CHIP_ERROR_NO_MEMORY;
    }

    if (icaCert != nil) {
        mIntermediateCert = [NSData dataWithData:icaCert];
        if (mIntermediateCert == nil) {
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTROperationalCredentialsDelegate::GenerateNOC(
    NodeId nodeId, FabricId fabricId, const chip::CATValues & cats, const Crypto::P256PublicKey & pubkey, MutableByteSpan & noc)
{
    if (!mIssuerKey) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return GenerateNOC(
        *mIssuerKey, (mIntermediateCert != nil) ? mIntermediateCert : mRootCert, nodeId, fabricId, cats, pubkey, noc);
}

CHIP_ERROR MTROperationalCredentialsDelegate::GenerateNOC(P256Keypair & signingKeypair, NSData * signingCertificate, NodeId nodeId,
    FabricId fabricId, const CATValues & cats, const P256PublicKey & pubkey, MutableByteSpan & noc)
{
    uint32_t validityStart, validityEnd;

    if (!ToChipEpochTime(0, validityStart)) {
        MTR_LOG_ERROR("Failed in computing certificate validity start date");
        return CHIP_ERROR_INTERNAL;
    }

    if (!ToChipEpochTime(kCertificateValiditySecs, validityEnd)) {
        MTR_LOG_ERROR("Failed in computing certificate validity end date");
        return CHIP_ERROR_INTERNAL;
    }

    ChipDN signerSubject;
    ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(AsByteSpan(signingCertificate), signerSubject));

    ChipDN noc_dn;
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterFabricId(fabricId));
    ReturnErrorOnFailure(noc_dn.AddAttribute_MatterNodeId(nodeId));
    ReturnErrorOnFailure(noc_dn.AddCATs(cats));

    X509CertRequestParams noc_request = { 1, validityStart, validityEnd, noc_dn, signerSubject };
    return NewNodeOperationalX509Cert(noc_request, pubkey, signingKeypair, noc);
}

CHIP_ERROR MTROperationalCredentialsDelegate::GenerateNOCChain(const chip::ByteSpan & csrElements, const chip::ByteSpan & csrNonce,
    const chip::ByteSpan & attestationSignature, const chip::ByteSpan & attestationChallenge, const chip::ByteSpan & DAC,
    const chip::ByteSpan & PAI, chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion)
{
    if (mOperationalCertificateIssuer != nil) {
        return ExternalGenerateNOCChain(csrElements, csrNonce, attestationSignature, attestationChallenge, DAC, PAI, onCompletion);
    } else {
        return LocalGenerateNOCChain(csrElements, csrNonce, attestationSignature, attestationChallenge, DAC, PAI, onCompletion);
    }
}

CHIP_ERROR MTROperationalCredentialsDelegate::ExternalGenerateNOCChain(const chip::ByteSpan & csrElements,
    const chip::ByteSpan & csrNonce, const chip::ByteSpan & csrElementsSignature, const chip::ByteSpan & attestationChallenge,
    const chip::ByteSpan & DAC, const chip::ByteSpan & PAI,
    chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mCppCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MTRDeviceController * strongController = mWeakController;
    VerifyOrReturnError(strongController != nil, CHIP_ERROR_INCORRECT_STATE);

    mOnNOCCompletionCallback = onCompletion;

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

    chip::ByteSpan csr;
    reader.Get(csr);
    reader.ExitContainer(containerType);

    auto * csrInfo = [[MTROperationalCSRInfo alloc] initWithCSR:AsData(csr)
                                                       csrNonce:AsData(csrNonce)
                                                 csrElementsTLV:AsData(csrElements)
                                           attestationSignature:AsData(csrElementsSignature)];

    chip::ByteSpan certificationDeclarationSpan;
    chip::ByteSpan attestationNonceSpan;
    uint32_t timestampDeconstructed;
    chip::ByteSpan firmwareInfoSpan;
    chip::Credentials::DeviceAttestationVendorReservedDeconstructor vendorReserved;

    chip::Optional<chip::Controller::CommissioningParameters> commissioningParameters
        = mCppCommissioner->GetCommissioningParameters();
    VerifyOrReturnError(commissioningParameters.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    // Attestation Elements, nonce and signature will have a value in Commissioning Params as the CSR needs a signature or else we
    // cannot trust it
    ReturnErrorOnFailure(
        chip::Credentials::DeconstructAttestationElements(commissioningParameters.Value().GetAttestationElements().Value(),
            certificationDeclarationSpan, attestationNonceSpan, timestampDeconstructed, firmwareInfoSpan, vendorReserved));

    NSData * firmwareInfo = nil;
    if (!firmwareInfoSpan.empty()) {
        firmwareInfo = AsData(firmwareInfoSpan);
    }
    MTRDeviceAttestationInfo * attestationInfo = [[MTRDeviceAttestationInfo alloc]
               initWithDeviceAttestationChallenge:AsData(attestationChallenge)
                                            nonce:AsData(commissioningParameters.Value().GetAttestationNonce().Value())
                                      elementsTLV:AsData(commissioningParameters.Value().GetAttestationElements().Value())
                                elementsSignature:AsData(commissioningParameters.Value().GetAttestationSignature().Value())
                     deviceAttestationCertificate:AsData(DAC)
        productAttestationIntermediateCertificate:AsData(PAI)
                         certificationDeclaration:AsData(certificationDeclarationSpan)
                                     firmwareInfo:firmwareInfo];

    MTRDeviceController * __weak weakController = mWeakController;
    dispatch_async(mOperationalCertificateIssuerQueue, ^{
        [mOperationalCertificateIssuer
            issueOperationalCertificateForRequest:csrInfo
                                  attestationInfo:attestationInfo
                                       controller:strongController
                                       completion:^(MTROperationalCertificateChain * _Nullable chain, NSError * _Nullable error) {
                                           MTRDeviceController * strongController = weakController;
                                           if (strongController == nil || !strongController.isRunning) {
                                               // No longer safe to touch "this"
                                               return;
                                           }
                                           this->ExternalNOCChainGenerated(chain, error);
                                       }];
    });

    return CHIP_NO_ERROR;
}

void MTROperationalCredentialsDelegate::ExternalNOCChainGenerated(
    MTROperationalCertificateChain * _Nullable chain, NSError * _Nullable error)
{
    // Dispatch will only happen if the controller is still running, which means we
    // are safe to touch our members.
    [mWeakController
        asyncGetCommissionerOnMatterQueue:^(Controller::DeviceCommissioner * commissioner) {
            assertChipStackLockedByCurrentThread();

            if (mOnNOCCompletionCallback == nullptr) {
                return;
            }

            auto * onCompletion = mOnNOCCompletionCallback;
            mOnNOCCompletionCallback = nullptr;

            if (mCppCommissioner != commissioner) {
                // Quite unexpected!
                return;
            }

            if (chain == nil) {
                onCompletion->mCall(onCompletion->mContext, [MTRError errorToCHIPErrorCode:error], ByteSpan(), ByteSpan(),
                    ByteSpan(), NullOptional, NullOptional);
                return;
            }

            auto commissioningParameters = commissioner->GetCommissioningParameters();
            if (!commissioningParameters.HasValue()) {
                return;
            }

            IdentityProtectionKeySpan ipk = commissioningParameters.Value().GetIpk().ValueOr(GetIPK());

            Optional<NodeId> adminSubject;
            if (chain.adminSubject != nil) {
                adminSubject.SetValue(chain.adminSubject.unsignedLongLongValue);
            } else {
                adminSubject = commissioningParameters.Value().GetAdminSubject();
            }

            ByteSpan intermediateCertificate;
            if (chain.intermediateCertificate != nil) {
                intermediateCertificate = AsByteSpan(chain.intermediateCertificate);
            }

            onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, AsByteSpan(chain.operationalCertificate),
                intermediateCertificate, AsByteSpan(chain.rootCertificate), MakeOptional(ipk), adminSubject);
        }
                             // If we can't run the block, we're torn down and should
                             // just do nothing.
                             errorHandler:nil];
}

CHIP_ERROR MTROperationalCredentialsDelegate::LocalGenerateNOCChain(const chip::ByteSpan & csrElements,
    const chip::ByteSpan & csrNonce, const chip::ByteSpan & attestationSignature, const chip::ByteSpan & attestationChallenge,
    const chip::ByteSpan & DAC, const chip::ByteSpan & PAI,
    chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion)
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

    ReturnErrorOnFailure(GenerateNOC(assignedId, mNextFabricId, chip::kUndefinedCATs, pubkey, noc));

    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, noc, IntermediateCertSpan(), RootCertSpan(), MakeOptional(GetIPK()),
        Optional<NodeId>());

    return CHIP_NO_ERROR;
}

ByteSpan MTROperationalCredentialsDelegate::RootCertSpan() const { return AsByteSpan(mRootCert); }

ByteSpan MTROperationalCredentialsDelegate::IntermediateCertSpan() const
{
    if (mIntermediateCert == nil) {
        return ByteSpan();
    }

    return AsByteSpan(mIntermediateCert);
}

bool MTROperationalCredentialsDelegate::ToChipEpochTime(uint32_t offset, uint32_t & epoch)
{
    NSDate * date = [NSDate dateWithTimeIntervalSinceNow:offset];
    NSCalendar * calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    NSDateComponents * components = [calendar componentsInTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0] fromDate:date];

    uint16_t year = static_cast<uint16_t>([components year]);
    uint8_t month = static_cast<uint8_t>([components month]);
    uint8_t day = static_cast<uint8_t>([components day]);
    uint8_t hour = static_cast<uint8_t>([components hour]);
    uint8_t minute = static_cast<uint8_t>([components minute]);
    uint8_t second = static_cast<uint8_t>([components second]);
    return chip::CalendarToChipEpochTime(year, month, day, hour, minute, second, epoch);
}

namespace {
uint64_t GetIssuerId(NSNumber * _Nullable providedIssuerId)
{
    if (providedIssuerId != nil) {
        return [providedIssuerId unsignedLongLongValue];
    }

    return (uint64_t(arc4random()) << 32) | arc4random();
}
} // anonymous namespace

CHIP_ERROR MTROperationalCredentialsDelegate::GenerateRootCertificate(id<MTRKeypair> keypair, NSNumber * _Nullable issuerId,
    NSNumber * _Nullable fabricId, NSData * _Nullable __autoreleasing * _Nonnull rootCert)
{
    *rootCert = nil;
    MTRP256KeypairBridge keypairBridge;
    ReturnErrorOnFailure(keypairBridge.Init(keypair));

    ChipDN rcac_dn;
    ReturnErrorOnFailure(rcac_dn.AddAttribute_MatterRCACId(GetIssuerId(issuerId)));

    if (fabricId != nil) {
        FabricId fabric = [fabricId unsignedLongLongValue];
        VerifyOrReturnError(IsValidFabricId(fabric), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(rcac_dn.AddAttribute_MatterFabricId(fabric));
    }

    uint32_t validityStart, validityEnd;

    if (!ToChipEpochTime(0, validityStart)) {
        MTR_LOG_ERROR("Failed in computing certificate validity start date");
        return CHIP_ERROR_INTERNAL;
    }

    if (!ToChipEpochTime(kCertificateValiditySecs, validityEnd)) {
        MTR_LOG_ERROR("Failed in computing certificate validity end date");
        return CHIP_ERROR_INTERNAL;
    }

    uint8_t rcacBuffer[Controller::kMaxCHIPDERCertLength];
    MutableByteSpan rcac(rcacBuffer);
    X509CertRequestParams rcac_request = { 0, validityStart, validityEnd, rcac_dn, rcac_dn };
    ReturnErrorOnFailure(NewRootX509Cert(rcac_request, keypairBridge, rcac));
    *rootCert = AsData(rcac);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MTROperationalCredentialsDelegate::GenerateIntermediateCertificate(id<MTRKeypair> rootKeypair, NSData * rootCertificate,
    SecKeyRef intermediatePublicKey, NSNumber * _Nullable issuerId, NSNumber * _Nullable fabricId,
    NSData * _Nullable __autoreleasing * _Nonnull intermediateCert)
{
    *intermediateCert = nil;

    // Verify that the provided root certificate public key matches the root keypair.
    if ([MTRCertificates keypair:rootKeypair matchesCertificate:rootCertificate] == NO) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    MTRP256KeypairBridge keypairBridge;
    ReturnErrorOnFailure(keypairBridge.Init(rootKeypair));

    ByteSpan rcac = AsByteSpan(rootCertificate);

    P256PublicKey pubKey;
    ReturnErrorOnFailure(MTRP256KeypairBridge::MatterPubKeyFromSecKeyRef(intermediatePublicKey, &pubKey));

    ChipDN rcac_dn;
    ReturnErrorOnFailure(ExtractSubjectDNFromX509Cert(rcac, rcac_dn));

    ChipDN icac_dn;
    ReturnErrorOnFailure(icac_dn.AddAttribute_MatterICACId(GetIssuerId(issuerId)));
    if (fabricId != nil) {
        FabricId fabric = [fabricId unsignedLongLongValue];
        VerifyOrReturnError(IsValidFabricId(fabric), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(icac_dn.AddAttribute_MatterFabricId(fabric));
    }

    uint32_t validityStart, validityEnd;

    if (!ToChipEpochTime(0, validityStart)) {
        MTR_LOG_ERROR("Failed in computing certificate validity start date");
        return CHIP_ERROR_INTERNAL;
    }

    if (!ToChipEpochTime(kCertificateValiditySecs, validityEnd)) {
        MTR_LOG_ERROR("Failed in computing certificate validity end date");
        return CHIP_ERROR_INTERNAL;
    }

    uint8_t icacBuffer[Controller::kMaxCHIPDERCertLength];
    MutableByteSpan icac(icacBuffer);
    X509CertRequestParams icac_request = { 0, validityStart, validityEnd, icac_dn, rcac_dn };
    ReturnErrorOnFailure(NewICAX509Cert(icac_request, pubKey, keypairBridge, icac));
    *intermediateCert = AsData(icac);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MTROperationalCredentialsDelegate::GenerateOperationalCertificate(id<MTRKeypair> signingKeypair,
    NSData * signingCertificate, SecKeyRef operationalPublicKey, NSNumber * fabricId, NSNumber * nodeId,
    NSSet<NSNumber *> * _Nullable caseAuthenticatedTags, NSData * _Nullable __autoreleasing * _Nonnull operationalCert)
{
    *operationalCert = nil;

    // Verify that the provided signing certificate public key matches the signing keypair.
    if ([MTRCertificates keypair:signingKeypair matchesCertificate:signingCertificate] == NO) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if ([caseAuthenticatedTags count] > kMaxSubjectCATAttributeCount) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    FabricId fabric = [fabricId unsignedLongLongValue];
    VerifyOrReturnError(IsValidFabricId(fabric), CHIP_ERROR_INVALID_ARGUMENT);

    NodeId node = [nodeId unsignedLongLongValue];
    VerifyOrReturnError(IsOperationalNodeId(node), CHIP_ERROR_INVALID_ARGUMENT);

    MTRP256KeypairBridge keypairBridge;
    ReturnErrorOnFailure(keypairBridge.Init(signingKeypair));

    P256PublicKey pubKey;
    ReturnErrorOnFailure(MTRP256KeypairBridge::MatterPubKeyFromSecKeyRef(operationalPublicKey, &pubKey));

    CATValues cats;
    if (caseAuthenticatedTags != nil) {
        size_t idx = 0;
        for (NSNumber * cat in [caseAuthenticatedTags.allObjects sortedArrayUsingSelector:@selector(compare:)]) {
            cats.values[idx++] = [cat unsignedIntValue];
        }
    }

    uint8_t nocBuffer[Controller::kMaxCHIPDERCertLength];
    MutableByteSpan noc(nocBuffer);
    ReturnErrorOnFailure(GenerateNOC(keypairBridge, signingCertificate, node, fabric, cats, pubKey, noc));

    *operationalCert = AsData(noc);
    return CHIP_NO_ERROR;
}
