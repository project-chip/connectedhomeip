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
#import "MTRConversion.h"
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
#include <platform/LockTracker.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace TLV;
using namespace Credentials;
using namespace Crypto;

namespace {
// Upper bound on how long we will wait for the external operational-certificate
// issuer to invoke its completion before we fail the SendNOC commissioning
// stage with CHIP_ERROR_TIMEOUT. Sized comfortably under the default 60s
// failsafe so a hung 3rd-party issuer cannot consume the entire failsafe
// window, and comfortably over normal issuer round-trips (sub-second to a few
// seconds in healthy cases).
constexpr NSTimeInterval kExternalNOCChainIssuerWatchdogTimeoutSeconds = 20.0;
} // anonymous namespace

MTROperationalCredentialsDelegate::MTROperationalCredentialsDelegate(MTRDeviceController_Concrete * deviceController)
    : mWeakController(deviceController)
{
}

MTROperationalCredentialsDelegate::~MTROperationalCredentialsDelegate()
{
    // If we are torn down while the external issuer watchdog is still armed
    // (e.g. controller shutdown races a hung 3rd-party issuer), libdispatch
    // requires we cancel the source before releasing it; otherwise it traps
    // with "BUG IN CLIENT OF LIBDISPATCH: Release of a source that has not
    // been cancelled". cancel is idempotent, so this is safe even if the
    // watchdog already fired or was cancelled by ExternalNOCChainGenerated.
    //
    // Contract for callers:
    //   (i) The chip stack lock MUST NOT be held by the calling thread:
    //       we issue dispatch_sync against the Matter work queue below,
    //       and if a handler running on the Matter queue tries to take the
    //       stack lock (it does, via assertChipStackLockedByCurrentThread)
    //       we would deadlock. There is no helper to assert "not held" in
    //       LockTracker today, so this is comment-enforced.
    //   (ii) Destruction from the Matter queue with the watchdog still armed
    //       is UNSUPPORTED and trapped below — we cannot dispatch_sync to
    //       ourselves. Callers must teardown off the Matter queue.
    //   (iii) The two dispatch_sync fences serialize against two distinct
    //       enqueued blocks: the watchdog handler itself, and the single
    //       dispatch_async hop the handler performs through
    //       asyncGetCommissionerOnMatterQueue (depth-1 contract — see the
    //       parallel comment in MTRDeviceController_Concrete.mm).
    if (mExternalNOCChainWatchdog != nil) {
        // Use the queue we cached at watchdog-arm time; refetching it here
        // could yield nil during late-stage platform shutdown and silently
        // skip the fence (UAF window).
        VerifyOrDie(mWatchdogQueue != nil);
        dispatch_source_cancel(mExternalNOCChainWatchdog);
        if (chip::DeviceLayer::PlatformMgrImpl().IsWorkQueueCurrentQueue()) {
            // Same-queue destruction with an armed watchdog cannot be made
            // safe via dispatch_sync (we would dispatch_sync to ourselves
            // and deadlock). Trap so callers fix their teardown path
            // instead of silently risking UAF.
            VerifyOrDieWithMsg(false, AppServer,
                "MTROperationalCredentialsDelegate destroyed on Matter queue with armed watchdog");
        }
        // Two fences: first drains the watchdog handler itself; second
        // drains any asyncGetCommissionerOnMatterQueue inner block the
        // handler enqueued. See the original comment block (kept above
        // in spirit) for the FIFO-ordering argument.
        dispatch_sync(mWatchdogQueue, ^ {
                          // Intentional no-op: the act of running on the Matter queue
                          // is the fence we need (handler completion).
                      });
        dispatch_sync(mWatchdogQueue, ^ {
                          // Second fence: drain any block the handler enqueued
                          // (e.g. the asyncGetCommissionerOnMatterQueue inner
                          // block from ExternalNOCChainGenerated) that touches
                          // `this`, before we return and let `this` be freed.
                      });
        mExternalNOCChainWatchdog = nil;
    }
}

CHIP_ERROR MTROperationalCredentialsDelegate::Init(
    ChipP256KeypairPtr nocSigner, NSData * ipk, NSData * rootCert, NSData * _Nullable icaCert)
{
    if (ipk == nil || rootCert == nil) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mIssuerKey = nocSigner;

    if ([ipk length] != mIPK.Length()) {
        MTR_LOG_ERROR("MTROperationalCredentialsDelegate::init provided IPK is wrong size");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    memcpy(mIPK.Bytes(), [ipk bytes], [ipk length]);

    // Make copies of the certificates, just in case the API consumer
    // has them as MutableData.
    mRootCert = [rootCert copy];
    mIntermediateCert = [icaCert copy];

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTROperationalCredentialsDelegate::GenerateNOC(
    NodeId nodeId, FabricId fabricId, const chip::CATValues & cats, const Crypto::P256PublicKey & pubkey, MutableByteSpan & noc)
{
    if (!mIssuerKey) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto * validityPeriod = [[NSDateInterval alloc] initWithStartDate:[NSDate now] endDate:[NSDate distantFuture]];
    return GenerateNOC(*mIssuerKey, (mIntermediateCert != nil) ? mIntermediateCert : mRootCert, nodeId, fabricId, cats, pubkey,
        validityPeriod, noc);
}

CHIP_ERROR MTROperationalCredentialsDelegate::GenerateNOC(P256Keypair & signingKeypair, NSData * signingCertificate, NodeId nodeId,
    FabricId fabricId, const CATValues & cats, const P256PublicKey & pubkey, NSDateInterval * validityPeriod, MutableByteSpan & noc)
{
    uint32_t validityStart, validityEnd;

    if (!ToChipEpochTime(validityPeriod.startDate, validityStart)) {
        MTR_LOG_ERROR("Failed in computing certificate validity start date");
        return CHIP_ERROR_INTERNAL;
    }

    if (!ToChipNotAfterEpochTime(validityPeriod.endDate, validityEnd)) {
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

    MTRDeviceController_Concrete * strongController = mWeakController;
    VerifyOrReturnError(strongController != nil, CHIP_ERROR_INCORRECT_STATE);

    mOnNOCCompletionCallback = onCompletion;

    auto * csrInfo = [[MTROperationalCSRInfo alloc] initWithCSRNonce:AsData(csrNonce)
                                                      csrElementsTLV:AsData(csrElements)
                                                attestationSignature:AsData(csrElementsSignature)];

    chip::ByteSpan certificationDeclarationSpan;
    chip::ByteSpan attestationNonceSpan;
    uint32_t timestampDeconstructed;
    chip::ByteSpan firmwareInfoSpan;
    chip::Credentials::DeviceAttestationVendorReservedDeconstructor vendorReserved;

    auto & commissioningParameters = mCppCommissioner->GetCommissioningParameters();

    // Attestation Elements, nonce and signature will have a value in Commissioning Params as the CSR needs a signature or else we
    // cannot trust it
    ReturnErrorOnFailure(
        chip::Credentials::DeconstructAttestationElements(commissioningParameters.GetAttestationElements().Value(),
            certificationDeclarationSpan, attestationNonceSpan, timestampDeconstructed, firmwareInfoSpan, vendorReserved));

    NSData * firmwareInfo = nil;
    if (!firmwareInfoSpan.empty()) {
        firmwareInfo = AsData(firmwareInfoSpan);
    }
    MTRDeviceAttestationInfo * attestationInfo = [[MTRDeviceAttestationInfo alloc]
               initWithDeviceAttestationChallenge:AsData(attestationChallenge)
                                            nonce:AsData(commissioningParameters.GetAttestationNonce().Value())
                                      elementsTLV:AsData(commissioningParameters.GetAttestationElements().Value())
                                elementsSignature:AsData(commissioningParameters.GetAttestationSignature().Value())
                     deviceAttestationCertificate:AsData(DAC)
        productAttestationIntermediateCertificate:AsData(PAI)
                         certificationDeclaration:AsData(certificationDeclarationSpan)
                                     firmwareInfo:firmwareInfo];

    // Capture the issuer object into a local so the dispatch_async block does
    // NOT dereference `this` to read `mOperationalCertificateIssuer` on the
    // issuer queue. The issuer queue is API-consumer-owned and not fenced by
    // the destructor; reading `this->mOperationalCertificateIssuer` off-queue
    // would race a controller cleanup that has already deleted this delegate
    // (cleanup sets _cppCommissioner=nullptr THEN deletes the delegate, so an
    // issuer-queue block that already cleared the isRunning gate could still
    // touch freed memory). Holding the issuer in the block via this local
    // strong reference avoids the `this->` deref entirely. The inner completion
    // block still gates on strongController.isRunning before re-entering the
    // SDK; a narrow (delete-after-gate-passes) UAF window remains there but
    // is bounded by the watchdog-teardown fence path, since the watchdog is
    // armed for the entire issuer-callback wait window.
    id<MTROperationalCertificateIssuer> issuer = mOperationalCertificateIssuer;
    MTRDeviceController_Concrete * __weak weakController = mWeakController;
    dispatch_async(mOperationalCertificateIssuerQueue, ^{
        [issuer
            issueOperationalCertificateForRequest:csrInfo
                                  attestationInfo:attestationInfo
                                       controller:strongController
                                       completion:^(MTROperationalCertificateChain * _Nullable chain, NSError * _Nullable error) {
                                           MTRDeviceController_Concrete * strongController = weakController;
                                           if (strongController == nil || !strongController.isRunning) {
                                               // No longer safe to touch "this"
                                               return;
                                           }
                                           this->ExternalNOCChainGenerated(chain, error);
                                       }];
    });

    // Arm a watchdog so a 3rd-party issuer that crashes or hangs cannot stall
    // SendNOC for the full MRP-inflated CommandSender exchange timeout (~67s).
    // The timer is created suspended, configured, then resumed; the handler
    // simply synthesizes a timeout error through ExternalNOCChainGenerated,
    // which already self-guards against double-callback via
    // mOnNOCCompletionCallback == nullptr.
    //
    // Defensively cancel any prior watchdog before overwriting the member.
    // Today the single-in-flight invariant of GenerateNOCChain holds, but if
    // a future re-entry path skips ExternalNOCChainGenerated this prevents
    // releasing an active dispatch_source (libdispatch trap) and leaking a
    // stale timer that would fire against the new commissioning.
    if (mExternalNOCChainWatchdog != nil) {
        MTR_LOG_ERROR("MTROperationalCredentialsDelegate: external NOC chain watchdog re-armed without prior completion; cancelling stale timer");
        dispatch_source_cancel(mExternalNOCChainWatchdog);
        mExternalNOCChainWatchdog = nil;
    }
    // Run the watchdog on the Matter work queue so its event handler is
    // serialized with the controller / delegate teardown path. The
    // destructor cancels and then dispatch_sync's onto this same queue,
    // which fences any in-flight handler invocation before `this` is
    // freed. This closes the use-after-free window that exists when the
    // handler runs on an unrelated queue (e.g. global QOS_USER_INITIATED)
    // concurrent with delegate destruction.
    dispatch_queue_t watchdogQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();
    VerifyOrDie(watchdogQueue != nullptr);
    // Cache the queue for the destructor to fence on. Refetching it in
    // the destructor risks a nil return during platform teardown, which
    // would silently skip the fence and reopen the UAF window.
    mWatchdogQueue = watchdogQueue;
    dispatch_source_t watchdog = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, watchdogQueue);
    dispatch_source_set_timer(watchdog,
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(kExternalNOCChainIssuerWatchdogTimeoutSeconds * NSEC_PER_SEC)),
        DISPATCH_TIME_FOREVER,
        static_cast<uint64_t>(0.5 * NSEC_PER_SEC));
    dispatch_source_set_event_handler(watchdog, ^{
        // We are on the Matter queue here; the destructor's
        // dispatch_source_cancel + dispatch_sync(matterQueue) fence
        // guarantees this handler cannot run after `this` has been freed.
        dispatch_source_cancel(watchdog);
        // Always log that the watchdog fired, including the controller-gone
        // case, so we can post-hoc tell a hung issuer apart from a healthy
        // teardown.
        MTR_LOG_ERROR("MTROperationalCredentialsDelegate: external NOC chain issuer did not respond within %.0fs; failing commissioning (node 0x%016llx fabric 0x%016llx)",
            kExternalNOCChainIssuerWatchdogTimeoutSeconds,
            (unsigned long long) mDeviceBeingPaired,
            (unsigned long long) mNextFabricId);
        MTRDeviceController_Concrete * strongController = weakController;
        if (strongController == nil || !strongController.isRunning) {
            MTR_LOG_ERROR("MTROperationalCredentialsDelegate: external NOC chain issuer watchdog fired but controller is gone; abandoning completion (node 0x%016llx fabric 0x%016llx)",
                (unsigned long long) mDeviceBeingPaired,
                (unsigned long long) mNextFabricId);
            // The chip commissioning stack owns its Callback's lifetime and
            // tears it down during DeviceCommissioner::Shutdown; we must not
            // invoke it here (the stack lock may already be unsafe to take and
            // the Callback memory may already be released). Clear our member
            // references so a future ExternalGenerateNOCChain re-arm sees clean
            // state and the destructor doesn't run unnecessary fences.
            // mWatchdogQueue stays cached; only reset on next arm.
            mOnNOCCompletionCallback = nullptr;
            mExternalNOCChainWatchdog = nil;
            return;
        }
        NSError * timeoutError = [MTRError errorForCHIPErrorCode:CHIP_ERROR_TIMEOUT];
        this->ExternalNOCChainGenerated(nil, timeoutError);
    });
    mExternalNOCChainWatchdog = watchdog;
    dispatch_resume(watchdog);

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
                // Either the watchdog already drove a timeout completion, or
                // we are seeing a second invocation from a misbehaving issuer.
                // Both are no-ops here; log so post-hoc we can tell whether a
                // hung issuer eventually responded after the watchdog fired.
                MTR_LOG_ERROR("MTROperationalCredentialsDelegate: ExternalNOCChainGenerated callback already consumed; ignoring (error=%@)", error);
                return;
            }

            // Cancel the issuer watchdog so it cannot fire after we've already
            // dispatched the real completion. Idempotent against the case
            // where the watchdog itself drove us here (it cancels itself in
            // its event handler before re-entering).
            if (mExternalNOCChainWatchdog != nil) {
                dispatch_source_cancel(mExternalNOCChainWatchdog);
                mExternalNOCChainWatchdog = nil;
            }

            auto * onCompletion = mOnNOCCompletionCallback;
            mOnNOCCompletionCallback = nullptr;

            if (mCppCommissioner != commissioner) {
                MTR_LOG_ERROR("MTROperationalCredentialsDelegate: commissioner mismatch in ExternalNOCChainGenerated for node 0x%016llx fabric 0x%016llx (expected %p got %p); failing commissioning",
                    (unsigned long long) mDeviceBeingPaired,
                    (unsigned long long) mNextFabricId,
                    mCppCommissioner,
                    commissioner);
                if (onCompletion != nullptr) {
                    onCompletion->mCall(onCompletion->mContext, CHIP_ERROR_INCORRECT_STATE,
                        ByteSpan(), ByteSpan(), ByteSpan(), NullOptional, NullOptional);
                }
                return;
            }

            if (chain == nil) {
                onCompletion->mCall(onCompletion->mContext, [MTRError errorToCHIPErrorCode:error], ByteSpan(), ByteSpan(),
                    ByteSpan(), NullOptional, NullOptional);
                return;
            }

            auto & commissioningParameters = commissioner->GetCommissioningParameters();

            IdentityProtectionKeySpan ipk = commissioningParameters.GetIpk().ValueOr(GetIPK());

            Optional<NodeId> adminSubject;
            if (chain.adminSubject != nil) {
                adminSubject.SetValue(chain.adminSubject.unsignedLongLongValue);
            } else {
                adminSubject = commissioningParameters.GetAdminSubject();
            }

            ByteSpan intermediateCertificate;
            if (chain.intermediateCertificate != nil) {
                intermediateCertificate = AsByteSpan(chain.intermediateCertificate);
            }

            onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, AsByteSpan(chain.operationalCertificate),
                intermediateCertificate, AsByteSpan(chain.rootCertificate), MakeOptional(ipk), adminSubject);
        }
        // Note: we deliberately do NOT cancel mExternalNOCChainWatchdog here. If the
        // watchdog is still armed and later fires, it will route through the
        // controller-gone early-return path (handler line ~314) which is a no-op
        // log on the Matter queue. The redundant timer fire is preferable to a
        // cross-queue cancel that would race the destructor's fence.
        errorHandler:^(NSError * _Nullable controllerError) {
            // Log-only: this errorHandler may run on EITHER the Matter queue (second
            // checkIsRunning, async path) OR the original caller's queue (first
            // checkIsRunning, synchronous path — which for a real-completion call from
            // the issuer queue means the issuer queue). Therefore we must not touch
            // any `this->` member here; the watchdog handler / next ExternalGenerateNOCChain
            // owns mOnNOCCompletionCallback lifecycle on the Matter queue.
            MTR_LOG_ERROR("MTROperationalCredentialsDelegate: controller gone before NOC completion could be delivered for node 0x%016llx fabric 0x%016llx: %@",
                (unsigned long long) mDeviceBeingPaired,
                (unsigned long long) mNextFabricId,
                controllerError);
        }];
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

    ReturnErrorOnFailure(reader.Expect(kTLVType_Structure, AnonymousTag()));

    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    ReturnErrorOnFailure(reader.ExitContainer(containerType));

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

bool MTROperationalCredentialsDelegate::ToChipNotAfterEpochTime(NSDate * date, uint32_t & epoch)
{
    if ([date isEqualToDate:[NSDate distantFuture]]) {
        epoch = kNullCertTime;
        return true;
    }

    return ToChipEpochTime(date, epoch);
}

bool MTROperationalCredentialsDelegate::ToChipEpochTime(NSDate * date, uint32_t & epoch)
{
    if (DateToMatterEpochSeconds(date, epoch)) {
        return true;
    }

    NSCalendar * calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    NSDateComponents * components = [calendar componentsInTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0] fromDate:date];
    MTR_LOG_ERROR(
        "Year %lu is out of range for Matter epoch time.  Please use [NSDate distantFuture] to represent \"never expires\".",
        static_cast<unsigned long>(components.year));
    return false;
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
    NSNumber * _Nullable fabricId, NSDateInterval * validityPeriod, NSData * _Nullable __autoreleasing * _Nonnull rootCert)
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

    if (!ToChipEpochTime(validityPeriod.startDate, validityStart)) {
        MTR_LOG_ERROR("Failed in computing certificate validity start date");
        return CHIP_ERROR_INTERNAL;
    }

    if (!ToChipNotAfterEpochTime(validityPeriod.endDate, validityEnd)) {
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
    SecKeyRef intermediatePublicKey, NSNumber * _Nullable issuerId, NSNumber * _Nullable fabricId, NSDateInterval * validityPeriod,
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

    if (!ToChipEpochTime(validityPeriod.startDate, validityStart)) {
        MTR_LOG_ERROR("Failed in computing certificate validity start date");
        return CHIP_ERROR_INTERNAL;
    }

    if (!ToChipNotAfterEpochTime(validityPeriod.endDate, validityEnd)) {
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
    NSSet<NSNumber *> * _Nullable caseAuthenticatedTags, NSDateInterval * validityPeriod,
    NSData * _Nullable __autoreleasing * _Nonnull operationalCert)
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
        ReturnErrorOnFailure(SetToCATValues(caseAuthenticatedTags, cats));
    }

    uint8_t nocBuffer[Controller::kMaxCHIPDERCertLength];
    MutableByteSpan noc(nocBuffer);
    ReturnErrorOnFailure(GenerateNOC(keypairBridge, signingCertificate, node, fabric, cats, pubKey, validityPeriod, noc));

    *operationalCert = AsData(noc);
    return CHIP_NO_ERROR;
}
