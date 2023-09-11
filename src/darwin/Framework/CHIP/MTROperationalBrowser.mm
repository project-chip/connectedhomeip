/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRDeviceControllerFactory_Internal.h"
#import "MTROperationalBrowser.h"

#include <cinttypes>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/LockTracker.h>

namespace {
constexpr const char kLocalDot[] = "local.";
constexpr const char kOperationalType[] = "_matter._tcp";
constexpr DNSServiceFlags kBrowseFlags = 0;
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

    ChipLogProgress(Controller, "Trying to start operational browse");

    auto err
        = DNSServiceBrowse(&mBrowseRef, kBrowseFlags, kDNSServiceInterfaceIndexAny, kOperationalType, kLocalDot, OnBrowse, this);
    if (err != kDNSServiceErr_NoError) {
        ChipLogError(Controller, "Failed to start operational browse: %" PRId32, err);
        return;
    }

    err = DNSServiceSetDispatchQueue(mBrowseRef, mQueue);
    if (err != kDNSServiceErr_NoError) {
        ChipLogError(Controller, "Failed to set up dispatch queue properly");
        DNSServiceRefDeallocate(mBrowseRef);
        return;
    }

    mInitialized = true;
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
