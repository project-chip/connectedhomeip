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

#import "MTRDeviceConnectivityMonitor.h"
#import "MTRLogging_Internal.h"
#import "MTRUnfairLock.h"

#import <Network/Network.h>
#import <dns_sd.h>
#import <os/lock.h>

#include <lib/dnssd/ServiceNaming.h>
#include <lib/support/CodeUtils.h>
#include <vector>

@implementation MTRDeviceConnectivityMonitor {
    NSString * _instanceName;
    std::vector<DNSServiceRef> _resolvers;
    NSMutableDictionary<NSString *, nw_connection_t> * _connectionsByHostname;
    uintptr_t _monitorID; // Unique ID for safe DNS-SD callback lookup

    MTRDeviceConnectivityMonitorHandler _monitorHandler;
    dispatch_queue_t _handlerQueue;
}

namespace {
constexpr const char * kResolveDomains[] = {
    "default.service.arpa.", // SRP
    "local.",
};
constexpr char kOperationalType[] = "_matter._tcp";
constexpr int64_t kSharedConnectionLingerIntervalSeconds = (10);
}

static os_unfair_lock sConnectivityMonitorLock = OS_UNFAIR_LOCK_INIT;

// Tracks number of currently active monitors. When this drops to zero, triggers timer
// to cleanup sSharedResolverConnection after kSharedConnectionLingerIntervalSeconds.
static NSUInteger sConnectivityMonitorCount;
static DNSServiceRef sSharedResolverConnection;
static dispatch_queue_t sSharedResolverQueue;

// Map table solution for DNS-SD callback safety:
// Instead of passing raw object pointers to DNS-SD callbacks (which creates use-after-free
// vulnerabilities), we assign each MTRDeviceConnectivityMonitor a unique numerical ID and
// pass that ID as the context. The callback then looks up the monitor object in this weak
// map table. If the object has been deallocated, the weak reference becomes nil and the
// callback is safely ignored. This completely eliminates race conditions between object
// deallocation and asynchronous DNS-SD callbacks.
static NSMapTable<NSNumber *, MTRDeviceConnectivityMonitor *> * sMonitorMap;
static uintptr_t sNextMonitorID = 1;

- (instancetype)initWithInstanceName:(NSString *)instanceName
{
    if (self = [super init]) {
        _instanceName = [instanceName copy];
        _connectionsByHostname = [NSMutableDictionary dictionary];

        // Initialize map table once
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            sMonitorMap = [NSMapTable strongToWeakObjectsMapTable];
        });

        // Assign unique ID and register in map
        std::lock_guard lock(sConnectivityMonitorLock);

        // Mask to ensure monitor IDs fit within pointer size.
        // Casting UINTPTR_MAX through void* naturally truncates to pointer width.
        // If uintptr_t were somehow larger than void*, this mask ensures IDs
        // stay within the representable range when cast to void*.
        uintptr_t pointerSizeMask = (uintptr_t) (void *) (~(uintptr_t) 0);

        // Find an unused ID with bounded search to handle potential wrap-around
        uintptr_t candidateID = sNextMonitorID;
        uintptr_t attempts = 0;

        while ([sMonitorMap objectForKey:@(candidateID)] != nil) {
            attempts++;
            // Check against max pointer values, not UINTPTR_MAX
            NSAssert(attempts < pointerSizeMask, @"Exhausted all monitor IDs - this should never happen");

            candidateID = (candidateID + 1) & pointerSizeMask; // Wrap within pointer size
        }

        _monitorID = candidateID;
        sNextMonitorID = (candidateID + 1) & pointerSizeMask; // Ensure next ID also fits
        [sMonitorMap setObject:self forKey:@(_monitorID)];
    }
    return self;
}

- (instancetype)initWithCompressedFabricID:(NSNumber *)compressedFabricID nodeID:(NSNumber *)nodeID
{
    char instanceName[chip::Dnssd::kMaxOperationalServiceNameSize];
    chip::PeerId peerId(static_cast<chip::CompressedFabricId>(compressedFabricID.unsignedLongLongValue), static_cast<chip::NodeId>(nodeID.unsignedLongLongValue));
    CHIP_ERROR err = chip::Dnssd::MakeInstanceName(instanceName, sizeof(instanceName), peerId);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("%@ could not make instance name", self);
        return nil;
    }

    return [self initWithInstanceName:[NSString stringWithUTF8String:instanceName]];
}

- (void)dealloc
{
    std::lock_guard lock(sConnectivityMonitorLock);

    // Remove from map first - this makes any in-flight DNS callbacks safe
    // because they'll find nil when looking up our ID
    [sMonitorMap removeObjectForKey:@(_monitorID)];

    // Clean up all monitoring state
    [self _stopMonitoring];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRDeviceConnectivityMonitor: %p %@>", self, _instanceName];
}

+ (DNSServiceRef)_sharedResolverConnection
{
    os_unfair_lock_assert_owner(&sConnectivityMonitorLock);

    if (!sSharedResolverConnection) {
        DNSServiceErrorType dnsError = DNSServiceCreateConnection(&sSharedResolverConnection);
        if (dnsError != kDNSServiceErr_NoError) {
            MTR_LOG_ERROR("MTRDeviceConnectivityMonitor: DNSServiceCreateConnection failed %" PRId32, dnsError);
            return NULL;
        }
        dnsError = DNSServiceSetDispatchQueue(sSharedResolverConnection, sSharedResolverQueue);
        if (dnsError != kDNSServiceErr_NoError) {
            MTR_LOG_ERROR("MTRDeviceConnectivityMonitor: cannot set dispatch queue on resolver connection: %" PRId32, dnsError);
            DNSServiceRefDeallocate(sSharedResolverConnection);
            sSharedResolverConnection = NULL;
            return NULL;
        }
    }

    return sSharedResolverConnection;
}

- (void)_callHandler
{
    os_unfair_lock_assert_owner(&sConnectivityMonitorLock);
    MTRDeviceConnectivityMonitorHandler handlerToCall = self->_monitorHandler;
    if (handlerToCall) {
        dispatch_async(self->_handlerQueue, handlerToCall);
    }
}

- (void)handleResolvedHostname:(const char *)hostName port:(uint16_t)port error:(DNSServiceErrorType)error
{
    os_unfair_lock_assert_owner(&sConnectivityMonitorLock);

    // If we're stopped (resolvers cleared), ignore late callbacks
    if (_resolvers.size() == 0) {
        return;
    }

    if (hostName == NULL) {
        MTR_LOG_ERROR("%@ NULL host resolved, ignoring", self);
        return;
    }
    // dns_sd.h: must check and call deallocate if error is kDNSServiceErr_ServiceNotRunning
    if (error == kDNSServiceErr_ServiceNotRunning) {
        MTR_LOG_ERROR("%@ disconnected from dns-sd subsystem", self);
        // Notify caller to proceed with connection attempt despite DNS-SD failure.
        // Waiting indefinitely would cause hangs, so we trigger the handler to unblock.
        [self _callHandler];
        [self _stopMonitoring];
        return;
    }

    // Create a nw_connection to monitor connectivity if the host name is not being monitored yet
    NSString * hostNameString = [NSString stringWithUTF8String:hostName];
    if (!_connectionsByHostname[hostNameString]) {
        char portString[6];
        snprintf(portString, sizeof(portString), "%d", ntohs(port));
        nw_endpoint_t endpoint = nw_endpoint_create_host(hostName, portString);
        if (!endpoint) {
            MTR_LOG_ERROR("%@ failed to create endpoint for %s:%s", self, hostName, portString);
            return;
        }
        nw_parameters_t params = nw_parameters_create_secure_udp(NW_PARAMETERS_DISABLE_PROTOCOL, NW_PARAMETERS_DEFAULT_CONFIGURATION);
        if (!params) {
            MTR_LOG_ERROR("%@ failed to create udp parameters", self);
            return;
        }
        nw_connection_t connection = nw_connection_create(endpoint, params);
        if (!connection) {
            MTR_LOG_ERROR("%@ failed to create connection for %s:%s", self, hostName, portString);
            return;
        }
        nw_connection_set_queue(connection, sSharedResolverQueue);
        mtr_weakify(self);
        nw_connection_set_path_changed_handler(connection, ^(nw_path_t _Nonnull path) {
            mtr_strongify(self);
            if (self) {
                nw_path_status_t status = nw_path_get_status(path);
                if (status == nw_path_status_satisfied) {
                    MTR_LOG("%@ path is satisfied", self);
                    std::lock_guard lock(sConnectivityMonitorLock);
                    [self _callHandler];
                }
            }
        });
        nw_connection_set_viability_changed_handler(connection, ^(bool viable) {
            mtr_strongify(self);
            if (self) {
                if (viable) {
                    MTR_LOG("%@ connectivity now viable", self);
                    std::lock_guard lock(sConnectivityMonitorLock);
                    [self _callHandler];
                }
            }
        });
        nw_connection_start(connection);

        _connectionsByHostname[hostNameString] = connection;
    }
}

static void ResolveCallback(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char * fullName,
    const char * hostName,
    uint16_t port, /* In network byte order */
    uint16_t txtLen,
    const unsigned char * txtRecord,
    void * context)
{
    // Context contains monitor ID, to look up the monitor object
    uintptr_t monitorID = reinterpret_cast<uintptr_t>(context);

    std::lock_guard lock(sConnectivityMonitorLock);
    MTRDeviceConnectivityMonitor * monitor = [sMonitorMap objectForKey:@(monitorID)];

    // If monitor is nil, object was deallocated - callback safely ignored
    if (!monitor) {
        return;
    }

    [monitor handleResolvedHostname:hostName port:port error:errorCode];
}

- (BOOL)startMonitoringWithHandler:(MTRDeviceConnectivityMonitorHandler)handler queue:(dispatch_queue_t)queue
{
    // Initialize sSharedResolverQueue on first use
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sSharedResolverQueue = dispatch_queue_create("MTRDeviceConnectivityMonitor", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    });

    __block BOOL result = NO;

    // Run entire function on sSharedResolverQueue to maintain lock ordering (queue → lock)
    // and avoid deadlocks with callbacks that also use this pattern.
    dispatch_sync(sSharedResolverQueue, ^{
        std::lock_guard lock(sConnectivityMonitorLock);

        _monitorHandler = handler;
        _handlerQueue = queue;

        MTR_LOG("%@ start connectivity monitoring for %@ (current monitoring objects: %lu)", self, _instanceName, static_cast<unsigned long>(sConnectivityMonitorCount));

        // If there's already a resolver running, just return
        if (_resolvers.size() != 0) {
            MTR_LOG("%@ connectivity monitor already running", self);
            result = YES;
            return;
        }

        auto sharedConnection = [MTRDeviceConnectivityMonitor _sharedResolverConnection];
        if (!sharedConnection) {
            MTR_LOG_ERROR("%@ failed to get shared resolver connection", self);
            _monitorHandler = nil;
            _handlerQueue = nil;
            result = NO;
            return;
        }

        // We're already on the connection's dispatch queue, so call DNSServiceResolve directly
        for (auto domain : kResolveDomains) {
            DNSServiceRef resolver = sharedConnection;
            DNSServiceErrorType dnsError = DNSServiceResolve(&resolver,
                kDNSServiceFlagsShareConnection,
                kDNSServiceInterfaceIndexAny,
                _instanceName.UTF8String,
                kOperationalType,
                domain,
                ResolveCallback,
                reinterpret_cast<void *>(_monitorID)); // Pass ID as context, not object pointer
            if (dnsError == kDNSServiceErr_NoError) {
                _resolvers.emplace_back(std::move(resolver));
            } else {
                MTR_LOG_ERROR("%@ failed to create resolver for \"%s\" domain: %" PRId32, self, StringOrNullMarker(domain), dnsError);
            }
        }

        if (_resolvers.size() != 0) {
            sConnectivityMonitorCount++;
            result = YES;
            return;
        }

        // No resolvers created - clear handler and return failure
        MTR_LOG_ERROR("%@ failed to create any resolvers for %@", self, _instanceName);
        _monitorHandler = nil;
        _handlerQueue = nil;
        result = NO;
    });

    return result;
}

- (void)_clearResolvers:(std::vector<DNSServiceRef>)resolversToCleanUp
{
    if (resolversToCleanUp.empty()) {
        return;
    }

    // Deallocate DNS resources on correct queue
    dispatch_async(sSharedResolverQueue, ^{
        for (auto & resolver : resolversToCleanUp) {
            DNSServiceRefDeallocate(resolver);
        }

        // Check if we should do linger cleanup
        std::lock_guard lock(sConnectivityMonitorLock);
        if (!sConnectivityMonitorCount) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, kSharedConnectionLingerIntervalSeconds * NSEC_PER_SEC), sSharedResolverQueue, ^{
                std::lock_guard lock(sConnectivityMonitorLock);
                if (!sConnectivityMonitorCount && sSharedResolverConnection) {
                    MTR_LOG("MTRDeviceConnectivityMonitor: Closing shared resolver connection");
                    DNSServiceRefDeallocate(sSharedResolverConnection);
                    sSharedResolverConnection = NULL;
                }
            });
        }
    });
}

- (void)_stopMonitoring
{
    os_unfair_lock_assert_owner(&sConnectivityMonitorLock);

    for (NSString * hostName in _connectionsByHostname) {
        nw_connection_cancel(_connectionsByHostname[hostName]);
    }
    [_connectionsByHostname removeAllObjects];

    _monitorHandler = nil;
    _handlerQueue = nil;

    if (_resolvers.size() != 0) {
        sConnectivityMonitorCount--;
        auto resolversToCleanUp = std::move(_resolvers);
        [self _clearResolvers:resolversToCleanUp]; // Async DNS cleanup
    }
}

- (void)stopMonitoring
{
    MTR_LOG("%@ stop connectivity monitoring for %@", self, self->_instanceName);
    std::lock_guard lock(sConnectivityMonitorLock);
    [self _stopMonitoring]; // Now fully synchronous state clearing
}

#pragma mark - Testing Support

- (uintptr_t)monitorID
{
    return _monitorID;
}

#ifdef DEBUG
+ (BOOL)unitTestHasActiveSharedConnection
{
    std::lock_guard lock(sConnectivityMonitorLock);
    return sSharedResolverConnection != NULL;
}
#endif

@end
