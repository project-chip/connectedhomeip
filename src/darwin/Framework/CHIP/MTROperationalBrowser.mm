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
    // If we fail to start a browse, there's nothing our consumer would do
    // differently, so we might as well do this in the constructor.
    TryToStartBrowse();
}

void MTROperationalBrowser::TryToStartBrowse()
{
    assertChipStackLockedByCurrentThread();

    ChipLogProgress(Controller, "Trying to start persistent operational browse");

    auto err = DNSServiceCreateConnection(&mBrowseRef);
    if (err != kDNSServiceErr_NoError) {
        ChipLogError(Controller, "Failed to create connection for persistent operational browse: %" PRId32, err);
        return;
    }

    err = DNSServiceSetDispatchQueue(mBrowseRef, mQueue);
    if (err != kDNSServiceErr_NoError) {
        ChipLogError(Controller, "Failed to set up dispatch queue properly for persistent operational browse: %" PRId32, err);
        DNSServiceRefDeallocate(mBrowseRef);
        return;
    }

    mInitialized = true;

    for (auto domain : kBrowseDomains) {
        auto browseRef = mBrowseRef; // Mandatory copy because of kDNSServiceFlagsShareConnection.
        err = DNSServiceBrowse(&browseRef, kDNSServiceFlagsShareConnection, kDNSServiceInterfaceIndexAny, kOperationalType, domain, OnBrowse, this);
        if (err != kDNSServiceErr_NoError) {
            ChipLogError(Controller, "Failed to start persistent operational browse for \"%s\" domain: %" PRId32, StringOrNullMarker(domain), err);
        }
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
        DNSServiceRefDeallocate(self->mBrowseRef);
        self->mInitialized = false;

        // We shouldn't really get callbacks under our destructor, but guard
        // against it just in case.
        if (!self->mIsDestroying) {
            // Try to start a new browse, so we have one going.
            self->TryToStartBrowse();
        }
        return;
    }

    if (!(aFlags & kDNSServiceFlagsAdd)) {
        // We only care about new things appearing.
        return;
    }

    chip::PeerId peerId;
    CHIP_ERROR err = chip::Dnssd::ExtractIdFromInstanceName(aName, &peerId);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(Controller, "Invalid instance name: '%s'\n", aName);
        return;
    }

    ChipLogProgress(Controller, "Notifying controller factory about new operational instance: '%s'", aName);
    [self->mDeviceControllerFactory operationalInstanceAdded:peerId];
}

MTROperationalBrowser::~MTROperationalBrowser()
{
    assertChipStackLockedByCurrentThread();

    mIsDestroying = true;

    if (mInitialized) {
        DNSServiceRefDeallocate(mBrowseRef);
    }
}
