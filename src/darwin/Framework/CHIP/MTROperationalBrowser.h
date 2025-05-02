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

#pragma once

#import <Foundation/Foundation.h>
#import <Matter/MTRDeviceControllerFactory.h>
#import <dns_sd.h>

class MTROperationalBrowser {
public:
    // Should be created at a point when the dispatch queue is available.
    MTROperationalBrowser(MTRDeviceControllerFactory * aFactory, dispatch_queue_t aQueue);

    ~MTROperationalBrowser();

    // ControllerActivated should be called, on the Matter queue, when a
    // controller is either started in a non-suspended state or stops being
    // suspended.

    // ControllerDeactivated should be called, on the Matter queue, when a
    // controller is either suspended or shut down while in a non-suspended
    // state.
    void ControllerActivated();
    void ControllerDeactivated();

private:
    static void OnBrowse(DNSServiceRef aServiceRef, DNSServiceFlags aFlags, uint32_t aInterfaceId, DNSServiceErrorType aError,
        const char * aName, const char * aType, const char * aDomain, void * aContext);

    void EnsureBrowse();
    void StopBrowse();

    MTRDeviceControllerFactory * const __weak mDeviceControllerFactory;
    dispatch_queue_t mQueue;
    DNSServiceRef mBrowseRef;

    // If mInitialized is true, mBrowseRef is valid.
    bool mInitialized = false;

    // If mIsDestroying is true, we're in our destructor, shutting things down.
    bool mIsDestroying = false;

    // Count of controllers that are currently active; we aim to have a browse
    // going while this is nonzero;
    size_t mActiveControllerCount = 0;

    // Queued-up instance names to notify about.
    NSMutableSet<NSString *> * mAddedInstanceNames = nil;
};
