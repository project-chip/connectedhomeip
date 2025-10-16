/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

namespace chip {
namespace Test {

constexpr uint16_t kMaxGroupsPerFabric    = 5;
constexpr uint16_t kMaxGroupKeysPerFabric = 8;

chip::Credentials::GroupDataProviderImpl sProvider(kMaxGroupsPerFabric, kMaxGroupKeysPerFabric);
chip::SimpleSessionResumptionStorage sessionStorage;

// To prevent redundancy, the Engine_raii Class was created
// Before using the engine, it is necessary to initialize it.
class Engine_raii
{
public:
    Engine_raii() : engine{ chip::app::InteractionModelEngine::GetInstance() } {}

    ~Engine_raii() { engine->Shutdown(); }

    chip::app::InteractionModelEngine * operator->() { return engine; }

private:
    chip::app::InteractionModelEngine * engine;
};

// Fabric Init Params Class define the basic arguments to device controller factory
class FactoryInitParamsSetter
{
public:
    FactoryInitParamsSetter()
    {
        // Set all the basic requirement to test DeviceControllerFactory
        EXPECT_EQ(opCertStore.Init(&cerStorage), CHIP_NO_ERROR);

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
} // namespace Test
} // namespace chip
