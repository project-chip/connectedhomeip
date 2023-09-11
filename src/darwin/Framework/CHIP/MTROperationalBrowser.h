/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#import <Foundation/Foundation.h>
#import <Matter/MTRDeviceControllerFactory.h>
#import <dns_sd.h>

class MTROperationalBrowser
{
public:
    // Should be created at a point when the factory starts up the event loop,
    // and destroyed when the event loop is stopped.
    MTROperationalBrowser(MTRDeviceControllerFactory * aFactory, dispatch_queue_t aQueue);

    ~MTROperationalBrowser();

private:
    static void OnBrowse(DNSServiceRef aServiceRef, DNSServiceFlags aFlags, uint32_t aInterfaceId, DNSServiceErrorType aError,
                         const char * aName, const char * aType, const char * aDomain, void * aContext);

    void TryToStartBrowse();

    MTRDeviceControllerFactory * const mDeviceControllerFactory;
    dispatch_queue_t mQueue;
    DNSServiceRef mBrowseRef;

    // If mInitialized is true, mBrowseRef is valid.
    bool mInitialized = false;

    // If mIsDestroying is true, we're in our destructor, shutting things down.
    bool mIsDestroying = false;
};
