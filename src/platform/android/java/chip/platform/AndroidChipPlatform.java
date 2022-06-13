/*
 *   Copyright (c) 2021-2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package chip.platform;

public final class AndroidChipPlatform {
  private BleManager mBleManager = null;

  public AndroidChipPlatform(
      BleManager ble,
      KeyValueStoreManager kvm,
      ConfigurationManager cfg,
      ServiceResolver resolver,
      ServiceBrowser browser,
      ChipMdnsCallback chipMdnsCallback,
      DiagnosticDataProvider dataProvider) {
    // Order is important here: initChipStack() initializes the BLEManagerImpl, which depends on the
    // BLEManager being set.
    setBLEManager(ble);
    setKeyValueStoreManager(kvm);
    setConfigurationManager(cfg);
    setDnssdDelegates(resolver, browser, chipMdnsCallback);
    setDiagnosticDataProviderManager(dataProvider);
    initChipStack();
  }

  // for BLEManager
  public BleManager getBLEManager() {
    return mBleManager;
  }

  private void setBLEManager(BleManager manager) {
    if (manager != null) {
      mBleManager = manager;
      manager.setAndroidChipPlatform(this);
      nativeSetBLEManager(manager);
    }
  }

  private native void nativeSetBLEManager(BleManager manager);

  // apis in BleLayer.h called by Platform
  // write success
  public native void handleWriteConfirmation(
      int connId, byte[] svcId, byte[] charId, boolean success);

  // onSubscribeCharacteristic get data
  public native void handleIndicationReceived(int connId, byte[] svcId, byte[] charId, byte[] data);

  // Subscribe success
  public native void handleSubscribeComplete(
      int connId, byte[] svcId, byte[] charId, boolean success);

  // Unsubscribe success
  public native void handleUnsubscribeComplete(
      int connId, byte[] svcId, byte[] charId, boolean success);

  // connection status changed
  public native void handleConnectionError(int connId);

  // for KeyValueStoreManager
  private native void setKeyValueStoreManager(KeyValueStoreManager manager);

  // for ConfigurationManager
  private native void setConfigurationManager(ConfigurationManager manager);

  /** Initialize the CHIP stack. */
  private native void initChipStack();

  // for DnssdDelegates
  private void setDnssdDelegates(
      ServiceResolver resolver, ServiceBrowser browser, ChipMdnsCallback chipMdnsCallback) {
    if (resolver != null) {
      nativeSetDnssdDelegates(resolver, browser, chipMdnsCallback);
    }
  }

  private native void nativeSetDnssdDelegates(
      ServiceResolver resolver, ServiceBrowser browser, ChipMdnsCallback chipMdnsCallback);

  private native void setDiagnosticDataProviderManager(DiagnosticDataProvider dataProviderCallback);

  /**
   * update commission info
   *
   * @param spake2pVerifierBase64 base64 encoded spake2p verifier, ref
   *     CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER. using null to generate it from passcode.
   * @param Spake2pSaltBase64 base64 encoded spake2p salt, ref
   *     CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT. using null to generate a random one.
   * @param spake2pIterationCount Spake2p iteration count, or 0 to use
   *     CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
   * @return true on success of false on failed
   */
  public native boolean updateCommissionableDataProviderData(
      String spake2pVerifierBase64,
      String Spake2pSaltBase64,
      int spake2pIterationCount,
      long setupPasscode,
      int discriminator);
}
