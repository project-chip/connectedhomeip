/*
 *   Copyright (c) 2021 Project CHIP Authors
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

import android.content.Context;

public final class AndroidChipPlatform {
  private BleManager mBleManager = null;

  /** Constructs AndroidChipPlatform with default implementations for all dependencies. */
  public static AndroidChipPlatform defaultInstance(Context context) {
    return new AndroidChipPlatform(
        new AndroidBleManager(),
        new PreferencesKeyValueStoreManager(context),
        new PreferencesConfigurationManager(context),
        new NsdManagerServiceResolver(context),
        new NsdManagerServiceBrowser(context),
        new ChipMdnsCallbackImpl());
  }

  public static AndroidChipPlatform newInstance(
      BleManager ble,
      KeyValueStoreManager kvm,
      ConfigurationManager cfg,
      ServiceResolver resolver,
      ServiceBrowser browser,
      ChipMdnsCallback chipMdnsCallback) {
    return new AndroidChipPlatform(ble, kvm, cfg, resolver, browser, chipMdnsCallback);
  }

  private AndroidChipPlatform(
      BleManager ble,
      KeyValueStoreManager kvm,
      ConfigurationManager cfg,
      ServiceResolver resolver,
      ServiceBrowser browser,
      ChipMdnsCallback chipMdnsCallback) {
    setBLEManager(ble);
    setKeyValueStoreManager(kvm);
    setConfigurationManager(cfg);
    setDnssdDelegates(resolver, browser, chipMdnsCallback);
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

  // for ServiceResolver
  private void setDnssdDelegates(
      ServiceResolver resolver, ServiceBrowser browser, ChipMdnsCallback chipMdnsCallback) {
    if (resolver != null) {
      nativeSetDnssdDelegates(resolver, browser, chipMdnsCallback);
    }
  }

  private native void nativeSetDnssdDelegates(
      ServiceResolver resolver, ServiceBrowser browser, ChipMdnsCallback chipMdnsCallback);
}
