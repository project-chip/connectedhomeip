/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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

/** BluetoothGatt.java Stub file to allow compiling standalone, without Android SDK. */
package android.bluetooth;

import java.util.List;
import java.util.UUID;

// Stub class to allow standalone compilation without Android
public final class BluetoothGatt {

  public static final int GATT_SUCCESS = 0;

  public void close() {}

  public BluetoothGattService getService(UUID uuid) {
    return null;
  }

  public List<BluetoothGattService> getServices() {
    return null;
  }

  public boolean setCharacteristicNotification(
      BluetoothGattCharacteristic characteristic, boolean enable) {
    return false;
  }

  public boolean writeCharacteristic(BluetoothGattCharacteristic characteristic) {
    return false;
  }

  public boolean writeDescriptor(BluetoothGattDescriptor descriptor) {
    return false;
  }
}
