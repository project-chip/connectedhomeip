/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;

public interface BleManager {
  // For app
  int addConnection(BluetoothGatt bleGatt);

  BluetoothGatt removeConnection(int connId);

  BluetoothGatt getConnection(int connId);

  void setBleCallback(BleCallback bleCallback);

  BluetoothGattCallback getCallback();

  void setAndroidChipPlatform(AndroidChipPlatform platform);

  // BLEManager
  int init();

  long setFlag(long flag, boolean isSet);

  boolean hasFlag(long flag);

  // BlePlatformDelegate
  boolean onSubscribeCharacteristic(int connId, byte[] svcId, byte[] charId);

  boolean onUnsubscribeCharacteristic(int connId, byte[] svcId, byte[] charId);

  boolean onCloseConnection(int connId);

  int onGetMTU(int connId);

  boolean onSendWriteRequest(int connId, byte[] svcId, byte[] charId, byte[] characteristicData);

  // BleApplicationDelegate
  void onNotifyChipConnectionClosed(int connId);

  // BleConnectionDelegate
  void onNewConnection(
      int discriminator, boolean isShortDiscriminator, long implPtr, long appStatePtr);
}
