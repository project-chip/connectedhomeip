/*
 *   Copyright (c) 2020 Project CHIP Authors
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
  private static final String TAG = AndroidChipPlatform.class.getSimpleName();
  private static AndroidChipPlatform sInstance = new AndroidChipPlatform();
  private BLEManager mBLEManager = null;

  private AndroidChipPlatform() {
  }

  public static AndroidChipPlatform getInstance() {
    return sInstance;
  }

  //for BLEManager
  public BLEManager getBLEManager() {
    return mBLEManager;
  }

  public void setBLEManager(BLEManager manager) {
    if(mBLEManager == null) {
      mBLEManager = manager;
      nativeSetBLEManager(manager);
    }
  }

  public native void nativeSetBLEManager(BLEManager manager);

  //apis in BleLayer.h called by Platform
  //write success
  public native void handleWriteConfirmation(
          int connId, byte[] svcId, byte[] charId, boolean success);

  //onSubscribeCharacteristic get data
  public native void handleIndicationReceived(
          int connId, byte[] svcId, byte[] charId, byte[] data);

  //Subscribe success
  public native void handleSubscribeComplete(
          int connId, byte[] svcId, byte[] charId, boolean success);

  //Unsubscribe success
  public native void handleUnsubscribeComplete(
          int connId, byte[] svcId, byte[] charId, boolean success);

  //connection status changed
  public native void handleConnectionError(int connId);


  //for KeyValueStoreManager
  public native void setKeyValueStoreManager(KeyValueStoreManager manager);

  //for ConfigurationManager
  public native void setConfigurationManager(ConfigurationManager manager);

  //for ServiceResolver
  public native void setServiceResolver(ServiceResolver resolver);

  public native void handleServiceResolve(
          String instanceName,
          String serviceType,
          String address,
          int port,
          long callbackHandle,
          long contextHandle);
}
