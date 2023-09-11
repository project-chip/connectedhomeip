/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** BluetoothGattCallback.java Stub file to allow compiling standalone, without Android SDK. */
package android.bluetooth;

// Stub class to allow standalone compilation without Android
public abstract class BluetoothGattCallback {
  public void onCharacteristicChanged(
      BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {}

  public void onCharacteristicRead(
      BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {}

  public void onCharacteristicWrite(
      BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {}

  public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {}

  public void onDescriptorRead(
      BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {}

  public void onDescriptorWrite(
      BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {}

  public void onServicesDiscovered(BluetoothGatt gatt, int status) {}
}
