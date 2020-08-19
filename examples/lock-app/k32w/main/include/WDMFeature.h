/*
 *
 *    Copyright (c) 2020 Google LLC.
 *    All rights reserved.
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

#ifndef WDM_FEATURE_H
#define WDM_FEATURE_H

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <Weave/Profiles/data-management/Current/DataManagement.h>

#include "traits/include/BoltLockTraitDataSource.h"
#include "traits/include/DeviceIdentityTraitDataSource.h"
#include "traits/include/BoltLockSettingsTraitDataSink.h"

#include "FreeRTOS.h"
#include "semphr.h"

class PublisherLock : public nl::Weave::Profiles::DataManagement::IWeavePublisherLock
{
public:
    // Creates a recursice mutex
    int Init();

    // Takes the mutex recursively.
    WEAVE_ERROR Lock();

    // Gives the mutex recursively.
    WEAVE_ERROR Unlock();

private:
    SemaphoreHandle_t mRecursiveLock;
};

class WDMFeature
{
    typedef ::nl::Weave::Profiles::DataManagement_Current::SubscriptionClient SubscriptionClient;
    typedef ::nl::Weave::Profiles::DataManagement_Current::SubscriptionEngine SubscriptionEngine;
    typedef ::nl::Weave::Profiles::DataManagement_Current::SubscriptionHandler SubscriptionHandler;
    typedef ::nl::Weave::Profiles::DataManagement_Current::TraitDataSink TraitDataSink;
    typedef ::nl::Weave::Profiles::DataManagement_Current::TraitDataSource TraitDataSource;
    typedef ::nl::Weave::Profiles::DataManagement_Current::TraitPath TraitPath;
    typedef ::nl::Weave::Profiles::DataManagement_Current::ResourceIdentifier ResourceIdentifier;
    typedef ::nl::Weave::Profiles::DataManagement_Current::PropertyPathHandle PropertyPathHandle;

public:
    WDMFeature(void);
    WEAVE_ERROR Init(void);
    void ProcessTraitChanges(void);
    void TearDownSubscriptions(void);

    bool AreServiceSubscriptionsEstablished(void);

    BoltLockTraitDataSource & GetBoltLockTraitDataSource(void);

    nl::Weave::Profiles::DataManagement::SubscriptionEngine mSubscriptionEngine;

private:
    friend WDMFeature & WdmFeature(void);

    enum SourceTraitHandle
    {
        kSourceHandle_BoltLockTrait = 0,
        kSourceHandle_DeviceIdentityTrait,

        kSourceHandle_Max
    };

    enum SinkTraitHandle
    {
        kSinkHandle_BoltLockSettingsTrait = 0,

        kSinkHandle_Max
    };

    // Published Traits
    BoltLockTraitDataSource mBoltLockTraitSource;
    DeviceIdentityTraitDataSource mDeviceIdentityTraitSource;

    // Subscribed Traits
    BoltLockSettingsTraitDataSink mBoltLockSettingsTraitSink;

    void InitiateSubscriptionToService(void);
    static void AsyncProcessChanges(intptr_t arg);

    static void PlatformEventHandler(const ::nl::Weave::DeviceLayer::WeaveDeviceEvent * event, intptr_t arg);
    static void HandleSubscriptionEngineEvent(void * appState, SubscriptionEngine::EventID eventType,
                                              const SubscriptionEngine::InEventParam & inParam,
                                              SubscriptionEngine::OutEventParam & outParam);
    static void HandleServiceBindingEvent(void * appState, ::nl::Weave::Binding::EventType eventType,
                                          const ::nl::Weave::Binding::InEventParam & inParam,
                                          ::nl::Weave::Binding::OutEventParam & outParam);
    static void HandleOutboundServiceSubscriptionEvent(void * appState, SubscriptionClient::EventID eventType,
                                                       const SubscriptionClient::InEventParam & inParam,
                                                       SubscriptionClient::OutEventParam & outParam);
    static void HandleInboundSubscriptionEvent(void * aAppState, SubscriptionHandler::EventID eventType,
                                               const SubscriptionHandler::InEventParam & inParam,
                                               SubscriptionHandler::OutEventParam & outParam);

    // Sink Catalog
    nl::Weave::Profiles::DataManagement::SingleResourceSinkTraitCatalog::CatalogItem mServiceSinkCatalogStore[kSinkHandle_Max];
    nl::Weave::Profiles::DataManagement::SingleResourceSinkTraitCatalog mServiceSinkTraitCatalog;

    // Source Catalog
    nl::Weave::Profiles::DataManagement::SingleResourceSourceTraitCatalog mServiceSourceTraitCatalog;
    nl::Weave::Profiles::DataManagement::SingleResourceSourceTraitCatalog::CatalogItem
        mServiceSourceCatalogStore[kSourceHandle_Max];

    nl::Weave::Profiles::DataManagement::TraitPath mServiceSinkTraitPaths[kSinkHandle_Max];

    // Subscription Clients
    nl::Weave::Profiles::DataManagement::SubscriptionClient * mServiceSubClient;

    // Subscription Handler
    nl::Weave::Profiles::DataManagement::SubscriptionHandler * mServiceCounterSubHandler;

    // Binding
    nl::Weave::Binding * mServiceSubBinding;

    static WDMFeature sWDMfeature;
    PublisherLock mPublisherLock;

    bool mIsSubToServiceEstablished;
    bool mIsServiceCounterSubEstablished;
    bool mIsSubToServiceActivated;
};

inline WDMFeature & WdmFeature(void)
{
    return WDMFeature::sWDMfeature;
}

inline BoltLockTraitDataSource & WDMFeature::GetBoltLockTraitDataSource(void)
{
    return mBoltLockTraitSource;
}

#endif // WDM_FEATURE_H
