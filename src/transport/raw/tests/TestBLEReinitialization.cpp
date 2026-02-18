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

/**
 *    @file
 *      This file implements unit tests for the BLE transport re-initialization functionality.
 */

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <transport/raw/BLE.h>

#if CONFIG_NETWORK_LAYER_BLE

namespace chip {
namespace Transport {
namespace {

// Mock BLE layer for testing
class MockBleLayer : public Ble::BleLayer
{
public:
    MockBleLayer() {}
};

// Test fixture for BLE re-initialization tests
class TestBLEReinitialization : public ::testing::Test
{
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}

protected:
    void SetUp() override
    {
        mBleLayer = new MockBleLayer();
        ASSERT_NE(mBleLayer, nullptr);
    }

    void TearDown() override
    {
        if (mBleLayer != nullptr)
        {
            delete mBleLayer;
            mBleLayer = nullptr;
        }
    }

    MockBleLayer * mBleLayer = nullptr;
};

// Test basic BLE initialization
TEST_F(TestBLEReinitialization, TestBasicInitialization)
{
    BLE<1> bleTransport;
    BleListenParameters params(mBleLayer);

    CHIP_ERROR err = bleTransport.Init(params);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify transport was set on the BLE layer
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport);
}

// Test re-initialization with PreserveExistingBleLayerTransport = false (default)
TEST_F(TestBLEReinitialization, TestReinitializationOverridesTransport)
{
    BLE<1> bleTransport1;
    BLE<1> bleTransport2;

    // First initialization
    BleListenParameters params1(mBleLayer);
    CHIP_ERROR err = bleTransport1.Init(params1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport1);

    // Second initialization should override the transport
    BleListenParameters params2(mBleLayer);
    params2.SetPreserveExistingBleLayerTransport(false);
    err = bleTransport2.Init(params2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport2);
}

// Test re-initialization with PreserveExistingBleLayerTransport = true
TEST_F(TestBLEReinitialization, TestReinitializationPreservesTransport)
{
    BLE<1> bleTransport1;
    BLE<1> bleTransport2;

    // First initialization
    BleListenParameters params1(mBleLayer);
    CHIP_ERROR err = bleTransport1.Init(params1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport1);

    // Second initialization with preserve flag should NOT override
    BleListenParameters params2(mBleLayer);
    params2.SetPreserveExistingBleLayerTransport(true);
    err = bleTransport2.Init(params2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport1); // Should still be first transport
}

// Test re-initialization when no existing transport
TEST_F(TestBLEReinitialization, TestReinitializationNoExistingTransport)
{
    BLE<1> bleTransport;

    // Initialize with preserve flag when no existing transport
    BleListenParameters params(mBleLayer);
    params.SetPreserveExistingBleLayerTransport(true);

    CHIP_ERROR err = bleTransport.Init(params);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Should set the transport even with preserve flag since there was none
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport);
}

// Test multiple re-initializations
TEST_F(TestBLEReinitialization, TestMultipleReinitializations)
{
    BLE<1> bleTransport1;
    BLE<1> bleTransport2;
    BLE<1> bleTransport3;

    // First initialization
    BleListenParameters params1(mBleLayer);
    EXPECT_EQ(bleTransport1.Init(params1), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport1);

    // Second initialization (override)
    BleListenParameters params2(mBleLayer);
    params2.SetPreserveExistingBleLayerTransport(false);
    EXPECT_EQ(bleTransport2.Init(params2), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport2);

    // Third initialization (preserve)
    BleListenParameters params3(mBleLayer);
    params3.SetPreserveExistingBleLayerTransport(true);
    EXPECT_EQ(bleTransport3.Init(params3), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport2); // Should still be second
}

// Test initialization with null BLE layer
TEST_F(TestBLEReinitialization, TestInitializationWithNullBleLayer)
{
    BLE<1> bleTransport;
    BleListenParameters params(nullptr);

    CHIP_ERROR err = bleTransport.Init(params);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
}

// Test re-initialization after close
TEST_F(TestBLEReinitialization, TestReinitializationAfterClose)
{
    BLE<1> bleTransport;

    // First initialization
    BleListenParameters params1(mBleLayer);
    EXPECT_EQ(bleTransport.Init(params1), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport);

    // Close the transport
    bleTransport.Close();
    EXPECT_EQ(mBleLayer->mBleTransport, nullptr);

    // Re-initialize should work
    BleListenParameters params2(mBleLayer);
    EXPECT_EQ(bleTransport.Init(params2), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport);
}

// Test double initialization without close (error case)
TEST_F(TestBLEReinitialization, TestDoubleInitializationSameTransport)
{
    BLE<1> bleTransport;

    // First initialization
    BleListenParameters params1(mBleLayer);
    EXPECT_EQ(bleTransport.Init(params1), CHIP_NO_ERROR);

    // Second initialization of same transport should fail
    BleListenParameters params2(mBleLayer);
    CHIP_ERROR err = bleTransport.Init(params2);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
}

// Test that ClearState is called on close
TEST_F(TestBLEReinitialization, TestClearStateOnClose)
{
    BLE<1> bleTransport;

    BleListenParameters params(mBleLayer);
    EXPECT_EQ(bleTransport.Init(params), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport);

    bleTransport.Close();

    // Verify transport was cleared
    EXPECT_EQ(mBleLayer->mBleTransport, nullptr);

    // Note: CancelBleIncompleteConnection is called internally but we can't easily
    // track it without modifying the BleLayer implementation. The important thing
    // is that the transport reference is cleared, which we verify above.
}

// Test preserve flag behavior with sequential initializations
TEST_F(TestBLEReinitialization, TestPreserveFlagSequence)
{
    BLE<1> bleTransport1;
    BLE<1> bleTransport2;
    BLE<1> bleTransport3;

    // Init 1: Set transport
    BleListenParameters params1(mBleLayer);
    EXPECT_EQ(bleTransport1.Init(params1), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport1);

    // Init 2: Preserve (should keep transport1)
    BleListenParameters params2(mBleLayer);
    params2.SetPreserveExistingBleLayerTransport(true);
    EXPECT_EQ(bleTransport2.Init(params2), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport1);

    // Init 3: Override (should set transport3)
    BleListenParameters params3(mBleLayer);
    params3.SetPreserveExistingBleLayerTransport(false);
    EXPECT_EQ(bleTransport3.Init(params3), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport3);
}

// Test that state is properly managed across re-initializations
TEST_F(TestBLEReinitialization, TestStateManagement)
{
    BLE<1> bleTransport1;
    BLE<1> bleTransport2;

    // First initialization
    BleListenParameters params1(mBleLayer);
    EXPECT_EQ(bleTransport1.Init(params1), CHIP_NO_ERROR);

    // Close first transport
    bleTransport1.Close();

    // Second initialization should work independently
    BleListenParameters params2(mBleLayer);
    EXPECT_EQ(bleTransport2.Init(params2), CHIP_NO_ERROR);
    EXPECT_EQ(mBleLayer->mBleTransport, &bleTransport2);

    // Close second transport
    bleTransport2.Close();
    EXPECT_EQ(mBleLayer->mBleTransport, nullptr);
}

} // namespace
} // namespace Transport
} // namespace chip

#endif // CONFIG_NETWORK_LAYER_BLE
