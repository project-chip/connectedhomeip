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
    NSMutableDictionary<NSString *, nw_connection_t> * _connections;

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
static NSUInteger sConnectivityMonitorCount;
static DNSServiceRef sSharedResolverConnection;
static dispatch_queue_t sSharedResolverQueue;

- (instancetype)initWithInstanceName:(NSString *)instanceName
{
    if (self = [super init]) {
        _instanceName = [instanceName copy];
        _connections = [NSMutableDictionary dictionary];
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
    for (auto & resolver : _resolvers) {
        DNSServiceRefDeallocate(resolver);
    }
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRDeviceConnectivityMonitor: %@>", _instanceName];
}

+ (DNSServiceRef)_sharedResolverConnection
{
    os_unfair_lock_assert_owner(&sConnectivityMonitorLock);

    if (!sSharedResolverConnection) {
        DNSServiceErrorType dnsError = DNSServiceCreateConnection(&sSharedResolverConnection);
        if (dnsError != kDNSServiceErr_NoError) {
            MTR_LOG_ERROR("MTRDeviceConnectivityMonitor: DNSServiceCreateConnection failed %d", dnsError);
            return NULL;
        }
        sSharedResolverQueue = dispatch_queue_create("MTRDeviceConnectivityMonitor", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        dnsError = DNSServiceSetDispatchQueue(sSharedResolverConnection, sSharedResolverQueue);
        if (dnsError != kDNSServiceErr_NoError) {
            MTR_LOG_ERROR("%@ cannot set dispatch queue on resolve", self);
            DNSServiceRefDeallocate(sSharedResolverConnection);
            sSharedResolverConnection = NULL;
            sSharedResolverQueue = nil;
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
    std::lock_guard lock(sConnectivityMonitorLock);

    // dns_sd.h: must check and call deallocate if error is kDNSServiceErr_ServiceNotRunning
    if (error == kDNSServiceErr_ServiceNotRunning) {
        MTR_LOG_ERROR("%@ disconnected from dns-sd subsystem", self);
        [self _stopMonitoring];
        return;
    }

    // Create a nw_connection to monitor connectivity if the host name is not being monitored yet
    NSString * hostNameString = [NSString stringWithUTF8String:hostName];
    if (!_connections[hostNameString]) {
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
                    std::lock_guard lock(sConnectivityMonitorLock);
                    MTR_LOG("%@ connectivity now viable", self);
                    [self _callHandler];
                }
            }
        });
        nw_connection_start(connection);

        _connections[hostNameString] = connection;
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
    auto * connectivityMonitor = (__bridge MTRDeviceConnectivityMonitor *) context;
    [connectivityMonitor handleResolvedHostname:hostName port:port error:errorCode];
}

- (void)startMonitoringWithHandler:(MTRDeviceConnectivityMonitorHandler)handler queue:(dispatch_queue_t)queue
{
    std::lock_guard lock(sConnectivityMonitorLock);

    _monitorHandler = handler;
    _handlerQueue = queue;

    // If there's already a resolver running, just return
    if (_resolvers.size() != 0) {
        MTR_LOG("%@ connectivity monitor already running", self);
        return;
    }

    MTR_LOG("%@ start connectivity monitoring for %@ (%lu monitoring objects)", self, _instanceName, static_cast<unsigned long>(sConnectivityMonitorCount));

    auto sharedConnection = [MTRDeviceConnectivityMonitor _sharedResolverConnection];
    if (!sharedConnection) {
        MTR_LOG_ERROR("%@ failed to get shared resolver connection", self);
        return;
    }

    for (auto domain : kResolveDomains) {
        DNSServiceRef resolver = sharedConnection;
        DNSServiceErrorType dnsError = DNSServiceResolve(&resolver,
            kDNSServiceFlagsShareConnection,
            kDNSServiceInterfaceIndexAny,
            _instanceName.UTF8String,
            kOperationalType,
            domain,
            ResolveCallback,
            (__bridge void *) self);
        if (dnsError == kDNSServiceErr_NoError) {
            _resolvers.emplace_back(std::move(resolver));
        } else {
            MTR_LOG_ERROR("%@ failed to create resolver for \"%s\" domain: %" PRId32, self, StringOrNullMarker(domain), dnsError);
        }
    }

    if (_resolvers.size() != 0) {
        sConnectivityMonitorCount++;
    }
}

- (void)_stopMonitoring
{
    os_unfair_lock_assert_owner(&sConnectivityMonitorLock);
    for (NSString * hostName in _connections) {
        nw_connection_cancel(_connections[hostName]);
    }
    [_connections removeAllObjects];

    _monitorHandler = nil;
    _handlerQueue = nil;

    if (_resolvers.size() != 0) {
        for (auto & resolver : _resolvers) {
            DNSServiceRefDeallocate(resolver);
        }
        _resolvers.clear();

        // If no monitor objects exist, schedule to deallocate shared connection and queue
        sConnectivityMonitorCount--;
        if (!sConnectivityMonitorCount) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, kSharedConnectionLingerIntervalSeconds * NSEC_PER_SEC), sSharedResolverQueue, ^{
                std::lock_guard lock(sConnectivityMonitorLock);

                if (!sConnectivityMonitorCount) {
                    MTR_LOG("MTRDeviceConnectivityMonitor: Closing shared resolver connection");
                    DNSServiceRefDeallocate(sSharedResolverConnection);
                    sSharedResolverConnection = NULL;
                    sSharedResolverQueue = nil;
                }
            });
        }
    }
}

- (void)stopMonitoring
{
    // DNSServiceRefDeallocate must be called on the same queue set on the shared connection.
    dispatch_async(sSharedResolverQueue, ^{
        MTR_LOG("%@ stop connectivity monitoring for %@", self, self->_instanceName);
        std::lock_guard lock(sConnectivityMonitorLock);
        [self _stopMonitoring];
    });
}
@end
