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

#include <app/InteractionModelEngine.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/util/DataModelHandler.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CHIPDeviceControllerSystemState.h>
#include <controller/tests/DispatchDataModel.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/support/TestPersistentStorageDelegate.h>

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;
using TestSessionKeystoreImpl = chip::Crypto::DefaultSessionKeystore;

namespace {

// Fabric Table Holder Class handle a Factory Table class to test
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

    chip::FabricTable & GetFabricTable() { return mFabricTable; }

private:
    chip::FabricTable mFabricTable;
    chip::TestPersistentStorageDelegate mStorage;
    chip::PersistentStorageOperationalKeystore mOpKeyStore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
};

// Test DeviceControllerFactory Class
class TestDeviceControllerFactory : public chip::Test::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();
        mOldProvider = chip::app::InteractionModelEngine::GetInstance()->SetDataModelProvider(
            &chip::TestDataModel::DispatchTestDataModel::Instance());
        factoryInitParams.listenPort               = 88;
        factoryInitParams.fabricTable              = nullptr;
        factoryInitParams.fabricIndependentStorage = &factoryStorage;
    }
    void TearDown() override
    {
        chip::app::InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
        chip::Credentials::GroupDataProvider * provider = chip::Credentials::GetGroupDataProvider();
        if (provider != nullptr)
        {
            provider->Finish();
        }
        AppContext::TearDown();
    }

protected:
    TestSessionKeystoreImpl keystore;
    chip::TestPersistentStorageDelegate factoryStorage;
    chip::Controller::FactoryInitParams factoryInitParams;

private:
    chip::app::DataModel::Provider * mOldProvider = nullptr;
};

// Test all methods of DeviceControllerFactory with four different tests.
TEST_F(TestDeviceControllerFactory, DeviceControllerFactoryMethods)
{
    // Set all the basic requirement to test DeviceControllerFactory
    chip::Controller::DeviceController device;
    chip::Controller::DeviceCommissioner commissioner;
    chip::Controller::SetupParams dParams;

    chip::TestPersistentStorageDelegate cerStorage;

    chip::Credentials::PersistentStorageOpCertStore opCertStore;
    EXPECT_EQ(opCertStore.Init(&cerStorage), CHIP_NO_ERROR);
    chip::TestPersistentStorageDelegate storage;
    chip::SimpleSessionResumptionStorage sessionStorage;
    FabricTableHolder fHolder;
    chip::TestPersistentStorageDelegate sStorageDelegate;

    chip::Credentials::GroupDataProviderImpl sProvider(5, 8);
    sessionStorage.Init(&storage);

    // Initialize Group Data Provider
    sProvider.SetStorageDelegate(&sStorageDelegate);
    sProvider.SetSessionKeystore(&keystore);
    sProvider.Init();
    chip::Credentials::SetGroupDataProvider(&sProvider);

    EXPECT_EQ(fHolder.Init(), CHIP_NO_ERROR);

    // Initialize the ember side server logic
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), chip::app::reporting::GetDefaultReportScheduler()),
              CHIP_NO_ERROR);

    // Set initials params of factoryInitParams
    factoryInitParams.sessionKeystore   = &keystore;
    factoryInitParams.groupDataProvider = &sProvider;
    factoryInitParams.opCertStore       = &opCertStore;

    //
    // Test DeviceControllerFactory init fails to not define DataModel
    //
    {
        // Expect an init method failure when the data model is not initialized to test print log error.
        CHIP_ERROR err = DeviceControllerFactory::GetInstance().Init(factoryInitParams);
        EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Init device controller factory
    factoryInitParams.dataModelProvider = engine->GetDataModelProvider();

    //
    // Test success init DeviceControllerFactory without fabricTable
    //
    {
        CHIP_ERROR err = DeviceControllerFactory::GetInstance().Init(factoryInitParams);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        DeviceControllerFactory::GetInstance().Shutdown();
    }

    factoryInitParams.fabricTable              = &fHolder.GetFabricTable();
    factoryInitParams.sessionResumptionStorage = &sessionStorage;
    factoryInitParams.enableServerInteractions = true;

    //
    // Test SetupController and SetupCommissioner
    //
    {
        CHIP_ERROR err = DeviceControllerFactory::GetInstance().Init(factoryInitParams);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = DeviceControllerFactory::GetInstance().Init(factoryInitParams);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        dParams.controllerVendorId = chip::VendorId::TestVendor1;
        EXPECT_EQ(DeviceControllerFactory::GetInstance().SetupController(dParams, device), CHIP_NO_ERROR);
        EXPECT_EQ(DeviceControllerFactory::GetInstance().SetupCommissioner(dParams, commissioner), CHIP_ERROR_INVALID_ARGUMENT);

        EXPECT_TRUE(DeviceControllerFactory::GetInstance().ReleaseSystemState());
        DeviceControllerFactory::GetInstance().Shutdown();
    }

    //
    // Test retain and release system state
    //
    {
        CHIP_ERROR err = DeviceControllerFactory::GetInstance().Init(factoryInitParams);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        DeviceControllerFactory::GetInstance().RetainSystemState();
        DeviceControllerFactory::GetInstance().RetainSystemState();
        EXPECT_FALSE(DeviceControllerFactory::GetInstance().GetSystemState()->IsShutDown());
        EXPECT_FALSE(DeviceControllerFactory::GetInstance().ReleaseSystemState());
        EXPECT_TRUE(DeviceControllerFactory::GetInstance().ReleaseSystemState());
        EXPECT_TRUE(DeviceControllerFactory::GetInstance().GetSystemState()->IsShutDown());

        // Reinit system state
        EXPECT_EQ(DeviceControllerFactory::GetInstance().EnsureAndRetainSystemState(), CHIP_NO_ERROR);
        EXPECT_TRUE(DeviceControllerFactory::GetInstance().ReleaseSystemState());
        DeviceControllerFactory::GetInstance().Shutdown();
    }
    // Free opCertStore and engine before finish
    opCertStore.Finish();
    engine->Shutdown();
}

// Add more test cases as needed to cover different scenarios
} // namespace
