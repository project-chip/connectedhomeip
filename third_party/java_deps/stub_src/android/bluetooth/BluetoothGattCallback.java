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
