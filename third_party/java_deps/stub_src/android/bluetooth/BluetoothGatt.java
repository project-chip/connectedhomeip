/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
