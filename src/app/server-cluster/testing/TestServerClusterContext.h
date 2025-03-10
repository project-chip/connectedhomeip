/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#pragma once

#include "app/data-model-provider/Context.h"
#include <app/data-model-provider/ActionContext.h>
#include <app/data-model-provider/Provider.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/testing/EmptyProvider.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestProviderChangeListener.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace Test {

/// An action context that does not have a current exchange (just returns nullptr)
class NullActionContext : public app::DataModel::ActionContext
{
public:
    Messaging::ExchangeContext * CurrentExchange() override { return nullptr; }
};

/// This is a ServerClusterContext that is initialized with VALID
/// entries that can then be used during testing
///
/// NOTE:
///   At thist time, `interactionContext::actionContext::CurrentExchange` WILL return nullptr
///   in the existing implementation as the exchange is too heavy of an object
///   to create for testing
class TestServerClusterContext
{
public:
    TestServerClusterContext() :
        mContext{
            .provider           = &mTestProvider,
            .storage            = &mTestStorage,
            .interactionContext = &mTestContext,
        }
    {
        mTestContext.eventsGenerator         = &mTestEventsGenerator;
        mTestContext.dataModelChangeListener = &mTestDataModelChangeListener;
        mTestContext.actionContext           = &mNullActionContext;
    }

    /// Get a stable pointer to the underlying context
    app::ServerClusterContext * Get() { return &mContext; }

    /// Create a new context bound to this test context
    app::ServerClusterContext Create()
    {
        return {
            .provider           = &mTestProvider,
            .storage            = &mTestStorage,
            .interactionContext = &mTestContext,

        };
    };

    LogOnlyEvents & EventsGenerator() { return mTestEventsGenerator; }
    TestProviderChangeListener & ChangeListener() { return mTestDataModelChangeListener; }
    TestPersistentStorageDelegate & StorageDelegate() { return mTestStorage; }
    app::DataModel::InteractionModelContext & ImContext() { return mTestContext; }

private:
    NullActionContext mNullActionContext;
    LogOnlyEvents mTestEventsGenerator;
    TestProviderChangeListener mTestDataModelChangeListener;
    EmptyProvider mTestProvider;
    TestPersistentStorageDelegate mTestStorage;

    app::DataModel::InteractionModelContext mTestContext;

    app::ServerClusterContext mContext;
};

} // namespace Test
} // namespace chip
