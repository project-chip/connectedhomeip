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

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include <wifipaf/WiFiPAFLayer.h>
#endif

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;
using TestSessionKeystoreImpl = chip::Crypto::DefaultSessionKeystore;

chip::Credentials::GroupDataProviderImpl sProvider(5, 8);

namespace {

class Engine_raii {
public:
    Engine_raii(): engine{ chip::app::InteractionModelEngine::GetInstance() } {}

    ~Engine_raii() { engine->Shutdown(); }

    chip::app::InteractionModelEngine * operator->() { return engine; }

private:
    chip::app::InteractionModelEngine * engine;     
};

class FactoryInitParamsSetter
{
public:
    FactoryInitParamsSetter()
    {
        // Set all the basic requirement to test DeviceControllerFactory
        ASSERT_EQ(opCertStore.Init(&cerStorage), CHIP_NO_ERROR);

        // Initialize Group Data Provider
        sProvider.SetStorageDelegate(&sStorageDelegate);
        sProvider.SetSessionKeystore(&keystore);
        sProvider.Init();
        chip::Credentials::SetGroupDataProvider(&sProvider);

        // Set initials params of factoryInitParams
        factoryInitParams.sessionKeystore          = &keystore;
        factoryInitParams.groupDataProvider        = &sProvider;
        factoryInitParams.opCertStore              = &opCertStore;
        factoryInitParams.listenPort               = 88;
        factoryInitParams.fabricTable              = nullptr;
        factoryInitParams.fabricIndependentStorage = &factoryStorage;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        factoryInitParams.wifipaf_layer = &wifipaf;
#endif
    };
    ~FactoryInitParamsSetter()
    {
        sProvider.Finish();
        opCertStore.Finish();
    };
    chip::Controller::FactoryInitParams GetFactoryInitParams() { return factoryInitParams; };

protected:
    chip::TestPersistentStorageDelegate cerStorage;
    chip::Credentials::PersistentStorageOpCertStore opCertStore;
    chip::TestPersistentStorageDelegate sStorageDelegate;
    TestSessionKeystoreImpl keystore;
    chip::TestPersistentStorageDelegate factoryStorage;
    chip::Controller::FactoryInitParams factoryInitParams;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    chip::WiFiPAF::WiFiPAFLayer wifipaf;
#endif
};

// Fabric Table Holder Class handle a Factory Table class to test
class FabricTableHolder
{
public:
    FabricTableHolder() {}
    ~FabricTableHolder()
    {
        mFabricTable.Shutdown();
        ChipLogProgress(Controller, "Shutting down Keystore");
        mOpKeyStore.Finish();
        ChipLogProgress(Controller, "Shutting down CertStore");
        mOpCertStore.Finish();
        ChipLogProgress(Controller, "All processes are closed");
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
    FactoryInitParamsSetter params;

private:
    chip::app::DataModel::Provider * mOldProvider = nullptr;
};

TEST_F(TestDeviceControllerFactory, DeviceControllerFactoryMethods_FailInit)
{
    chip::Controller::FactoryInitParams factoryInitParams = params.GetFactoryInitParams();
    // Initialize the ember side server logic
    Engine_raii engine;
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), chip::app::reporting::GetDefaultReportScheduler()),
              CHIP_NO_ERROR);

    // Expect an init method failure when the data model is not initialized to test print log error.
    CHIP_ERROR err = DeviceControllerFactory::GetInstance().Init(factoryInitParams);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);

    // Init device controller factory
    factoryInitParams.dataModelProvider = engine->GetDataModelProvider();
} // DeviceControllerFactoryMethods_FailInit

TEST_F(TestDeviceControllerFactory, DeviceControllerFactoryMethods_DobleInit)
{
    chip::Controller::FactoryInitParams factoryInitParams = params.GetFactoryInitParams();
    // Initialize the ember side server logic
    Engine_raii engine;
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), chip::app::reporting::GetDefaultReportScheduler()),
              CHIP_NO_ERROR);

    // Init device controller factory
    factoryInitParams.dataModelProvider = engine->GetDataModelProvider();

    // Init success without fabricTable
    EXPECT_EQ(DeviceControllerFactory::GetInstance().Init(factoryInitParams), CHIP_NO_ERROR);
    EXPECT_EQ(DeviceControllerFactory::GetInstance().Init(factoryInitParams), CHIP_NO_ERROR);
    DeviceControllerFactory::GetInstance().Shutdown();
} // DeviceControllerFactoryMethods_DobleInit

TEST_F(TestDeviceControllerFactory, DeviceControllerFactoryMethods_SetupControllerAndCommissioner)
{
    chip::Controller::FactoryInitParams factoryInitParams = params.GetFactoryInitParams();
    chip::TestPersistentStorageDelegate storage;
    chip::Controller::SetupParams deviceParams;
    chip::Controller::DeviceCommissioner commissioner;
    chip::Controller::DeviceController device;
    // Initialize the ember side server logic
    Engine_raii engine;
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), chip::app::reporting::GetDefaultReportScheduler()),
              CHIP_NO_ERROR);

    // Init device controller factory
    factoryInitParams.dataModelProvider = engine->GetDataModelProvider();

    chip::SimpleSessionResumptionStorage sessionStorage;
    FabricTableHolder fHolder;

    EXPECT_EQ(sessionStorage.Init(&storage), CHIP_NO_ERROR);
    EXPECT_EQ(fHolder.Init(), CHIP_NO_ERROR);

    factoryInitParams.fabricTable              = &fHolder.GetFabricTable();
    factoryInitParams.sessionResumptionStorage = &sessionStorage;
    factoryInitParams.enableServerInteractions = true;

    EXPECT_EQ(DeviceControllerFactory::GetInstance().Init(factoryInitParams), CHIP_NO_ERROR);

    deviceParams.controllerVendorId = chip::VendorId::TestVendor1;
    EXPECT_EQ(DeviceControllerFactory::GetInstance().SetupController(deviceParams, device), CHIP_NO_ERROR);
    // SetupCommissioner is expected to fail because deviceParams does not have a pairingDelegate,
    // which is required for a commissioner.
    EXPECT_EQ(DeviceControllerFactory::GetInstance().SetupCommissioner(deviceParams, commissioner), CHIP_ERROR_INVALID_ARGUMENT);

    EXPECT_TRUE(DeviceControllerFactory::GetInstance().ReleaseSystemState());
    DeviceControllerFactory::GetInstance().Shutdown();
} // DeviceControllerFactoryMethods_SetupControllerAndCommissioner

TEST_F(TestDeviceControllerFactory, DeviceControllerFactoryMethods_RetainAndRelease)
{
    chip::Controller::FactoryInitParams factoryInitParams = params.GetFactoryInitParams();
    chip::TestPersistentStorageDelegate storage;
    chip::Controller::SetupParams dparams;
    // Initialize the ember side server logic
    Engine_raii engine;
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), chip::app::reporting::GetDefaultReportScheduler()),
              CHIP_NO_ERROR);

    // Init device controller factory
    factoryInitParams.dataModelProvider = engine->GetDataModelProvider();

    chip::SimpleSessionResumptionStorage sessionStorage;
    FabricTableHolder fHolder;

    EXPECT_EQ(sessionStorage.Init(&storage), CHIP_NO_ERROR);
    EXPECT_EQ(fHolder.Init(), CHIP_NO_ERROR);

    factoryInitParams.fabricTable              = &fHolder.GetFabricTable();
    factoryInitParams.sessionResumptionStorage = &sessionStorage;
    factoryInitParams.enableServerInteractions = true;

    EXPECT_EQ(DeviceControllerFactory::GetInstance().Init(factoryInitParams), CHIP_NO_ERROR);

    // Test retain and release system state
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
} // DeviceControllerFactoryMethods_RetainAndRelease

} // namespace
