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

  public String getIpAddress() {
    return getIpAddress(deviceControllerPtr);
  }

  public void beginSendMessage(String message) {
    beginSendMessage(deviceControllerPtr, message);
  }

  public void beginSendCommand(ChipCommandType command, int value) {
    beginSendCommand(deviceControllerPtr, command, value);
  }

  public void sendWiFiCredentials(String ssid, String password) {
    sendWiFiCredentials(deviceControllerPtr, ssid, password);
  }

  public void sendThreadCredentials(int channel, int panId, byte[] xpanId, byte[] masterKey) {
    sendThreadCredentials(deviceControllerPtr, channel, panId, xpanId, masterKey);
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

  public void onNetworkCredentialsRequested() {
    if (completionListener != null) {
      completionListener.onNetworkCredentialsRequested();
    }
  }

  public void onOperationalCredentialsRequested(byte[] csr) {
    if (completionListener != null) {
      completionListener.onOperationalCredentialsRequested(csr);
    }
  }

  public void onStatusUpdate(int status) {
    if (completionListener != null) {
      completionListener.onStatusUpdate(status);
    }
  }

  public void onPairingComplete(int errorCode) {
    if (completionListener != null) {
      completionListener.onPairingComplete(errorCode);
    }
  }

  public void onPairingDeleted(int errorCode) {
    if (completionListener != null) {
      completionListener.onPairingDeleted(errorCode);
    }
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

  public void close() {
    releaseBluetoothGatt(connectionId);
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

  private native String getIpAddress(long deviceControllerPtr);

  private native void beginSendMessage(long deviceControllerPtr, String message);

  private native void beginSendCommand(
      long deviceControllerPtr, ChipCommandType command, int value);

  private native void sendWiFiCredentials(long deviceControllerPtr, String ssid, String password);

  private native void sendThreadCredentials(
      long deviceControllerPtr, int channel, int panId, byte[] xpanId, byte[] masterKey);

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

    /** Notifies that the device is ready to receive Wi-Fi network credentials. */
    void onNetworkCredentialsRequested();

    /** Notifies that the device is ready to receive operational credentials. */
    void onOperationalCredentialsRequested(byte[] csr);

    /** Notifies the pairing status. */
    void onStatusUpdate(int status);

    /** Notifies the completion of pairing. */
    void onPairingComplete(int errorCode);

    /** Notifies the deletion of pairing session. */
    void onPairingDeleted(int errorCode);

    /** Notifies that the Chip connection has been closed. */
    void onNotifyChipConnectionClosed();

    /** Notifies the completion of the "close BLE connection" command. */
    void onCloseBleComplete();

    /** Notifies the listener of the error. */
    void onError(Throwable error);
  }
}
