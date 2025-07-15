/*
 *
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

#include <gtest/gtest.h>

#include <controller/tests/data_model/DataModelFixtures.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/test-interaction-model-api.h>
#include <app/tests/AppTestContext.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CHIPDeviceControllerSystemState.h>
#include <lib/core/CHIPError.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <credentials/GroupDataProviderImpl.h>
#include <app/util/DataModelHandler.h>
#include <messaging/tests/MessagingContext.h>
#include <controller/tests/DispatchDataModel.h>
#include <app/CommandHandlerInterfaceRegistry.h>

using namespace chip;
using namespace chip::Controller;
using namespace chip::app;
using namespace chip::Test;
using namespace chip::TestDataModel;

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;
using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

namespace {

// Just enough init to replace a ton of boilerplate
class FabricTableHolder
{
public:
    FabricTableHolder() {}
    ~FabricTableHolder()
    {
        mFabricTable.Shutdown();
        mOpKeyStore.Finish();
        mOpCertStore.Finish();
    }

    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(mOpKeyStore.Init(&mStorage));
        ReturnErrorOnFailure(mOpCertStore.Init(&mStorage));

        chip::FabricTable::InitParams initParams;
        initParams.storage             = &mStorage;
        initParams.operationalKeystore = &mOpKeyStore;
        initParams.opCertStore         = &mOpCertStore;

        return mFabricTable.Init(initParams);
    }

    FabricTable & GetFabricTable() { return mFabricTable; }

private:
    chip::FabricTable mFabricTable;
    chip::TestPersistentStorageDelegate mStorage;
    chip::PersistentStorageOperationalKeystore mOpKeyStore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
};

/********************************************************************************************/
class TestDeviceController : public chip::Test::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();

        mOldProvider = InteractionModelEngine::GetInstance()->SetDataModelProvider(&DispatchTestDataModel::Instance());
    }
    void TearDown() override
    {
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
        chip::Credentials::GroupDataProvider * provider = chip::Credentials::GetGroupDataProvider();
        if (provider != nullptr)
        {
            provider->Finish();
        }
        AppContext::TearDown();
    }

protected:
    TestSessionKeystoreImpl keystore;
    chip::Controller::FactoryInitParams factoryInitParams;
private:
    chip::app::DataModel::Provider * mOldProvider = nullptr;
};

TEST_F(TestDeviceController, DeviceCommissioner_)
{
    chip::TestPersistentStorageDelegate mStorage;
    chip::TestPersistentStorageDelegate storage;
    chip::SimpleSessionResumptionStorage sessionStorage;
    FabricTableHolder fHolder;
    chip::TestPersistentStorageDelegate sStorageDelegate;
    
    Credentials::GroupDataProviderImpl sProvider(5, 8);
    sessionStorage.Init(&storage);

    // Initialize Group Data Provider
    sProvider.SetStorageDelegate(&sStorageDelegate);
    sProvider.SetSessionKeystore(&keystore);
    // sProvider.SetListener(&chip::app::TestGroups::sListener);
    sProvider.Init();
    Credentials::SetGroupDataProvider(&sProvider);

    EXPECT_EQ(fHolder.Init(), CHIP_NO_ERROR);
    // Initialize the ember side server logic
        ChipLogProgress(NotSpecified, "otro init");

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);
    factoryInitParams.listenPort = 88;
    factoryInitParams.dataModelProvider = engine->GetDataModelProvider();
    factoryInitParams.fabricTable = &fHolder.GetFabricTable();
    factoryInitParams.sessionKeystore = &keystore;
    factoryInitParams.fabricIndependentStorage = &mStorage;
    factoryInitParams.sessionResumptionStorage = &sessionStorage;
    factoryInitParams.groupDataProvider = &sProvider;
        ChipLogProgress(NotSpecified, "otro init");

    CHIP_ERROR err = DeviceControllerFactory::GetInstance().Init(factoryInitParams);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    engine->Shutdown();
}

// Add more test cases as needed to cover different scenarios
}
