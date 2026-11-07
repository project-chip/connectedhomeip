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

#import <Foundation/Foundation.h>

#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTROperationalBrowser.h"

#include <cinttypes>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/LockTracker.h>

namespace {
constexpr const char * kBrowseDomains[] = {
    "default.service.arpa.", // SRP
    "local.",
};
constexpr char kOperationalType[] = "_matter._tcp";
}

MTROperationalBrowser::MTROperationalBrowser(MTRDeviceControllerFactory * aFactory, dispatch_queue_t aQueue)
    : mDeviceControllerFactory(aFactory)
    , mQueue(aQueue)
{
}

void MTROperationalBrowser::ControllerActivated()
{
    assertChipStackLockedByCurrentThread();

    if (mActiveControllerCount == 0) {
        EnsureBrowse();
    }
    ++mActiveControllerCount;
}

void MTROperationalBrowser::ControllerDeactivated()
{
    assertChipStackLockedByCurrentThread();

    if (mActiveControllerCount == 1) {
        StopBrowse();
    }

    --mActiveControllerCount;
}

void MTROperationalBrowser::EnsureBrowse()
{
    assertChipStackLockedByCurrentThread();

    if (mInitialized) {
        ChipLogProgress(Controller, "%p already has a persistent operational browse running", this);
        return;
    }

    ChipLogProgress(Controller, "%p trying to start persistent operational browse", this);

    auto err = DNSServiceCreateConnection(&mBrowseRef);
    if (err != kDNSServiceErr_NoError) {
        ChipLogError(Controller, "%p failed to create connection for persistent operational browse: %" PRId32, this, err);
        return;
    }

    err = DNSServiceSetDispatchQueue(mBrowseRef, mQueue);
    if (err != kDNSServiceErr_NoError) {
        ChipLogError(Controller, "%p failed to set up dispatch queue properly for persistent operational browse: %" PRId32, this, err);
        DNSServiceRefDeallocate(mBrowseRef);
        return;
    }

    mAddedInstanceNames = [[NSMutableSet alloc] init];
    mPresentInstances = [[NSMutableDictionary alloc] init];

    mInitialized = true;

    for (auto domain : kBrowseDomains) {
        auto browseRef = mBrowseRef; // Mandatory copy because of kDNSServiceFlagsShareConnection.
        err = DNSServiceBrowse(&browseRef, kDNSServiceFlagsShareConnection, kDNSServiceInterfaceIndexAny, kOperationalType, domain, OnBrowse, this);
        if (err != kDNSServiceErr_NoError) {
            ChipLogError(Controller, "%p failed to start persistent operational browse for \"%s\" domain: %" PRId32, this, StringOrNullMarker(domain), err);
        }
    }
}

void MTROperationalBrowser::StopBrowse()
{
    ChipLogProgress(Controller, "%p stopping persistent operational browse", this);
    if (mInitialized) {
        mAddedInstanceNames = nil;
        mPresentInstances = nil;
        DNSServiceRefDeallocate(mBrowseRef);
        mInitialized = false;
    }
}

void MTROperationalBrowser::OnBrowse(DNSServiceRef aServiceRef, DNSServiceFlags aFlags, uint32_t aInterfaceId,
    DNSServiceErrorType aError, const char * aName, const char * aType, const char * aDomain, void * aContext)
{
    assertChipStackLockedByCurrentThread();

    auto self = static_cast<MTROperationalBrowser *>(aContext);

    // We only expect to get notified about our type/domain.
    if (aError != kDNSServiceErr_NoError) {
        ChipLogError(Controller, "Operational browse failure: %" PRId32, aError);
        self->StopBrowse();

        // We shouldn't really get callbacks under our destructor, but guard
        // against it just in case.
        if (!self->mIsDestroying) {
            // Try to start a new browse, so we have one going.
            self->EnsureBrowse();
        }
        return;
    }

    auto * newInstanceName = [NSString stringWithUTF8String:aName];
    auto * domain = @(StringOrNullMarker(aDomain));
    NSArray * location = @[ @(aInterfaceId), domain ];
    NSMutableSet<NSArray *> * locations = self->mPresentInstances[newInstanceName];
    if (aFlags & kDNSServiceFlagsAdd) {
        // Notify only when the instance goes from browsed nowhere to browsed
        // somewhere.  Known trade-off: if a device re-advertises on interface B
        // while a stale record is still cached on interface A, we do not notify
        // until mDNSResponder removes the stale record from A and the instance
        // disappears everywhere.  This relies on mDNSResponder delivering a
        // Remove when a record expires or its interface goes away.
        MTR_LOG("Matter operational instance advertisement added: '%@' on interface %" PRIu32 " in %@", newInstanceName, aInterfaceId, domain);
        if (locations == nil) {
            locations = [[NSMutableSet alloc] init];
            self->mPresentInstances[newInstanceName] = locations;
            [self->mAddedInstanceNames addObject:newInstanceName];
        }
        [locations addObject:location];
    } else {
        // Note that a removal does not return early: a batch can end with a
        // removal (e.g. an Add with MoreComing followed by a Remove of a
        // different instance), and the pending Adds still have to be delivered
        // at the end of that batch.  Before, they waited for the next Add.
        //
        // A removal for an instance or location we do not know about is
        // harmless: nil `locations` gives count == 0, and removing a missing
        // key or name is a no-op.
        MTR_LOG("Matter operational instance advertisement removed: '%@' on interface %" PRIu32 " in %@", newInstanceName, aInterfaceId, domain);
        [locations removeObject:location];
        if (locations.count == 0) {
            [self->mPresentInstances removeObjectForKey:newInstanceName];
            // Make sure that we don't notify about things that appear and then disappear.
            [self->mAddedInstanceNames removeObject:newInstanceName];
        }
    }

    if (aFlags & kDNSServiceFlagsMoreComing) {
        // Just wait for those other notifications, so we coalesce everything.
        return;
    }

    for (NSString * instanceName in self->mAddedInstanceNames) {
        chip::PeerId peerId;
        CHIP_ERROR err = chip::Dnssd::ExtractIdFromInstanceName(instanceName.UTF8String, &peerId);
        if (err != CHIP_NO_ERROR) {
            ChipLogError(Controller, "Invalid instance name: '%@'\n", instanceName);
            continue;
        }

        ChipLogProgress(Controller, "Notifying controller factory about new operational instance: '%@'", instanceName);
        [self->mDeviceControllerFactory operationalInstanceAdded:peerId];
    }
    [self->mAddedInstanceNames removeAllObjects];
}

#ifdef DEBUG
void MTROperationalBrowser::UnitTestInjectBrowseResult(DNSServiceFlags aFlags, uint32_t aInterfaceId, const char * aName, const char * aDomain)
{
    assertChipStackLockedByCurrentThread();
    // Without a running browse there is no tracking state, so results would be
    // notified without being tracked.
    VerifyOrDie(mInitialized);
    OnBrowse(nullptr, aFlags, aInterfaceId, kDNSServiceErr_NoError, aName, kOperationalType, aDomain, this);
}
#endif

MTROperationalBrowser::~MTROperationalBrowser()
{
    assertChipStackLockedByCurrentThread();

    mIsDestroying = true;

    StopBrowse();
}
