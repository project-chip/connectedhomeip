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
package chip.devicecontroller;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothGattDescriptor;
import java.util.UUID;
import android.util.Log;

/** Controller to interact with the CHIP device. */
public class ChipDeviceController {
  private static final String TAG = ChipDeviceController.class.getSimpleName();

  private long deviceControllerPtr;
  private int connectionId;
  private BluetoothGatt bleGatt;
  private CompletionListener completionListener;

  public ChipDeviceController() {
    deviceControllerPtr = newDeviceController();
  }

  public void setCompletionListener(CompletionListener listener) {
    completionListener = listener;
  }

  public BluetoothGatt getBluetoothGatt() {
    return bleGatt;
  }

  public BluetoothGattCallback getCallback() {
    return AndroidChipStack.getInstance().getCallback();
  }

  public void beginConnectDeviceBle(BluetoothGatt bleServer, long setupPincode) {
    if (connectionId == 0) {
      bleGatt = bleServer;

      connectionId = AndroidChipStack.getInstance().addConnection(this);
      if (connectionId == 0) {
        Log.e(TAG, "Failed to add Bluetooth connection.");
        completionListener.onError(new Exception("Failed to add Bluetooth connection."));
        return;
      }

      Log.d(TAG, "Bluetooth connection added with ID: " + connectionId);
      beginConnectDevice(deviceControllerPtr, connectionId, setupPincode);
    } else {
      Log.e(TAG, "Bluetooth connection already in use.");
      completionListener.onError(new Exception("Bluetooth connection already in use."));
    }
  }

  public void beginConnectDevice(String ipAddress) {
    beginConnectDeviceIp(deviceControllerPtr, ipAddress);
  }

  public boolean isConnected() {
    return isConnected(deviceControllerPtr);
  }

  public void beginSendMessage(String message) {
    beginSendMessage(deviceControllerPtr, message);
  }

  public void beginSendCommand(ChipCommandType command) {
    beginSendCommand(deviceControllerPtr, command);
  }

  public boolean disconnectDevice() {
    return disconnectDevice(deviceControllerPtr);
  }

  public void onConnectDeviceComplete() {
    completionListener.onConnectDeviceComplete();
  }

  public void onSendMessageComplete(String message) {
    completionListener.onSendMessageComplete(message);
  }

  public void onCharacteristicWrite(BluetoothGatt gatt, String charId, int status) {
    int connId = AndroidChipStack.getInstance().getConnId(gatt);

    if (connId <= 0) {
      Log.e(TAG, "onCharacteristicWrite for non-active connection");
      return;
    }

    if (status != BluetoothGatt.GATT_SUCCESS) {
      return;
    }

    handleWriteConfirmation(connId, charId);
  }

  public void onDescriptorWrite(BluetoothGatt gatt, String charId, byte[] value, int status) {
    int connId = AndroidChipStack.getInstance().getConnId(gatt);

    if (connId <= 0) {
      Log.e(TAG, "onCharacteristicWrite for non-active connection");
      return;
    }

    if (status != BluetoothGatt.GATT_SUCCESS)
      return;

    if (value.equals(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE))
      handleSubscribeComplete(connId, charId);
  }

  public void onCharacteristicChanged(BluetoothGatt gatt, String charId, byte[] value) {
    int connId = AndroidChipStack.getInstance().getConnId(gatt);

    if (connId <= 0) {
      Log.e(TAG, "onCharacteristicChanged for non-active connection");
      return;
    }

    handleIndicationReceived(connId, charId, value);
  }

  public void onNotifyChipConnectionClosed(int connId) {
    // Clear connection state.
    AndroidChipStack.getInstance().removeConnection(connId);
    connectionId = 0;
    bleGatt = null;

    Log.d(TAG, "Calling onNotifyChipConnectionClosed()");
    completionListener.onNotifyChipConnectionClosed();
  }

  public void onCloseBleComplete(int connId) {
    if (releaseBluetoothGatt(connId)) {
      Log.d(TAG, "Calling onCloseBleComplete()");
      completionListener.onCloseBleComplete();
    } else {
      Log.d(TAG, "Skipped calling onCloseBleComplete(). Connection has already been closed.");
    }
  }

  public void onError(Throwable error) {
    completionListener.onError(error);
  }

  private boolean releaseBluetoothGatt(int connId) {
    if (connectionId == 0) {
      return false;
    }

    Log.d(TAG, "Closing GATT and removing connection for " + connId);

    // Close gatt
    bleGatt.close();

    // Clear connection state.
    AndroidChipStack.getInstance().removeConnection(connId);
    connectionId = 0;
    bleGatt = null;
    return true;
  }

  private native long newDeviceController();

  private native void beginConnectDevice(long deviceControllerPtr, int connectionId, long pinCode);

  private native void beginConnectDeviceIp(long deviceControllerPtr, String ipAddress);

  private native boolean isConnected(long deviceControllerPtr);

  private native void beginSendMessage(long deviceControllerPtr, String message);

  private native void beginSendCommand(long deviceControllerPtr, ChipCommandType command);

  private native void handleIndicationReceived(int conn, String charId, byte[] value);

  private native void handleWriteConfirmation(int conn, String charId);

  private native void handleSubscribeComplete(int conn, String charId);

  private native boolean disconnectDevice(long deviceControllerPtr);

  private native void deleteDeviceController(long deviceControllerPtr);

  static {
    System.loadLibrary("CHIPController");
  }

  @SuppressWarnings("deprecation")
  protected void finalize() throws Throwable {
    super.finalize();

    if (deviceControllerPtr != 0) {
      deleteDeviceController(deviceControllerPtr);
      deviceControllerPtr = 0;
    }
  }

  /** Interface to listen for callbacks from CHIPDeviceController. */
  public interface CompletionListener {

    /** Notifies the completion of "ConnectDevice" command. */
    void onConnectDeviceComplete();

    /** Notifies the completion of "SendMessage" echo command. */
    void onSendMessageComplete(String message);

    /** Notifies that the Chip connection has been closed. */
    void onNotifyChipConnectionClosed();

    /** Notifies the completion of the "close BLE connection" command. */
    void onCloseBleComplete();

    /** Notifies the listener of the error. */
    void onError(Throwable error);
  }
}
