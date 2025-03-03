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

    chip::PeerId peerId;
    CHIP_ERROR err = chip::Dnssd::ExtractIdFromInstanceName(aName, &peerId);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(Controller, "Invalid instance name: '%s'\n", aName);
        return;
    }

    if (!(aFlags & kDNSServiceFlagsAdd)) {
        // We mostly only care about new things appearing, but log it when things
        // disappear.
        MTR_LOG("Matter operational instance advertisement removed: '%s'\n", aName);
        return;
    }

    ChipLogProgress(Controller, "Notifying controller factory about new operational instance: '%s'", aName);
    [self->mDeviceControllerFactory operationalInstanceAdded:peerId];
}

MTROperationalBrowser::~MTROperationalBrowser()
{
    assertChipStackLockedByCurrentThread();

    mIsDestroying = true;

    StopBrowse();
}
