/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#import "CastingServerBridge.h"
#import "CastingServer.h"

#import "CommissionableDataProviderImpl.hpp"
#import "CommissionerDiscoveryDelegateImpl.h"
#import "ConversionUtils.hpp"
#import "DeviceAttestationCredentialsProviderImpl.hpp"
#import "MatterCallbacks.h"
#import "OnboardingPayload.h"

#include <app/data-model/ListLargeSystemExtensions.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/support/CHIPMem.h>
#include <platform/PlatformManager.h>

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#endif

@interface CastingServerBridge ()

@property AppParameters * appParameters;

@property OnboardingPayload * _Nonnull onboardingPayload;

@property CommissionableDataProviderImpl * commissionableDataProvider;

@property chip::Credentials::DeviceAttestationCredentialsProvider * deviceAttestationCredentialsProvider;

@property chip::CommonCaseDeviceServerInitParams * serverInitParams;

@property CommissionerDiscoveryDelegateImpl * commissionerDiscoveryDelegate;

@property TargetVideoPlayerInfo * previouslyConnectedVideoPlayer;

// queue used to serialize all work performed by the CastingServerBridge
@property (atomic) dispatch_queue_t chipWorkQueue;

@property NSMutableDictionary * subscriptionEstablishedCallbacks;

@property NSMutableDictionary * subscriptionReadSuccessCallbacks;

@property NSMutableDictionary * subscriptionReadFailureCallbacks;

@property NSMutableDictionary * readSuccessCallbacks;

@property NSMutableDictionary * readFailureCallbacks;

@end

@implementation CastingServerBridge

+ (CastingServerBridge * _Nullable)getSharedInstance
{
    static CastingServerBridge * instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (instancetype)init
{
    if (self = [super init]) {
        CHIP_ERROR err = chip::Platform::MemoryInit();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "MemoryInit failed: %s", ErrorStr(err));
            return nil;
        }

        // TODO: Constructors should not perform heavy work like initializing the Matter SDK. This should
        // be moved to initializeApp or another suitable location.
        err = chip::DeviceLayer::PlatformMgr().InitChipStack();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "InitChipStack failed: %s", ErrorStr(err));
            return nil;
        }

        _commissionerDiscoveryDelegate = new CommissionerDiscoveryDelegateImpl();

        _subscriptionEstablishedCallbacks = [NSMutableDictionary dictionary];
        _subscriptionReadSuccessCallbacks = [NSMutableDictionary dictionary];
        _subscriptionReadFailureCallbacks = [NSMutableDictionary dictionary];
        _readSuccessCallbacks = [NSMutableDictionary dictionary];
        _readFailureCallbacks = [NSMutableDictionary dictionary];
    }
    return self;
}

/**
 Dispatches the block on the Matter SDK dispatch queue.

 @param description Optional. A description of the callback to be dispatched for diagnostic purposes.
 @param block The Block to be dispatched.
 */
- (void)dispatchOnMatterSDKQueue:(const NSString * _Nullable)description block:(dispatch_block_t)block
{
    if (nil != description) {
        ChipLogProgress(AppServer, "[SYNC] CastingServerBridge %s", [description UTF8String]);
    }
    dispatch_async(_chipWorkQueue, ^{
        // The Matter SDK is not, generally, internally thread-safe. It relies upon the client to lock
        // the SDK prior to any interactions after chip::DeviceLayer::PlatformMgr().InitChipStack()
        //
        // Note that it is presently safe to do at this point because InitChipStack is called in the
        // constructor for the CastingServerBridge.
        chip::DeviceLayer::StackLock lock;

        if (nil != description) {
            ChipLogProgress(AppServer, "[ASYNC (CHIP)] CastingServerBridge BEGIN %s", [description UTF8String]);
        }
        block();
        if (nil != description) {
            ChipLogProgress(AppServer, "[ASYNC (CHIP)] CastingServerBridge END %s", [description UTF8String]);
        }
    });
}

/**
 Dispatches a client callback via a `dispatch_block_t` to be run on the client's `dispatch_queue_t`.

 @param queue The client's Dispatch Queue.
 @param description Optional. A description of the callback to be dispatched for diagnostic purposes.
 @param block The Block to be invoked to invoke the client callback.
 */
- (void)dispatchOnClientQueue:(dispatch_queue_t)queue
                  description:(const NSString * _Nullable)description
                        block:(dispatch_block_t)block
{
    // Within the CastingServerBridge, the usage pattern is typically to expose asynchronous public APIs that
    // take a callback to indicate that the low-level SDK operation has been initiated (the "started"
    // callback), and a separate set of result callbacks to be invoked when the low-level SDK operation
    // has been completed (the "result" callbacks).
    //
    // All of the work is serialized on the _chipWorkQueue, with the started callback being invoked after the
    // Matter SDK is called with the result callbacks. This poses a challenge because we don't have a strong
    // guarantee that the Matter SDK will not complete its work and invoke the result callback before control
    // is returning to the _chipWorkQueue thread; this can cause callbacks to be received in an unexpected
    // order, where the result might arrive before the client knows that the call was started.
    //
    // To avoid this confusion, we perform a "double async" dispatch to ensure that events are delivered in a
    // sensible order. First, we schedule the result callback to run on the _chipWorkQueue (so it *must* run
    // after the end of the CastingServerBridge's wrapper logic that already started on the _chipWorkQueue)
    // and then immediately, asynchronously dispatch the callback onto the provided client queue.
    dispatch_async(_chipWorkQueue, ^{
        if (nil != description) {
            ChipLogProgress(AppServer, "[ASYNC (CHIP)] CastingServerBridge dispatching %s", [description UTF8String]);
        }
        dispatch_async(queue, ^{
            if (nil != description) {
                ChipLogProgress(AppServer, "[ASYNC (CLIENT)] CastingServerBridge BEGIN %s", [description UTF8String]);
            }
            block();
            if (nil != description) {
                ChipLogProgress(AppServer, "[ASYNC (CLIENT)] CastingServerBridge END %s", [description UTF8String]);
            }
        });
    });
}

/**
 A utility function that invokes an Objective C Block on the Matter Dispatch Queue in a thread-safe manner.

 The result of the Objective C Block will be relayed to the `blockCompleteCallback`.

 @param description  Optional. A description of the Objective C Block to be invoked for diagnostic purposes.
 @param callbackQueue The Dispatch Queue on which the callback shall be invoked.
 @param blockCompleteCallback The callback to be invoked with the result of the Objective C Block.
 @param block The Objective C Block to dispatch.
 */
- (void)withCastingServerInvokeBlock:(const NSString * _Nullable)description
                       callbackQueue:(dispatch_queue_t)callbackQueue
                     onBlockComplete:(void (^_Nonnull)(bool))blockCompleteCallback
                               block:(CHIP_ERROR (^_Nonnull)(CastingServer *))block
{
    [self dispatchOnMatterSDKQueue:description
                             block:^{
                                 CastingServer * castingServer = CastingServer::GetInstance();

                                 // We invoke the block and capture the result on the Matter Dispatch Queue with the stack lock
                                 // held, then use the result snapshot in the subsequent block to be asynchronously dispatched on
                                 // the client's callback Dispatch Queue.
                                 const CHIP_ERROR result = block(castingServer);
                                 dispatch_async(callbackQueue, ^{
                                     if (nil != description) {
                                         ChipLogProgress(AppServer,
                                             "[ASYNC (CLIENT)] CastingServerBridge invoking %s Started Callback",
                                             [description UTF8String]);
                                     }
                                     blockCompleteCallback(
                                         [[MatterError alloc] initWithCode:result.AsInteger()
                                                                   message:[NSString stringWithUTF8String:result.AsString()]]);
                                     if (nil != description) {
                                         ChipLogProgress(AppServer,
                                             "[ASYNC (CLIENT)] CastingServerBridge invoked %s Started Callback",
                                             [description UTF8String]);
                                     }
                                 });
                             }];
}

/**
 A utility function that invokes an Objective C Block on the Matter Dispatch Queue in a thread-safe manner.

 The result of the Objective C Block will be relayed to the `blockCompleteCallback`. The response callback is expected to be called
 at a later time with the asynchronous result of the Casting Server operation (by the Casting Server).

 @param description  Optional. A description of the Objective C Block to be invoked for diagnostic purposes.
 @param blockCompleteCallback The callback to be invoked with the result of the Objective C Block.
 @param responseCallback The callback that will eventually be invoked by the Casting Server when the operation has completed on the
 remote end.
 @param callbackQueue The Dispatch Queue on which the callbacks shall be invoked.
 @param block The Objective C Block to dispatch.
 */
- (void)withCastingServerInvokeBlock:(const NSString * _Nullable)description
                       callbackQueue:(dispatch_queue_t)callbackQueue
                     onBlockComplete:(void (^_Nonnull)(bool))blockCompleteCallback
                          onResponse:(void (^_Nonnull)(bool))responseCallback
                               block:(CHIP_ERROR (^_Nonnull)(CastingServer *, std::function<void(CHIP_ERROR)>))block
{
    [self withCastingServerInvokeBlock:description
                         callbackQueue:callbackQueue
                       onBlockComplete:blockCompleteCallback
                                 block:^(CastingServer * castingServer) {
                                     return block(castingServer, [description, responseCallback, callbackQueue](CHIP_ERROR err) {
                                         NSString * _description = description == nil
                                             ? nil
                                             : [NSString stringWithFormat:@"%@ Response Callback", description];

                                         [[CastingServerBridge getSharedInstance]
                                             dispatchOnClientQueue:callbackQueue
                                                       description:_description
                                                             block:^{
                                                                 responseCallback(CHIP_NO_ERROR == err);
                                                             }];
                                     });
                                 }];
}

- (MatterError *)initializeApp:(AppParameters * _Nullable)appParameters
                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
          initAppStatusHandler:(nullable void (^)(bool))initAppStatusHandler
{
    // The Matter SDK is not, generally, internally thread-safe. It relies upon the client to lock
    // the SDK prior to any interactions after chip::DeviceLayer::PlatformMgr().InitChipStack()
    //
    // Note that it is presently safe to do at this point because InitChipStack is called in the
    // constructor for the CastingServerBridge.
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingServerBridge().initApp() called");

    CHIP_ERROR err = CHIP_NO_ERROR;
    _commissionableDataProvider = new CommissionableDataProviderImpl();

    _appParameters = appParameters;
    AppParams cppAppParams;
    uint32_t setupPasscode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
    uint16_t setupDiscriminator = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
    uint32_t spake2pIterationCount;
    chip::ByteSpan spake2pSaltSpan, spake2pVerifierSpan;
    if (_appParameters != nil) {
        err = [ConversionUtils convertToCppAppParamsInfoFrom:_appParameters outAppParams:cppAppParams];
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "AppParameters conversion failed: %s", ErrorStr(err));
            return [[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]];
        }

        // set fields in commissionableDataProvider
        if (_appParameters.onboardingPayload != nil) {
            setupPasscode = _appParameters.onboardingPayload.setupPasscode > 0 ? _appParameters.onboardingPayload.setupPasscode
                                                                               : CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
            setupDiscriminator = _appParameters.onboardingPayload.setupDiscriminator > 0
                ? _appParameters.onboardingPayload.setupDiscriminator
                : CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;
        }
        spake2pIterationCount = _appParameters.spake2pIterationCount;
        if (_appParameters.spake2pSaltBase64 != nil) {
            spake2pSaltSpan = chip::ByteSpan(
                static_cast<const uint8_t *>(_appParameters.spake2pSaltBase64.bytes), _appParameters.spake2pSaltBase64.length);
        }

        if (_appParameters.spake2pVerifierBase64 != nil) {
            chip::ByteSpan spake2pVerifierSpan
                = chip::ByteSpan(static_cast<const uint8_t *>(_appParameters.spake2pVerifierBase64.bytes),
                    _appParameters.spake2pVerifierBase64.length);
        }

        err = _commissionableDataProvider->Initialize(_appParameters.spake2pVerifierBase64 != nil ? &spake2pVerifierSpan : nil,
            _appParameters.spake2pSaltBase64 != nil ? &spake2pSaltSpan : nil, spake2pIterationCount, setupPasscode,
            setupDiscriminator);
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "Failed to initialize CommissionableDataProvider: %s", ErrorStr(err));
            return [[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]];
        }
    }
    chip::DeviceLayer::SetCommissionableDataProvider(_commissionableDataProvider);

    _commissionableDataProvider->GetSetupPasscode(setupPasscode);
    _commissionableDataProvider->GetSetupDiscriminator(setupDiscriminator);
    _onboardingPayload = [[OnboardingPayload alloc] initWithSetupPasscode:setupPasscode setupDiscriminator:setupDiscriminator];

    // Initialize device attestation verifier from a constant version
    {
        // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
        const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
        SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore));
    }

    // init app Server
    _serverInitParams = new chip::CommonCaseDeviceServerInitParams();
    err = _serverInitParams->InitializeStaticResourcesBeforeServerInit();
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "InitializeStaticResourcesBeforeServerInit failed: %s", ErrorStr(err));
        return [[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]];
    }

    AppParams appParam;
    if (appParameters == nil) {
        err = CastingServer::GetInstance()->PreInit();
    } else if ((err = [ConversionUtils convertToCppAppParamsInfoFrom:appParameters outAppParams:appParam]) == CHIP_NO_ERROR) {
        err = CastingServer::GetInstance()->PreInit(&appParam);
    }
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "CastingServer PreInit failed: %s", ErrorStr(err));
        return [[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]];
    }

    err = chip::Server::GetInstance().Init(*_serverInitParams);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "chip::Server init failed: %s", ErrorStr(err));
        return [[MatterError alloc] initWithCode:err.AsInteger() message:[NSString stringWithUTF8String:err.AsString()]];
    }

    _chipWorkQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();

    chip::DeviceLayer::PlatformMgrImpl().StartEventLoopTask();

    dispatch_async(_chipWorkQueue, ^{
        CHIP_ERROR err = CHIP_NO_ERROR;
        AppParams appParam;
        if (appParameters == nil) {
            err = CastingServer::GetInstance()->Init();
        } else if ((err = [ConversionUtils convertToCppAppParamsInfoFrom:appParameters outAppParams:appParam]) == CHIP_NO_ERROR) {
            err = CastingServer::GetInstance()->Init(&appParam);
        }

        Boolean initAppStatus = true;
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "CastingServerBridge().initApp() failed: %" CHIP_ERROR_FORMAT, err.Format());
            initAppStatus = false;
        }

        dispatch_async(clientQueue, ^{
            initAppStatusHandler(initAppStatus);
        });
    });

    return [[MatterError alloc] initWithCode:CHIP_NO_ERROR.AsInteger()
                                     message:[NSString stringWithUTF8String:CHIP_NO_ERROR.AsString()]];
}

- (void)setDacHolder:(DeviceAttestationCredentialsHolder * _Nonnull)deviceAttestationCredentials
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    setDacHolderStatus:(void (^_Nonnull)(MatterError * _Nonnull))setDacHolderStatus
{
    dispatch_sync(_chipWorkQueue, ^{
        NSData * certificationDeclarationNsData = deviceAttestationCredentials.getCertificationDeclaration;
        chip::MutableByteSpan certificationDeclaration
            = chip::MutableByteSpan(const_cast<uint8_t *>(static_cast<const uint8_t *>(certificationDeclarationNsData.bytes)),
                certificationDeclarationNsData.length);

        NSData * firmwareInformationNsData = deviceAttestationCredentials.getFirmwareInformation;
        chip::MutableByteSpan firmwareInformation = chip::MutableByteSpan(
            const_cast<uint8_t *>(static_cast<const uint8_t *>(firmwareInformationNsData.bytes)), firmwareInformationNsData.length);

        NSData * deviceAttestationCertNsData = deviceAttestationCredentials.getDeviceAttestationCert;
        chip::MutableByteSpan deviceAttestationCert
            = chip::MutableByteSpan(const_cast<uint8_t *>(static_cast<const uint8_t *>(deviceAttestationCertNsData.bytes)),
                deviceAttestationCertNsData.length);

        NSData * productAttestationIntermediateCertNsData = deviceAttestationCredentials.getProductAttestationIntermediateCert;
        chip::MutableByteSpan productAttestationIntermediateCert = chip::MutableByteSpan(
            const_cast<uint8_t *>(static_cast<const uint8_t *>(productAttestationIntermediateCertNsData.bytes)),
            productAttestationIntermediateCertNsData.length);

        self->_deviceAttestationCredentialsProvider
            = new DeviceAttestationCredentialsProviderImpl(&certificationDeclaration, &firmwareInformation, &deviceAttestationCert,
                &productAttestationIntermediateCert, deviceAttestationCredentials.getDeviceAttestationCertPrivateKeyRef);

        SetDeviceAttestationCredentialsProvider(self->_deviceAttestationCredentialsProvider);

        dispatch_async(clientQueue, ^{
            setDacHolderStatus([[MatterError alloc] initWithCode:CHIP_NO_ERROR.AsInteger()
                                                         message:[NSString stringWithUTF8String:CHIP_NO_ERROR.AsString()]]);
        });
    });
}

- (void)discoverCommissioners:(dispatch_queue_t _Nonnull)clientQueue
      discoveryRequestSentHandler:(nullable void (^)(bool))discoveryRequestSentHandler
    discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData *))discoveredCommissionerHandler
{
    [self dispatchOnMatterSDKQueue:@"discoverCommissioners(...)"
                             block:^{
                                 bool discoveryRequestStatus = true;

                                 if (discoveredCommissionerHandler != nil) {
                                     TargetVideoPlayerInfo * cachedTargetVideoPlayerInfos
                                         = CastingServer::GetInstance()->ReadCachedTargetVideoPlayerInfos();
                                     self->_commissionerDiscoveryDelegate->SetUp(
                                         clientQueue, discoveredCommissionerHandler, cachedTargetVideoPlayerInfos);
                                 }
                                 CHIP_ERROR err = CastingServer::GetInstance()->DiscoverCommissioners(
                                     discoveredCommissionerHandler != nil ? self->_commissionerDiscoveryDelegate : nullptr);
                                 if (err != CHIP_NO_ERROR) {
                                     ChipLogError(AppServer,
                                         "CastingServerBridge().discoverCommissioners() failed: %" CHIP_ERROR_FORMAT, err.Format());
                                     discoveryRequestStatus = false;
                                 }

                                 dispatch_async(clientQueue, ^{
                                     discoveryRequestSentHandler(discoveryRequestStatus);
                                 });
                             }];
}

- (void)getDiscoveredCommissioner:(int)index
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData * _Nullable))discoveredCommissionerHandler
{
    [self dispatchOnMatterSDKQueue:@"getDiscoveredCommissioner(...)"
                             block:^{
                                 chip::Optional<TargetVideoPlayerInfo *> associatedConnectableVideoPlayer;
                                 DiscoveredNodeData * commissioner = nil;
                                 const chip::Dnssd::DiscoveredNodeData * cppDiscoveredNodeData
                                     = CastingServer::GetInstance()->GetDiscoveredCommissioner(
                                         index, associatedConnectableVideoPlayer);
                                 if (cppDiscoveredNodeData != nullptr) {
                                     commissioner = [ConversionUtils convertToObjCDiscoveredNodeDataFrom:cppDiscoveredNodeData];
                                     if (associatedConnectableVideoPlayer.HasValue()) {
                                         VideoPlayer * connectableVideoPlayer = [ConversionUtils
                                             convertToObjCVideoPlayerFrom:associatedConnectableVideoPlayer.Value()];
                                         [commissioner setConnectableVideoPlayer:connectableVideoPlayer];
                                     }
                                 }

                                 dispatch_async(clientQueue, ^{
                                     discoveredCommissionerHandler(commissioner);
                                 });
                             }];
}

- (void)sendUserDirectedCommissioningRequest:(NSString * _Nonnull)commissionerIpAddress
                            commissionerPort:(uint16_t)commissionerPort
                           platformInterface:(unsigned int)platformInterface
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(bool))udcRequestSentHandler
{
    const NSString * description =
        [NSString stringWithFormat:@"sendUserDirectedCommissioningRequest() IP %s port %d platformInterface %d",
                  [commissionerIpAddress UTF8String], commissionerPort, platformInterface];
    [self dispatchOnMatterSDKQueue:description
                             block:^{
                                 bool udcRequestStatus;
                                 chip::Inet::IPAddress commissionerAddrInet;
                                 if (chip::Inet::IPAddress::FromString([commissionerIpAddress UTF8String], commissionerAddrInet)
                                     == false) {
                                     ChipLogError(AppServer,
                                         "CastingServerBridge().sendUserDirectedCommissioningRequest() failed to parse IP address");
                                     udcRequestStatus = false;
                                 } else {
                                     chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId(platformInterface);

                                     chip::Transport::PeerAddress commissionerPeerAddress
                                         = chip::Transport::PeerAddress::UDP(commissionerAddrInet, commissionerPort, interfaceId);

                                     CHIP_ERROR err = CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(
                                         commissionerPeerAddress);
                                     if (err != CHIP_NO_ERROR) {
                                         ChipLogError(AppServer,
                                             "CastingServerBridge().sendUserDirectedCommissioningRequest() failed: "
                                             "%" CHIP_ERROR_FORMAT,
                                             err.Format());
                                         udcRequestStatus = false;
                                     } else {
                                         udcRequestStatus = true;
                                     }
                                 }

                                 dispatch_async(clientQueue, ^{
                                     udcRequestSentHandler(udcRequestStatus);
                                 });
                             }];
}

- (void)sendUserDirectedCommissioningRequest:(DiscoveredNodeData * _Nonnull)commissioner
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(bool))udcRequestSentHandler
{
    const NSString * description =
        [NSString stringWithFormat:@"sendUserDirectedCommissioningRequest(...) IP %s port %d platformInterface %d deviceName %s",
                  [commissioner.ipAddresses[0] UTF8String], commissioner.port, commissioner.platformInterface,
                  [commissioner.deviceName UTF8String]];

    [self dispatchOnMatterSDKQueue:description
                             block:^{
                                 bool udcRequestStatus;

                                 chip::Dnssd::DiscoveredNodeData cppCommissioner;
                                 if ([ConversionUtils convertToCppDiscoveredNodeDataFrom:commissioner
                                                                   outDiscoveredNodeData:cppCommissioner]
                                     != CHIP_NO_ERROR) {
                                     ChipLogError(AppServer,
                                         "CastingServerBridge().sendUserDirectedCommissioningRequest() failed to convert "
                                         "Commissioner(DiscoveredNodeData) "
                                         "to Cpp type");
                                     udcRequestStatus = false;
                                 } else {
                                     CHIP_ERROR err
                                         = CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(&cppCommissioner);
                                     if (err != CHIP_NO_ERROR) {
                                         ChipLogError(AppServer,
                                             "CastingServerBridge().sendUserDirectedCommissioningRequest() failed: "
                                             "%" CHIP_ERROR_FORMAT,
                                             err.Format());
                                         udcRequestStatus = false;
                                     } else {
                                         udcRequestStatus = true;
                                     }
                                 }

                                 dispatch_async(clientQueue, ^{
                                     udcRequestSentHandler(udcRequestStatus);
                                 });
                             }];
}

- (OnboardingPayload *)getOnboardingPayload
{
    return _onboardingPayload;
}

- (void)openBasicCommissioningWindow:(dispatch_queue_t _Nonnull)clientQueue
       commissioningCallbackHandlers:(CommissioningCallbackHandlers * _Nullable)commissioningCallbackHandlers
         onConnectionSuccessCallback:(void (^_Nonnull)(VideoPlayer * _Nonnull))onConnectionSuccessCallback
         onConnectionFailureCallback:(void (^_Nonnull)(MatterError * _Nonnull))onConnectionFailureCallback
      onNewOrUpdatedEndpointCallback:(void (^_Nonnull)(ContentApp * _Nonnull))onNewOrUpdatedEndpointCallback
{
    [self
        dispatchOnMatterSDKQueue:@"openBasicCommissioningWindow(...)"
                           block:^{
                               CommissioningCallbacks commissioningCallbacks;
                               if (commissioningCallbackHandlers != nil) {
                                   if (commissioningCallbackHandlers.commissioningCompleteCallback != nil) {
                                       commissioningCallbacks.commissioningComplete = [clientQueue, commissioningCallbackHandlers](
                                                                                          CHIP_ERROR err) {
                                           [[CastingServerBridge getSharedInstance]
                                               dispatchOnClientQueue:clientQueue
                                                         description:
                                                             @"openBasicCommissioningWindow(...) commissioningCompleteCallback"
                                                               block:^{
                                                                   commissioningCallbackHandlers.commissioningCompleteCallback(
                                                                       CHIP_NO_ERROR == err);
                                                               }];
                                       };
                                   }

                                   if (commissioningCallbackHandlers.sessionEstablishmentStartedCallback != nil) {
                                       commissioningCallbacks.sessionEstablishmentStarted
                                           = [clientQueue, commissioningCallbackHandlers]() {
                                                 [[CastingServerBridge getSharedInstance]
                                                     dispatchOnClientQueue:clientQueue
                                                               description:@"openBasicCommissioningWindow(...) "
                                                                           @"sessionEstablishmentStartedCallback"
                                                                     block:^{
                                                                         commissioningCallbackHandlers
                                                                             .sessionEstablishmentStartedCallback();
                                                                     }];
                                             };
                                   }

                                   if (commissioningCallbackHandlers.sessionEstablishedCallback != nil) {
                                       commissioningCallbacks.sessionEstablished = [clientQueue, commissioningCallbackHandlers]() {
                                           [[CastingServerBridge getSharedInstance]
                                               dispatchOnClientQueue:clientQueue
                                                         description:@"openBasicCommissioningWindow(...) sessionEstablishedCallback"
                                                               block:^{
                                                                   commissioningCallbackHandlers.sessionEstablishedCallback();
                                                               }];
                                       };
                                   }

                                   if (commissioningCallbackHandlers.sessionEstablishmentErrorCallback != nil) {
                                       commissioningCallbacks.sessionEstablishmentError = [clientQueue,
                                                                                              commissioningCallbackHandlers](
                                                                                              CHIP_ERROR err) {
                                           [[CastingServerBridge getSharedInstance]
                                               dispatchOnClientQueue:clientQueue
                                                         description:@"openBasicCommissioningWindow(...) "
                                                                     @"sessionEstablishmentErrorCallback"
                                                               block:^{
                                                                   commissioningCallbackHandlers.sessionEstablishmentErrorCallback(
                                                                       [[MatterError alloc]
                                                                           initWithCode:err.AsInteger()
                                                                                message:[NSString
                                                                                            stringWithUTF8String:err.AsString()]]);
                                                               }];
                                       };
                                   }

                                   if (commissioningCallbackHandlers.sessionEstablishmentStoppedCallback != nil) {
                                       commissioningCallbacks.sessionEstablishmentStopped
                                           = [clientQueue, commissioningCallbackHandlers]() {
                                                 [[CastingServerBridge getSharedInstance]
                                                     dispatchOnClientQueue:clientQueue
                                                               description:@"openBasicCommissioningWindow(...) "
                                                                           @"sessionEstablishmentStoppedCallback"
                                                                     block:^{
                                                                         commissioningCallbackHandlers
                                                                             .sessionEstablishmentStoppedCallback();
                                                                     }];
                                             };
                                   }
                               }
                               CHIP_ERROR err = CastingServer::GetInstance()->OpenBasicCommissioningWindow(
                                   commissioningCallbacks,
                                   [clientQueue, onConnectionSuccessCallback](TargetVideoPlayerInfo * cppTargetVideoPlayerInfo) {
                                       VideoPlayer * videoPlayer =
                                           [ConversionUtils convertToObjCVideoPlayerFrom:cppTargetVideoPlayerInfo];
                                       [[CastingServerBridge getSharedInstance]
                                           dispatchOnClientQueue:clientQueue
                                                     description:@"openBasicCommissioningWindow(...) onConnectionSuccessCallback"
                                                           block:^{
                                                               onConnectionSuccessCallback(videoPlayer);
                                                           }];
                                   },
                                   [clientQueue, onConnectionFailureCallback](CHIP_ERROR err) {
                                       [[CastingServerBridge getSharedInstance]
                                           dispatchOnClientQueue:clientQueue
                                                     description:@"openBasicCommissioningWindow(...) onConnectionFailureCallback"
                                                           block:^{
                                                               onConnectionFailureCallback([[MatterError alloc]
                                                                   initWithCode:err.AsInteger()
                                                                        message:[NSString stringWithUTF8String:err.AsString()]]);
                                                           }];
                                   },
                                   [clientQueue, onNewOrUpdatedEndpointCallback](TargetEndpointInfo * cppTargetEndpointInfo) {
                                       ContentApp * contentApp =
                                           [ConversionUtils convertToObjCContentAppFrom:cppTargetEndpointInfo];
                                       [[CastingServerBridge getSharedInstance]
                                           dispatchOnClientQueue:clientQueue
                                                     description:@"openBasicCommissioningWindow(...) onNewOrUpdatedEndpointCallback"
                                                           block:^{
                                                               onNewOrUpdatedEndpointCallback(contentApp);
                                                           }];
                                   });

                               if (commissioningCallbackHandlers != nil
                                   && commissioningCallbackHandlers.commissioningWindowRequestedHandler != nil) {
                                   dispatch_async(clientQueue, ^{
                                       ChipLogProgress(AppServer, "[async] Dispatching commissioningWindowRequestedHandler");
                                       commissioningCallbackHandlers.commissioningWindowRequestedHandler(CHIP_NO_ERROR == err);
                                   });
                               }
                           }];
}

- (void)getActiveTargetVideoPlayers:(dispatch_queue_t _Nonnull)clientQueue
    activeTargetVideoPlayersHandler:(nullable void (^)(NSMutableArray * _Nullable))activeTargetVideoPlayersHandler
{
    [self dispatchOnMatterSDKQueue:@"getActiveTargetVideoPlayers(...)"
                             block:^{
                                 NSMutableArray * videoPlayers = [NSMutableArray new];
                                 TargetVideoPlayerInfo * cppTargetVideoPlayerInfo
                                     = CastingServer::GetInstance()->GetActiveTargetVideoPlayer();
                                 if (cppTargetVideoPlayerInfo != nullptr && cppTargetVideoPlayerInfo->IsInitialized()) {
                                     videoPlayers[0] = [ConversionUtils convertToObjCVideoPlayerFrom:cppTargetVideoPlayerInfo];
                                 }

                                 dispatch_async(clientQueue, ^{
                                     activeTargetVideoPlayersHandler(videoPlayers);
                                 });
                             }];
}

- (void)readCachedVideoPlayers:(dispatch_queue_t _Nonnull)clientQueue
    readCachedVideoPlayersHandler:(nullable void (^)(NSMutableArray * _Nullable))readCachedVideoPlayersHandler
{
    [self dispatchOnMatterSDKQueue:@"readCachedVideoPlayers(...)"
                             block:^{
                                 NSMutableArray * videoPlayers = nil;
                                 TargetVideoPlayerInfo * cppTargetVideoPlayerInfos
                                     = CastingServer::GetInstance()->ReadCachedTargetVideoPlayerInfos();
                                 if (cppTargetVideoPlayerInfos != nullptr) {
                                     videoPlayers = [NSMutableArray new];
                                     for (size_t i = 0; cppTargetVideoPlayerInfos[i].IsInitialized(); i++) {
                                         ChipLogProgress(AppServer,
                                             "CastingServerBridge().readCachedVideoPlayers() with nodeId: 0x" ChipLogFormatX64
                                             " fabricIndex: %d deviceName: %s vendorId: %d",
                                             ChipLogValueX64(cppTargetVideoPlayerInfos[i].GetNodeId()),
                                             cppTargetVideoPlayerInfos[i].GetFabricIndex(),
                                             cppTargetVideoPlayerInfos[i].GetDeviceName(),
                                             cppTargetVideoPlayerInfos[i].GetVendorId());
                                         videoPlayers[i] =
                                             [ConversionUtils convertToObjCVideoPlayerFrom:&cppTargetVideoPlayerInfos[i]];
                                     }
                                 }

                                 dispatch_async(clientQueue, ^{
                                     readCachedVideoPlayersHandler(videoPlayers);
                                 });
                             }];
}

- (void)verifyOrEstablishConnection:(VideoPlayer * _Nonnull)videoPlayer
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(nullable void (^)(MatterError * _Nonnull))requestSentHandler
        onConnectionSuccessCallback:(void (^_Nonnull)(VideoPlayer * _Nonnull))onConnectionSuccessCallback
        onConnectionFailureCallback:(void (^_Nonnull)(MatterError * _Nonnull))onConnectionFailureCallback
     onNewOrUpdatedEndpointCallback:(void (^_Nonnull)(ContentApp * _Nonnull))onNewOrUpdatedEndpointCallback
{
    [self dispatchOnMatterSDKQueue:@"verifyOrEstablishConnection(...)"
                             block:^{
                                 TargetVideoPlayerInfo targetVideoPlayerInfo;
                                 [ConversionUtils convertToCppTargetVideoPlayerInfoFrom:videoPlayer
                                                               outTargetVideoPlayerInfo:targetVideoPlayerInfo];

                                 CHIP_ERROR err = CastingServer::GetInstance()->VerifyOrEstablishConnection(
                                     targetVideoPlayerInfo,
                                     [clientQueue, onConnectionSuccessCallback](TargetVideoPlayerInfo * cppTargetVideoPlayerInfo) {
                                         VideoPlayer * videoPlayer =
                                             [ConversionUtils convertToObjCVideoPlayerFrom:cppTargetVideoPlayerInfo];
                                         [[CastingServerBridge getSharedInstance]
                                             dispatchOnClientQueue:clientQueue
                                                       description:@"onConnectionSuccessCallback"
                                                             block:^{
                                                                 onConnectionSuccessCallback(videoPlayer);
                                                             }];
                                     },
                                     [clientQueue, onConnectionFailureCallback](CHIP_ERROR err) {
                                         [[CastingServerBridge getSharedInstance]
                                             dispatchOnClientQueue:clientQueue
                                                       description:@"onConnectionFailureCallback"
                                                             block:^{
                                                                 onConnectionFailureCallback([[MatterError alloc]
                                                                     initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                                             }];
                                     },
                                     [clientQueue, onNewOrUpdatedEndpointCallback](TargetEndpointInfo * cppTargetEndpointInfo) {
                                         ContentApp * contentApp =
                                             [ConversionUtils convertToObjCContentAppFrom:cppTargetEndpointInfo];
                                         [[CastingServerBridge getSharedInstance]
                                             dispatchOnClientQueue:clientQueue
                                                       description:@"onNewOrUpdatedEndpointCallback"
                                                             block:^{
                                                                 onNewOrUpdatedEndpointCallback(contentApp);
                                                             }];
                                     });

                                 dispatch_async(clientQueue, ^{
                                     requestSentHandler(
                                         [[MatterError alloc] initWithCode:err.AsInteger()
                                                                   message:[NSString stringWithUTF8String:err.AsString()]]);
                                 });
                             }];
}

- (void)shutdownAllSubscriptions:(dispatch_queue_t _Nonnull)clientQueue requestSentHandler:(nullable void (^)())requestSentHandler
{
    [self dispatchOnMatterSDKQueue:@"shutdownAllSubscriptions(...)"
                             block:^{
                                 CastingServer::GetInstance()->ShutdownAllSubscriptions();
                                 dispatch_async(clientQueue, ^{
                                     requestSentHandler();
                                 });
                             }];
}

- (void)startMatterServer:(dispatch_queue_t _Nonnull)clientQueue
    startMatterServerCompletionCallback:(nullable void (^)(MatterError * _Nonnull))startMatterServerCompletionCallback
{
    [self dispatchOnMatterSDKQueue:@"startMatterServer(...)"
                             block:^{
                                 // Initialize the Matter server
                                 CHIP_ERROR err = chip::Server::GetInstance().Init(*self->_serverInitParams);
                                 if (err != CHIP_NO_ERROR) {
                                     ChipLogError(AppServer, "chip::Server init failed: %s", ErrorStr(err));
                                     dispatch_async(clientQueue, ^{
                                         startMatterServerCompletionCallback(
                                             [[MatterError alloc] initWithCode:err.AsInteger()
                                                                       message:[NSString stringWithUTF8String:err.AsString()]]);
                                     });
                                     return;
                                 }

                                 // Initialize AppDelegation
                                 CastingServer::GetInstance()->InitAppDelegation();

                                 // Initialize binding handlers
                                 err = CastingServer::GetInstance()->InitBindingHandlers();
                                 if (err != CHIP_NO_ERROR) {
                                     ChipLogError(AppServer, "Binding init failed: %s", ErrorStr(err));
                                     dispatch_async(clientQueue, ^{
                                         startMatterServerCompletionCallback(
                                             [[MatterError alloc] initWithCode:err.AsInteger()
                                                                       message:[NSString stringWithUTF8String:err.AsString()]]);
                                     });
                                     return;
                                 }

                                 // Now reconnect to the VideoPlayer the casting app was previously connected to (if any)
                                 if (self->_previouslyConnectedVideoPlayer != nil) {
                                     ChipLogProgress(AppServer,
                                         "CastingServerBridge().startMatterServer() reconnecting to previously connected "
                                         "VideoPlayer...");
                                     err = CastingServer::GetInstance()->VerifyOrEstablishConnection(
                                         *(self->_previouslyConnectedVideoPlayer),
                                         [clientQueue, startMatterServerCompletionCallback](
                                             TargetVideoPlayerInfo * cppTargetVideoPlayerInfo) {
                                             dispatch_async(clientQueue, ^{
                                                 startMatterServerCompletionCallback([[MatterError alloc]
                                                     initWithCode:CHIP_NO_ERROR.AsInteger()
                                                          message:[NSString stringWithUTF8String:CHIP_NO_ERROR.AsString()]]);
                                             });
                                         },
                                         [clientQueue, startMatterServerCompletionCallback](CHIP_ERROR err) {
                                             dispatch_async(clientQueue, ^{
                                                 startMatterServerCompletionCallback([[MatterError alloc]
                                                     initWithCode:err.AsInteger()
                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                             });
                                         },
                                         [](TargetEndpointInfo * cppTargetEndpointInfo) {});
                                 } else {
                                     dispatch_async(clientQueue, ^{
                                         startMatterServerCompletionCallback([[MatterError alloc]
                                             initWithCode:CHIP_NO_ERROR.AsInteger()
                                                  message:[NSString stringWithUTF8String:CHIP_NO_ERROR.AsString()]]);
                                     });
                                 }
                             }];
}

- (void)stopMatterServer
{
    [self
        dispatchOnMatterSDKQueue:@"stopMatterServer(...)"
                           block:^{
                               // capture pointer to previouslyConnectedVideoPlayer, to be deleted
                               TargetVideoPlayerInfo * videoPlayerForDeletion
                                   = self->_previouslyConnectedVideoPlayer == nil ? nil : self->_previouslyConnectedVideoPlayer;

                               // On shutting down the Matter server, the casting app will be automatically disconnected from any
                               // Video Players it was connected to. Save the VideoPlayer that the casting app was targetting and
                               // connected to, so we can reconnect to it on re-starting the Matter server.
                               TargetVideoPlayerInfo * currentTargetVideoPlayerInfo
                                   = CastingServer::GetInstance()->GetActiveTargetVideoPlayer();
                               if (currentTargetVideoPlayerInfo != nil && currentTargetVideoPlayerInfo->IsInitialized()
                                   && currentTargetVideoPlayerInfo->GetOperationalDeviceProxy() != nil) {
                                   self->_previouslyConnectedVideoPlayer = new TargetVideoPlayerInfo();
                                   self->_previouslyConnectedVideoPlayer->Initialize(currentTargetVideoPlayerInfo->GetNodeId(),
                                       currentTargetVideoPlayerInfo->GetFabricIndex(), nullptr, nullptr,
                                       currentTargetVideoPlayerInfo->GetVendorId(), currentTargetVideoPlayerInfo->GetProductId(),
                                       currentTargetVideoPlayerInfo->GetDeviceType(), currentTargetVideoPlayerInfo->GetDeviceName(),
                                       currentTargetVideoPlayerInfo->GetHostName(), currentTargetVideoPlayerInfo->GetNumIPs(),
                                       const_cast<chip::Inet::IPAddress *>(currentTargetVideoPlayerInfo->GetIpAddresses()));

                                   TargetEndpointInfo * prevEndpoints = self->_previouslyConnectedVideoPlayer->GetEndpoints();
                                   if (prevEndpoints != nullptr) {
                                       for (size_t i = 0; i < kMaxNumberOfEndpoints; i++) {
                                           prevEndpoints[i].Reset();
                                       }
                                   }
                                   TargetEndpointInfo * currentEndpoints = currentTargetVideoPlayerInfo->GetEndpoints();
                                   for (size_t i = 0; i < kMaxNumberOfEndpoints && currentEndpoints[i].IsInitialized(); i++) {
                                       prevEndpoints[i].Initialize(currentEndpoints[i].GetEndpointId());
                                       chip::ClusterId * currentClusters = currentEndpoints[i].GetClusters();
                                       for (size_t j = 0;
                                            j < kMaxNumberOfClustersPerEndpoint && currentClusters[j] != chip::kInvalidClusterId;
                                            j++) {
                                           prevEndpoints[i].AddCluster(currentClusters[j]);
                                       }
                                   }
                               } else {
                                   self->_previouslyConnectedVideoPlayer = nil;
                               }

                               // Now shutdown the Matter server
                               chip::Server::GetInstance().Shutdown();

                               // Delete the old previouslyConnectedVideoPlayer, if non-nil
                               if (videoPlayerForDeletion != nil) {
                                   delete videoPlayerForDeletion;
                               }
                           }];
}

- (void)disconnect:(dispatch_queue_t _Nonnull)clientQueue requestSentHandler:(nullable void (^)())requestSentHandler
{
    [self dispatchOnMatterSDKQueue:@"disconnect(...)"
                             block:^{
                                 CastingServer::GetInstance()->Disconnect();
                                 dispatch_async(clientQueue, ^{
                                     requestSentHandler();
                                 });
                             }];
}

- (void)purgeCache:(dispatch_queue_t _Nonnull)clientQueue responseHandler:(void (^)(MatterError * _Nonnull))responseHandler
{
    dispatch_sync(_chipWorkQueue, ^{
        CHIP_ERROR err = CastingServer::GetInstance()->PurgeCache();
        dispatch_async(clientQueue, ^{
            responseHandler([[MatterError alloc] initWithCode:err.AsInteger()
                                                      message:[NSString stringWithUTF8String:err.AsString()]]);
        });
    });
}

- (void)contentLauncher_launchUrl:(ContentApp * _Nonnull)contentApp
                       contentUrl:(NSString * _Nonnull)contentUrl
                contentDisplayStr:(NSString * _Nonnull)contentDisplayStr
                 responseCallback:(void (^_Nonnull)(bool))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];
                                     return castingServer->ContentLauncherLaunchURL(
                                         &endpoint, [contentUrl UTF8String], [contentDisplayStr UTF8String], responseFunction);
                                 }];
}

- (void)contentLauncher_launchContent:(ContentApp * _Nonnull)contentApp
                        contentSearch:(ContentLauncher_ContentSearch * _Nonnull)contentSearch
                             autoPlay:(bool)autoPlay
                                 data:(NSString * _Nullable)data
                     responseCallback:(void (^_Nonnull)(bool))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description =
        [NSString stringWithFormat:@"contentLauncher_launchContent(...) with Content App endpoint ID %d", contentApp.endpointId];

    // Make a copy of params before we go async.
    contentSearch = [contentSearch copy];
    data = [data copy];

    [self
        dispatchOnMatterSDKQueue:description
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               ListFreer listFreer;
                               chip::app::Clusters::ContentLauncher::Structs::ContentSearchStruct::Type cppSearch;
                               if (contentSearch.parameterList.count > 0) {
                                   auto * parameterListHolder
                                       = new ListHolder<chip::app::Clusters::ContentLauncher::Structs::ParameterStruct::Type>(
                                           contentSearch.parameterList.count);
                                   listFreer.add(parameterListHolder);

                                   int parameterIndex = 0;
                                   for (ContentLauncher_Parameter * parameter in contentSearch.parameterList) {
                                       int externalIdListIndex = 0;
                                       if (parameter.externalIDList != nil) {
                                           auto * externalIdListHolder = new ListHolder<
                                               chip::app::Clusters::ContentLauncher::Structs::AdditionalInfoStruct::Type>(
                                               parameter.externalIDList.count);
                                           listFreer.add(externalIdListHolder);

                                           for (ContentLauncher_AdditionalInfo * additionalInfo in parameter.externalIDList) {
                                               externalIdListHolder->mList[externalIdListIndex].value
                                                   = chip::CharSpan([additionalInfo.value UTF8String],
                                                       [additionalInfo.value lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
                                               externalIdListHolder->mList[externalIdListIndex].name
                                                   = chip::CharSpan([additionalInfo.name UTF8String],
                                                       [additionalInfo.name lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
                                               externalIdListIndex++;
                                           }
                                           parameterListHolder->mList[parameterIndex].externalIDList
                                               = MakeOptional(chip::app::DataModel::List<
                                                   const chip::app::Clusters::ContentLauncher::Structs::AdditionalInfoStruct::Type>(
                                                   externalIdListHolder->mList, parameter.externalIDList.count));
                                       } else {
                                           parameterListHolder->mList[parameterIndex].externalIDList
                                               = chip::Optional<chip::app::DataModel::List<const chip::app::Clusters::
                                                       ContentLauncher::Structs::AdditionalInfoStruct::Type>>::Missing();
                                       }

                                       parameterListHolder->mList[parameterIndex].type
                                           = static_cast<chip::app::Clusters::ContentLauncher::ParameterEnum>(parameter.type);
                                       parameterListHolder->mList[parameterIndex].value
                                           = chip::CharSpan([parameter.value UTF8String],
                                               [parameter.value lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
                                       parameterIndex++;
                                       cppSearch.parameterList = chip::app::DataModel::List<
                                           chip::app::Clusters::ContentLauncher::Structs::ParameterStruct::Type>(
                                           parameterListHolder->mList, contentSearch.parameterList.count);
                                   }
                               }

                               CHIP_ERROR err = CastingServer::GetInstance()->ContentLauncher_LaunchContent(&endpoint, cppSearch,
                                   autoPlay,
                                   MakeOptional(
                                       chip::CharSpan([data UTF8String], [data lengthOfBytesUsingEncoding:NSUTF8StringEncoding])),
                                   [clientQueue, responseCallback](CHIP_ERROR err) {
                                       [[CastingServerBridge getSharedInstance]
                                           dispatchOnClientQueue:clientQueue
                                                     description:@"contentLauncher_launchContent(...) responseCallback"
                                                           block:^{
                                                               responseCallback(CHIP_NO_ERROR == err);
                                                           }];
                                   });

                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(CHIP_NO_ERROR == err);
                               });
                           }];
}

- (void)contentLauncher_subscribeSupportedStreamingProtocols:(ContentApp * _Nonnull)contentApp
                                                 minInterval:(uint16_t)minInterval
                                                 maxInterval:(uint16_t)maxInterval
                                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                             successCallback:(void (^_Nonnull)(uint32_t))successCallback
                                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
                             subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    const NSString * description =
        [NSString stringWithFormat:@"contentLauncher_subscribeSupportedStreamingProtocols(...) with Content App endpoint ID %d",
                  contentApp.endpointId];
    [self dispatchOnMatterSDKQueue:description
                             block:^{
                                 TargetEndpointInfo endpoint;
                                 [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                 CHIP_ERROR err
                                     = CastingServer::GetInstance()->ContentLauncher_SubscribeToSupportedStreamingProtocols(
                                         &endpoint, nullptr,
                                         [](void * context,
                                             chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::
                                                 TypeInfo::DecodableArgType supportedStreamingProtocols) {
                                             void (^callback)(uint32_t) =
                                                 [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                                     objectForKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
                                             callback(supportedStreamingProtocols.Raw());
                                         },
                                         [](void * context, CHIP_ERROR err) {
                                             void (^callback)(MatterError *) =
                                                 [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                                     objectForKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
                                             callback(
                                                 [[MatterError alloc] initWithCode:err.AsInteger()
                                                                           message:[NSString stringWithUTF8String:err.AsString()]]);
                                         },
                                         minInterval, maxInterval,
                                         [](void * context, chip::SubscriptionId subscriptionId) {
                                             void (^callback)() =
                                                 [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                                     objectForKey:@"contentLauncher_subscribeSupportedStreamingProtocols"];
                                             callback();
                                         });
                                 dispatch_async(clientQueue, ^{
                                     requestSentHandler(
                                         [[MatterError alloc] initWithCode:err.AsInteger()
                                                                   message:[NSString stringWithUTF8String:err.AsString()]]);
                                 });
                             }];
}

- (void)levelControl_step:(ContentApp * _Nonnull)contentApp
                 stepMode:(uint8_t)stepMode
                 stepSize:(uint8_t)stepSize
           transitionTime:(uint16_t)transitionTime
               optionMask:(uint8_t)optionMask
           optionOverride:(uint8_t)optionOverride
         responseCallback:(void (^_Nonnull)(bool))responseCallback
              clientQueue:(dispatch_queue_t _Nonnull)clientQueue
       requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->LevelControl_Step(&endpoint,
                                         static_cast<chip::app::Clusters::LevelControl::StepModeEnum>(stepMode), stepSize,
                                         transitionTime, optionMask, optionOverride, responseFunction);
                                 }];
}

- (void)levelControl_moveToLevel:(ContentApp * _Nonnull)contentApp
                           level:(uint8_t)level
                  transitionTime:(uint16_t)transitionTime
                      optionMask:(uint8_t)optionMask
                  optionOverride:(uint8_t)optionOverride
                responseCallback:(void (^_Nonnull)(bool))responseCallback
                     clientQueue:(dispatch_queue_t _Nonnull)clientQueue
              requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description =
        [NSString stringWithFormat:@"levelControl_moveToLevel(...) with Content App endpoint ID %d", contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->LevelControl_MoveToLevel(
                                         &endpoint, level, transitionTime, optionMask, optionOverride, responseFunction);
                                 }];
}

- (void)levelControl_subscribeCurrentLevel:(ContentApp * _Nonnull)contentApp
                               minInterval:(uint16_t)minInterval
                               maxInterval:(uint16_t)maxInterval
                               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                           successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                           failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
           subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().levelControl_subscribeCurrentLevel() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"levelControl_subscribeCurrentLevel"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"levelControl_subscribeCurrentLevel"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"levelControl_subscribeCurrentLevel"];

    [self
        dispatchOnMatterSDKQueue:@"levelControl_subscribeCurrentLevel(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_SubscribeToCurrentLevel(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo::DecodableArgType
                                           currentLevel) {
                                       void (^callback)(NSNumber * _Nullable) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"levelControl_subscribeCurrentLevel"];
                                       callback(@(currentLevel.Value()));
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"levelControl_subscribeCurrentLevel"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"levelControl_subscribeCurrentLevel"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)levelControl_subscribeMinLevel:(ContentApp * _Nonnull)contentApp
                           minInterval:(uint16_t)minInterval
                           maxInterval:(uint16_t)maxInterval
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint8_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
       subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_subscribeMinLevel() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"levelControl_subscribeMinLevel"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"levelControl_subscribeMinLevel"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"levelControl_subscribeMinLevel"];

    [self
        dispatchOnMatterSDKQueue:@"levelControl_subscribeMinLevel(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_SubscribeToMinLevel(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo::DecodableArgType
                                           minLevel) {
                                       void (^callback)(uint8_t) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"levelControl_subscribeMinLevel"];
                                       callback(minLevel);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"levelControl_subscribeMinLevel"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"levelControl_subscribeMinLevel"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)levelControl_subscribeMaxLevel:(ContentApp * _Nonnull)contentApp
                           minInterval:(uint16_t)minInterval
                           maxInterval:(uint16_t)maxInterval
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint8_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
       subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().levelControl_subscribeMaxLevel() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"levelControl_subscribeMaxLevel"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"levelControl_subscribeMaxLevel"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"levelControl_subscribeMaxLevel"];

    [self
        dispatchOnMatterSDKQueue:@"levelControl_subscribeMaxLevel(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->LevelControl_SubscribeToMaxLevel(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo::DecodableArgType
                                           maxLevel) {
                                       void (^callback)(uint8_t) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"levelControl_subscribeMaxLevel"];
                                       callback(maxLevel);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"levelControl_subscribeMaxLevel"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"levelControl_subscribeMaxLevel"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)mediaPlayback_play:(ContentApp * _Nonnull)contentApp
          responseCallback:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_Play(&endpoint, responseFunction);
                                 }];
}

- (void)mediaPlayback_pause:(ContentApp * _Nonnull)contentApp
           responseCallback:(void (^_Nonnull)(bool))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_Pause(&endpoint, responseFunction);
                                 }];
}

- (void)mediaPlayback_stopPlayback:(ContentApp * _Nonnull)contentApp
                  responseCallback:(void (^_Nonnull)(bool))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_StopPlayback(&endpoint, responseFunction);
                                 }];
}

- (void)mediaPlayback_next:(ContentApp * _Nonnull)contentApp
          responseCallback:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_Next(&endpoint, responseFunction);
                                 }];
}

- (void)mediaPlayback_seek:(ContentApp * _Nonnull)contentApp
                  position:(uint64_t)position
          responseCallback:(void (^_Nonnull)(bool))responseCallback
               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
        requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_Seek(&endpoint, position, responseFunction);
                                 }];
}

- (void)mediaPlayback_skipForward:(ContentApp * _Nonnull)contentApp
        deltaPositionMilliseconds:(uint64_t)deltaPositionMilliseconds
                 responseCallback:(void (^_Nonnull)(bool))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_SkipForward(
                                         &endpoint, deltaPositionMilliseconds, responseFunction);
                                 }];
}

- (void)mediaPlayback_skipBackward:(ContentApp * _Nonnull)contentApp
         deltaPositionMilliseconds:(uint64_t)deltaPositionMilliseconds
                  responseCallback:(void (^_Nonnull)(bool))responseCallback
                       clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_SkipBackward(
                                         &endpoint, deltaPositionMilliseconds, responseFunction);
                                 }];
}

- (void)mediaPlayback_previous:(ContentApp * _Nonnull)contentApp
              responseCallback:(void (^_Nonnull)(bool))responseCallback
                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
            requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_Previous(&endpoint, responseFunction);
                                 }];
}

- (void)mediaPlayback_rewind:(ContentApp * _Nonnull)contentApp
            responseCallback:(void (^_Nonnull)(bool))responseCallback
                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
          requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_Rewind(&endpoint, responseFunction);
                                 }];
}

- (void)mediaPlayback_fastForward:(ContentApp * _Nonnull)contentApp
                 responseCallback:(void (^_Nonnull)(bool))responseCallback
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
               requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_FastForward(&endpoint, responseFunction);
                                 }];
}

- (void)mediaPlayback_startOver:(ContentApp * _Nonnull)contentApp
               responseCallback:(void (^_Nonnull)(bool))responseCallback
                    clientQueue:(dispatch_queue_t _Nonnull)clientQueue
             requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->MediaPlayback_StartOver(&endpoint, responseFunction);
                                 }];
}

- (void)mediaPlayback_subscribeCurrentState:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(MediaPlayback_PlaybackState))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribeCurrentState() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeCurrentState"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeCurrentState"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeCurrentState"];

    [self
        dispatchOnMatterSDKQueue:@"mediaPlayback_subscribeCurrentState(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToCurrentState(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType
                                           currentState) {
                                       void (^callback)(MediaPlayback_PlaybackState) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"mediaPlayback_subscribeCurrentState"];
                                       callback(MediaPlayback_PlaybackState(currentState));
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"mediaPlayback_subscribeCurrentState"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"mediaPlayback_subscribeCurrentState"];
                                       callback();
                                   });

                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)mediaPlayback_subscribeStartTime:(ContentApp * _Nonnull)contentApp
                             minInterval:(uint16_t)minInterval
                             maxInterval:(uint16_t)maxInterval
                             clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                      requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                         successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                         failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
         subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeStartTime() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeStartTime"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeStartTime"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeStartTime"];

    [self
        dispatchOnMatterSDKQueue:@"mediaPlayback_subscribeStartTime(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToStartTime(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::MediaPlayback::Attributes::StartTime::TypeInfo::DecodableArgType
                                           startTime) {
                                       void (^callback)(NSNumber * _Nullable) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"mediaPlayback_subscribeStartTime"];
                                       callback(@(startTime.Value()));
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"mediaPlayback_subscribeStartTime"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"mediaPlayback_subscribeStartTime"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)mediaPlayback_subscribeDuration:(ContentApp * _Nonnull)contentApp
                            minInterval:(uint16_t)minInterval
                            maxInterval:(uint16_t)maxInterval
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                     requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                        successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                        failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
        subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().mediaPlayback_subscribeDuration() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeDuration"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeDuration"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeDuration"];

    [self
        dispatchOnMatterSDKQueue:@"mediaPlayback_subscribeDuration(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo::DecodableArgType
                                           startTime) {
                                       void (^callback)(NSNumber * _Nullable) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"mediaPlayback_subscribeDuration"];
                                       callback(@(startTime.Value()));
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"mediaPlayback_subscribeDuration"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"mediaPlayback_subscribeDuration"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)mediaPlayback_subscribeSampledPosition:(ContentApp * _Nonnull)contentApp
                                   minInterval:(uint16_t)minInterval
                                   maxInterval:(uint16_t)maxInterval
                                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                            requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                               successCallback:(void (^_Nonnull)(MediaPlayback_PlaybackPosition * _Nullable))successCallback
                               failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
               subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribeSampledPosition() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeSampledPosition"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeSampledPosition"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeSampledPosition"];

    [self
        dispatchOnMatterSDKQueue:@"mediaPlayback_subscribeSampledPosition(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToSampledPosition(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo::DecodableArgType
                                           playbackPosition) {
                                       void (^callback)(MediaPlayback_PlaybackPosition * _Nullable) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"mediaPlayback_subscribeSampledPosition"];
                                       MediaPlayback_PlaybackPosition * objCPlaybackPosition = nil;
                                       if (!playbackPosition.IsNull()) {
                                           if (playbackPosition.Value().position.IsNull()) {
                                               objCPlaybackPosition = [[MediaPlayback_PlaybackPosition alloc]
                                                   initWithUpdatedAt:@(playbackPosition.Value().updatedAt)
                                                            position:nil];
                                           } else {
                                               objCPlaybackPosition = [[MediaPlayback_PlaybackPosition alloc]
                                                   initWithUpdatedAt:@(playbackPosition.Value().updatedAt)
                                                            position:@(playbackPosition.Value().position.Value())];
                                           }
                                           callback(objCPlaybackPosition);
                                       }
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"mediaPlayback_subscribeSampledPosition"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"mediaPlayback_subscribeSampledPosition"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)mediaPlayback_subscribePlaybackSpeed:(ContentApp * _Nonnull)contentApp
                                 minInterval:(uint16_t)minInterval
                                 maxInterval:(uint16_t)maxInterval
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(float))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
             subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribePlaybackSpeed() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribePlaybackSpeed"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribePlaybackSpeed"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribePlaybackSpeed"];

    [self
        dispatchOnMatterSDKQueue:@"mediaPlayback_subscribePlaybackSpeed(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToPlaybackSpeed(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo::DecodableArgType
                                           playbackSpeed) {
                                       void (^callback)(float) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"mediaPlayback_subscribePlaybackSpeed"];
                                       callback(playbackSpeed);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"mediaPlayback_subscribePlaybackSpeed"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"mediaPlayback_subscribePlaybackSpeed"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)mediaPlayback_subscribeSeekRangeEnd:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribeSeekRangeEnd() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeSeekRangeEnd"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeSeekRangeEnd"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeSeekRangeEnd"];

    [self
        dispatchOnMatterSDKQueue:@"mediaPlayback_subscribeSeekRangeEnd(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType
                                           seekRangeEnd) {
                                       void (^callback)(NSNumber * _Nullable) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"mediaPlayback_subscribeSeekRangeEnd"];
                                       callback(@(seekRangeEnd.Value()));
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"mediaPlayback_subscribeSeekRangeEnd"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"mediaPlayback_subscribeSeekRangeEnd"];
                                       callback();
                                   });

                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)mediaPlayback_subscribeSeekRangeStart:(ContentApp * _Nonnull)contentApp
                                  minInterval:(uint16_t)minInterval
                                  maxInterval:(uint16_t)maxInterval
                                  clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                           requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                              successCallback:(void (^_Nonnull)(NSNumber * _Nullable))successCallback
                              failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
              subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().mediaPlayback_subscribeSeekRangeStart() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"mediaPlayback_subscribeSeekRangeStart"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"mediaPlayback_subscribeSeekRangeStart"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"mediaPlayback_subscribeSeekRangeStart"];

    [self
        dispatchOnMatterSDKQueue:@"mediaPlayback_subscribeSeekRangeStart(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->MediaPlayback_SubscribeToDuration(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType
                                           seekRangeStart) {
                                       void (^callback)(NSNumber * _Nullable) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"mediaPlayback_subscribeSeekRangeStart"];
                                       callback(@(seekRangeStart.Value()));
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"mediaPlayback_subscribeSeekRangeStart"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"mediaPlayback_subscribeSeekRangeStart"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationLauncher_launchApp:(ContentApp * _Nonnull)contentApp
                      catalogVendorId:(uint16_t)catalogVendorId
                        applicationId:(NSString * _Nonnull)applicationId
                                 data:(NSData * _Nullable)data
                     responseCallback:(void (^_Nonnull)(bool))responseCallback
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{

    chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application;
    application.catalogVendorID = catalogVendorId;
    application.applicationID = chip::CharSpan::fromCharString([applicationId UTF8String]);

    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->ApplicationLauncher_LaunchApp(&endpoint, application,
                                         chip::MakeOptional(chip::ByteSpan(static_cast<const uint8_t *>(data.bytes), data.length)),
                                         responseFunction);
                                 }];
}

- (void)applicationLauncher_stopApp:(ContentApp * _Nonnull)contentApp
                    catalogVendorId:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(bool))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application;
    application.catalogVendorID = catalogVendorId;
    application.applicationID = chip::CharSpan::fromCharString([applicationId UTF8String]);

    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->ApplicationLauncher_StopApp(&endpoint, application, responseFunction);
                                 }];
}

- (void)applicationLauncher_hideApp:(ContentApp * _Nonnull)contentApp
                    catalogVendorId:(uint16_t)catalogVendorId
                      applicationId:(NSString * _Nonnull)applicationId
                   responseCallback:(void (^_Nonnull)(bool))responseCallback
                        clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                 requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{

    chip::app::Clusters::ApplicationLauncher::Structs::ApplicationStruct::Type application;
    application.catalogVendorID = catalogVendorId;
    application.applicationID = chip::CharSpan::fromCharString([applicationId UTF8String]);

    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->ApplicationLauncher_HideApp(&endpoint, application, responseFunction);
                                 }];
}

- (void)targetNavigator_navigateTarget:(ContentApp * _Nonnull)contentApp
                                target:(uint8_t)target
                                  data:(NSString * _Nullable)data
                      responseCallback:(void (^_Nonnull)(bool))responseCallback
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->TargetNavigator_NavigateTarget(&endpoint, target,
                                         chip::MakeOptional(chip::CharSpan::fromCharString([data UTF8String])), responseFunction);
                                 }];
}

- (void)targetNavigator_subscribeTargetList:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(NSMutableArray *))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().targetNavigator_subscribeTargetList() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"targetNavigator_subscribeTargetList"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"targetNavigator_subscribeTargetList"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"targetNavigator_subscribeTargetList"];

    [self
        dispatchOnMatterSDKQueue:@"targetNavigator_subscribeTargetList(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->TargetNavigator_SubscribeToTargetList(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo::DecodableArgType
                                           targetList) {
                                       void (^callback)(NSMutableArray *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"targetNavigator_subscribeTargetList"];
                                       NSMutableArray * objCTargetList = nil;
                                       size_t targetInfoCount;
                                       targetList.ComputeSize(&targetInfoCount);
                                       if (targetInfoCount > 0) {
                                           objCTargetList = [NSMutableArray arrayWithCapacity:targetInfoCount];
                                           auto iter = targetList.begin();
                                           while (iter.Next()) {
                                               const chip::app::Clusters::TargetNavigator::Structs::TargetInfoStruct::
                                                   DecodableType & targetInfo
                                                   = iter.GetValue();
                                               TargetNavigator_TargetInfoStruct * objCTargetInfoStruct =
                                                   [[TargetNavigator_TargetInfoStruct alloc]
                                                       initWithIdentifier:@(targetInfo.identifier)
                                                                     name:[NSString stringWithUTF8String:targetInfo.name.data()]];
                                               [objCTargetList addObject:objCTargetInfoStruct];
                                           }
                                       }
                                       callback(objCTargetList);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"targetNavigator_subscribeTargetList"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"targetNavigator_subscribeTargetList"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)targetNavigator_subscribeCurrentTarget:(ContentApp * _Nonnull)contentApp
                                   minInterval:(uint16_t)minInterval
                                   maxInterval:(uint16_t)maxInterval
                                   clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                            requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                               successCallback:(void (^_Nonnull)(uint8_t))successCallback
                               failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
               subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().targetNavigator_subscribeCurrentTarget() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"targetNavigator_subscribeCurrentTarget"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"targetNavigator_subscribeCurrentTarget"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"targetNavigator_subscribeCurrentTarget"];

    [self
        dispatchOnMatterSDKQueue:@"targetNavigator_subscribeCurrentTarget(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->TargetNavigator_SubscribeToCurrentTarget(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo::DecodableArgType
                                           currentTarget) {
                                       void (^callback)(uint8_t) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"targetNavigator_subscribeCurrentTarget"];
                                       callback(currentTarget);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"targetNavigator_subscribeCurrentTarget"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"targetNavigator_subscribeCurrentTarget"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)keypadInput_sendKey:(ContentApp * _Nonnull)contentApp
                    keyCode:(uint8_t)keyCode
           responseCallback:(void (^_Nonnull)(bool))responseCallback
                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
         requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->KeypadInput_SendKey(&endpoint,
                                         static_cast<chip::app::Clusters::KeypadInput::CECKeyCodeEnum>(keyCode), responseFunction);
                                 }];
}

- (void)applicationBasic_subscribeVendorName:(ContentApp * _Nonnull)contentApp
                                 minInterval:(uint16_t)minInterval
                                 maxInterval:(uint16_t)maxInterval
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
             subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeVendorName() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeVendorName"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeVendorName"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"applicationBasic_subscribeVendorName"];

    [self
        dispatchOnMatterSDKQueue:@"applicationBasic_subscribeVendorName(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToVendorName(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType
                                           vendorName) {
                                       void (^callback)(NSString * _Nonnull) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"applicationBasic_subscribeVendorName"];
                                       callback(vendorName.data() != nil ? [NSString stringWithUTF8String:vendorName.data()] : nil);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"applicationBasic_subscribeVendorName"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"applicationBasic_subscribeVendorName"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationBasic_subscribeVendorID:(ContentApp * _Nonnull)contentApp
                               minInterval:(uint16_t)minInterval
                               maxInterval:(uint16_t)maxInterval
                               clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                        requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                           successCallback:(void (^_Nonnull)(NSNumber * _Nonnull))successCallback
                           failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
           subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeVendorID() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeVendorID"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeVendorID"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"applicationBasic_subscribeVendorID"];

    [self
        dispatchOnMatterSDKQueue:@"applicationBasic_subscribeVendorID(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToVendorID(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType
                                           vendorID) {
                                       void (^callback)(NSNumber * _Nonnull) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"applicationBasic_subscribeVendorID"];
                                       callback(@(vendorID));
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"applicationBasic_subscribeVendorID"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"applicationBasic_subscribeVendorID"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationBasic_subscribeApplicationName:(ContentApp * _Nonnull)contentApp
                                      minInterval:(uint16_t)minInterval
                                      maxInterval:(uint16_t)maxInterval
                                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                               requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                  successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                  failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
                  subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeApplicationName() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeApplicationName"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeApplicationName"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback
                                          forKey:@"applicationBasic_subscribeApplicationName"];

    [self
        dispatchOnMatterSDKQueue:@"applicationBasic_subscribeApplicationName(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToApplicationName(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::
                                           DecodableArgType applicationName) {
                                       void (^callback)(NSString * _Nonnull) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"applicationBasic_subscribeApplicationName"];
                                       callback([NSString stringWithUTF8String:applicationName.data()]);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"applicationBasic_subscribeApplicationName"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"applicationBasic_subscribeApplicationName"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationBasic_subscribeProductID:(ContentApp * _Nonnull)contentApp
                                minInterval:(uint16_t)minInterval
                                maxInterval:(uint16_t)maxInterval
                                clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                         requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                            successCallback:(void (^_Nonnull)(uint16_t))successCallback
                            failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
            subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeProductID() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeProductID"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeProductID"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback forKey:@"applicationBasic_subscribeProductID"];

    [self
        dispatchOnMatterSDKQueue:@"applicationBasic_subscribeProductID(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToProductID(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType
                                           productID) {
                                       void (^callback)(uint16_t) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"applicationBasic_subscribeProductID"];
                                       callback(productID);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"applicationBasic_subscribeProductID"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"applicationBasic_subscribeProductID"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationBasic_subscribeApplicationVersion:(ContentApp * _Nonnull)contentApp
                                         minInterval:(uint16_t)minInterval
                                         maxInterval:(uint16_t)maxInterval
                                         clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                                  requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                     successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                     failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
                     subscriptionEstablishedCallback:(void (^_Nonnull)())subscriptionEstablishedCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_subscribeApplicationVersion() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_subscriptionReadSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_subscribeApplicationVersion"];
    [_subscriptionReadFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_subscribeApplicationVersion"];
    [_subscriptionEstablishedCallbacks setObject:subscriptionEstablishedCallback
                                          forKey:@"applicationBasic_subscribeApplicationVersion"];

    [self
        dispatchOnMatterSDKQueue:@"applicationBasic_subscribeApplicationVersion(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_SubscribeToApplicationVersion(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::
                                           DecodableArgType applicationVersion) {
                                       void (^callback)(NSString * _Nonnull) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadSuccessCallbacks
                                               objectForKey:@"applicationBasic_subscribeApplicationVersion"];
                                       callback([NSString stringWithUTF8String:applicationVersion.data()]);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].subscriptionReadFailureCallbacks
                                               objectForKey:@"applicationBasic_subscribeApplicationVersion"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   },
                                   minInterval, maxInterval,
                                   [](void * context, chip::SubscriptionId subscriptionId) {
                                       void (^callback)() =
                                           [[CastingServerBridge getSharedInstance].subscriptionEstablishedCallbacks
                                               objectForKey:@"applicationBasic_subscribeApplicationVersion"];
                                       callback();
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationBasic_readVendorName:(ContentApp * _Nonnull)contentApp
                            clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                     requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                        successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                        failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readVendorName() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readVendorName"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readVendorName"];

    [self
        dispatchOnMatterSDKQueue:[NSString stringWithFormat:@"%s(...)", __func__]
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadVendorName(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType
                                           vendorName) {
                                       void (^callback)(NSString * _Nonnull) =
                                           [[CastingServerBridge getSharedInstance].readSuccessCallbacks
                                               objectForKey:@"applicationBasic_readVendorName"];
                                       callback([NSString stringWithUTF8String:vendorName.data()]);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].readFailureCallbacks
                                               objectForKey:@"applicationBasic_readVendorName"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationBasic_readVendorID:(ContentApp * _Nonnull)contentApp
                          clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                   requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                      successCallback:(void (^_Nonnull)(NSNumber * _Nonnull))successCallback
                      failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readVendorID() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readVendorID"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readVendorID"];

    [self
        dispatchOnMatterSDKQueue:@"applicationBasic_readVendorID(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadVendorID(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType
                                           vendorID) {
                                       void (^callback)(NSNumber * _Nonnull) =
                                           [[CastingServerBridge getSharedInstance].readSuccessCallbacks
                                               objectForKey:@"applicationBasic_readVendorID"];
                                       callback(@(vendorID));
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].readFailureCallbacks
                                               objectForKey:@"applicationBasic_readVendorID"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationBasic_readApplicationName:(ContentApp * _Nonnull)contentApp
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                          requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                             successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                             failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_readApplicationName() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readApplicationName"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readApplicationName"];

    [self
        dispatchOnMatterSDKQueue:@"applicationBasic_readApplicationName(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadApplicationName(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::
                                           DecodableArgType applicationName) {
                                       void (^callback)(NSString * _Nonnull) =
                                           [[CastingServerBridge getSharedInstance].readSuccessCallbacks
                                               objectForKey:@"applicationBasic_readApplicationName"];
                                       callback([NSString stringWithUTF8String:applicationName.data()]);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].readFailureCallbacks
                                               objectForKey:@"applicationBasic_readApplicationName"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationBasic_readProductID:(ContentApp * _Nonnull)contentApp
                           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                    requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                       successCallback:(void (^_Nonnull)(uint16_t))successCallback
                       failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer, "CastingServerBridge().applicationBasic_readProductID() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readProductID"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readProductID"];

    [self
        dispatchOnMatterSDKQueue:@"applicationBasic_readProductID(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadProductID(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType
                                           productID) {
                                       void (^callback)(uint16_t) = [[CastingServerBridge getSharedInstance].readSuccessCallbacks
                                           objectForKey:@"applicationBasic_readProductID"];
                                       callback(productID);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].readFailureCallbacks
                                               objectForKey:@"applicationBasic_readProductID"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)applicationBasic_readApplicationVersion:(ContentApp * _Nonnull)contentApp
                                    clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                             requestSentHandler:(void (^_Nonnull)(MatterError * _Nonnull))requestSentHandler
                                successCallback:(void (^_Nonnull)(NSString * _Nonnull))successCallback
                                failureCallback:(void (^_Nonnull)(MatterError * _Nonnull))failureCallback
{
    ChipLogProgress(AppServer,
        "CastingServerBridge().applicationBasic_readApplicationVersion() called on Content App with endpoint ID %d",
        contentApp.endpointId);

    [_readSuccessCallbacks setObject:successCallback forKey:@"applicationBasic_readApplicationVersion"];
    [_readFailureCallbacks setObject:failureCallback forKey:@"applicationBasic_readApplicationVersion"];

    [self
        dispatchOnMatterSDKQueue:@"applicationBasic_readApplicationVersion(...)"
                           block:^{
                               TargetEndpointInfo endpoint;
                               [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                               CHIP_ERROR err = CastingServer::GetInstance()->ApplicationBasic_ReadApplicationVersion(
                                   &endpoint, nullptr,
                                   [](void * context,
                                       chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::
                                           DecodableArgType applicationVersion) {
                                       void (^callback)(NSString * _Nonnull) =
                                           [[CastingServerBridge getSharedInstance].readSuccessCallbacks
                                               objectForKey:@"applicationBasic_readApplicationVersion"];
                                       callback([NSString stringWithUTF8String:applicationVersion.data()]);
                                   },
                                   [](void * context, CHIP_ERROR err) {
                                       void (^callback)(MatterError *) =
                                           [[CastingServerBridge getSharedInstance].readFailureCallbacks
                                               objectForKey:@"applicationBasic_readApplicationVersion"];
                                       callback([[MatterError alloc] initWithCode:err.AsInteger()
                                                                          message:[NSString stringWithUTF8String:err.AsString()]]);
                                   });
                               dispatch_async(clientQueue, ^{
                                   requestSentHandler(
                                       [[MatterError alloc] initWithCode:err.AsInteger()
                                                                 message:[NSString stringWithUTF8String:err.AsString()]]);
                               });
                           }];
}

- (void)onOff_on:(ContentApp * _Nonnull)contentApp
      responseCallback:(void (^_Nonnull)(bool))responseCallback
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->OnOff_On(&endpoint, responseFunction);
                                 }];
}

- (void)onOff_off:(ContentApp * _Nonnull)contentApp
      responseCallback:(void (^_Nonnull)(bool))responseCallback
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->OnOff_Off(&endpoint, responseFunction);
                                 }];
}

- (void)onOff_toggle:(ContentApp * _Nonnull)contentApp
      responseCallback:(void (^_Nonnull)(bool))responseCallback
           clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    requestSentHandler:(void (^_Nonnull)(bool))requestSentHandler
{
    const NSString * description = [NSString stringWithFormat:@"%s(...) (Content App %d)", __func__, contentApp.endpointId];
    [self withCastingServerInvokeBlock:description
                         callbackQueue:clientQueue
                       onBlockComplete:requestSentHandler
                            onResponse:responseCallback
                                 block:^(CastingServer * castingServer, std::function<void(CHIP_ERROR)> responseFunction) {
                                     TargetEndpointInfo endpoint;
                                     [ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:endpoint];

                                     return castingServer->OnOff_Toggle(&endpoint, responseFunction);
                                 }];
}
@end
