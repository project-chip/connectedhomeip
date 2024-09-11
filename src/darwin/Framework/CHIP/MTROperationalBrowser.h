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

class MTROperationalBrowser
{
public:
    // Should be created at a point when the dispatch queue is available.
    MTROperationalBrowser(MTRDeviceControllerFactory * aFactory, dispatch_queue_t aQueue);

    ~MTROperationalBrowser();

    // EnsureBrowse is a no-op if a browse has already been started.
    void EnsureBrowse();
    void StopBrowse();

private:
    static void OnBrowse(DNSServiceRef aServiceRef, DNSServiceFlags aFlags, uint32_t aInterfaceId, DNSServiceErrorType aError,
                         const char * aName, const char * aType, const char * aDomain, void * aContext);

    MTRDeviceControllerFactory * const __weak mDeviceControllerFactory;
    dispatch_queue_t mQueue;
    DNSServiceRef mBrowseRef;

    // If mInitialized is true, mBrowseRef is valid.
    bool mInitialized = false;

    // If mIsDestroying is true, we're in our destructor, shutting things down.
    bool mIsDestroying = false;
};
