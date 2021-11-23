/*
 *   Copyright (c) 2020-2021 Project CHIP Authors
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
import android.util.Log;
import androidx.annotation.Nullable;
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback;

/** Controller to interact with the CHIP device. */
public class ChipDeviceController {
  private static final String TAG = ChipDeviceController.class.getSimpleName();
  private long deviceControllerPtr;
  private int connectionId;
  private CompletionListener completionListener;

  /**
   * To load class and jni, we need to new AndroidChipPlatform after jni load but before new
   * ChipDeviceController
   */
  public static void loadJni() {
    return;
  }

  public ChipDeviceController() {
    deviceControllerPtr = newDeviceController();
  }

  public void setCompletionListener(CompletionListener listener) {
    completionListener = listener;
  }

  public void pairDevice(BluetoothGatt bleServer, int connId, long deviceId, long setupPincode) {
    pairDevice(bleServer, connId, deviceId, setupPincode, null);
  }

  /**
   * Pair a device connected through BLE.
   *
   * @param bleServer the BluetoothGatt representing the BLE connection to the device
   * @param connId the BluetoothGatt Id representing the BLE connection to the device
   * @param deviceId the node ID to assign to the device
   * @param setupPincode the pincode for the device
   * @param csrNonce the 32-byte CSR nonce to use, or null if we want to use an internally randomly
   *     generated CSR nonce.
   */
  public void pairDevice(
      BluetoothGatt bleServer,
      int connId,
      long deviceId,
      long setupPincode,
      @Nullable byte[] csrNonce) {
    if (connectionId == 0) {
      connectionId = connId;

      if (connectionId == 0) {
        Log.e(TAG, "Failed to add Bluetooth connection.");
        completionListener.onError(new Exception("Failed to add Bluetooth connection."));
        return;
      }

      Log.d(TAG, "Bluetooth connection added with ID: " + connectionId);
      Log.d(TAG, "Pairing device with ID: " + deviceId);
      pairDevice(deviceControllerPtr, deviceId, connectionId, setupPincode, csrNonce);
    } else {
      Log.e(TAG, "Bluetooth connection already in use.");
      completionListener.onError(new Exception("Bluetooth connection already in use."));
    }
  }

  public void pairDeviceWithAddress(
      long deviceId,
      String address,
      int port,
      int discriminator,
      long pinCode,
      @Nullable byte[] csrNonce) {
    pairDeviceWithAddress(
        deviceControllerPtr, deviceId, address, port, discriminator, pinCode, csrNonce);
  }

  public void unpairDevice(long deviceId) {
    unpairDevice(deviceControllerPtr, deviceId);
  }

  /**
   * Returns a pointer to a device currently being commissioned. This should be used before the
   * device is operationally available.
   */
  public long getDeviceBeingCommissionedPointer(long nodeId) {
    return getDeviceBeingCommissionedPointer(deviceControllerPtr, nodeId);
  }

  /**
   * Through GetConnectedDeviceCallback, returns a pointer to a connected device or an error.
   *
   * <p>TODO(#8443): This method could benefit from a ChipDevice abstraction to hide the pointer
   * passing.
   */
  public void getConnectedDevicePointer(long nodeId, GetConnectedDeviceCallback callback) {
    GetConnectedDeviceCallbackJni jniCallback = new GetConnectedDeviceCallbackJni(callback);
    getConnectedDevicePointer(deviceControllerPtr, nodeId, jniCallback.getCallbackHandle());
  }

  public boolean disconnectDevice(long deviceId) {
    return disconnectDevice(deviceControllerPtr, deviceId);
  }

  public void onConnectDeviceComplete() {
    completionListener.onConnectDeviceComplete();
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

  public void onCommissioningComplete(long nodeId, int errorCode) {
    if (completionListener != null) {
      completionListener.onCommissioningComplete(nodeId, errorCode);
    }
  }

  public void onOpCSRGenerationComplete(byte[] csr) {
    if (completionListener != null) {
      completionListener.onOpCSRGenerationComplete(csr);
    }
  }

  public void onPairingDeleted(int errorCode) {
    if (completionListener != null) {
      completionListener.onPairingDeleted(errorCode);
    }
  }

  public void onNotifyChipConnectionClosed(int connId) {
    connectionId = 0;

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

    connectionId = 0;
    return true;
  }

  public String getIpAddress(long deviceId) {
    return getIpAddress(deviceControllerPtr, deviceId);
  }

  public long getCompressedFabricId() {
    return getCompressedFabricId(deviceControllerPtr);
  }

  public void updateDevice(long fabricId, long deviceId) {
    updateDevice(deviceControllerPtr, fabricId, deviceId);
  }

  public boolean openPairingWindow(long devicePtr, int duration) {
    return openPairingWindow(deviceControllerPtr, devicePtr, duration);
  }

  public boolean openPairingWindowWithPIN(
      long devicePtr, int duration, int iteration, int discriminator, long setupPinCode) {
    return openPairingWindowWithPIN(
        deviceControllerPtr, devicePtr, duration, iteration, discriminator, setupPinCode);
  }

  public boolean isActive(long deviceId) {
    return isActive(deviceControllerPtr, deviceId);
  }

  /* Shutdown all cluster attribute subscriptions for a given device */
  public void shutdownSubscriptions(long devicePtr) {
    shutdownSubscriptions(deviceControllerPtr, devicePtr);
  }

  /**
   * Generates a new PASE verifier and passcode ID for the given setup PIN code.
   *
   * @param devicePtr a pointer to the device object for which to generate the PASE verifier
   * @param setupPincode the PIN code to use
   * @param iterations the number of iterations for computing the verifier
   * @param salt the 16-byte salt
   */
  public PaseVerifierParams computePaseVerifier(
      long devicePtr, long setupPincode, int iterations, byte[] salt) {
    return computePaseVerifier(deviceControllerPtr, devicePtr, setupPincode, iterations, salt);
  }

  private native PaseVerifierParams computePaseVerifier(
      long deviceControllerPtr, long devicePtr, long setupPincode, int iterations, byte[] salt);

  private native long newDeviceController();

  private native void pairDevice(
      long deviceControllerPtr,
      long deviceId,
      int connectionId,
      long pinCode,
      @Nullable byte[] csrNonce);

  private native void pairDeviceWithAddress(
      long deviceControllerPtr,
      long deviceId,
      String address,
      int port,
      int discriminator,
      long pinCode,
      @Nullable byte[] csrNonce);

  private native void unpairDevice(long deviceControllerPtr, long deviceId);

  private native long getDeviceBeingCommissionedPointer(long deviceControllerPtr, long nodeId);

  private native void getConnectedDevicePointer(
      long deviceControllerPtr, long deviceId, long callbackHandle);

  private native boolean disconnectDevice(long deviceControllerPtr, long deviceId);

  private native void deleteDeviceController(long deviceControllerPtr);

  private native String getIpAddress(long deviceControllerPtr, long deviceId);

  private native long getCompressedFabricId(long deviceControllerPtr);

  private native void updateDevice(long deviceControllerPtr, long fabricId, long deviceId);

  private native boolean openPairingWindow(long deviceControllerPtr, long devicePtr, int duration);

  private native boolean openPairingWindowWithPIN(
      long deviceControllerPtr,
      long devicePtr,
      int duration,
      int iteration,
      int discriminator,
      long setupPinCode);

  private native boolean isActive(long deviceControllerPtr, long deviceId);

  private native void shutdownSubscriptions(long deviceControllerPtr, long devicePtr);

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

    /** Notifies the pairing status. */
    void onStatusUpdate(int status);

    /** Notifies the completion of pairing. */
    void onPairingComplete(int errorCode);

    /** Notifies the deletion of pairing session. */
    void onPairingDeleted(int errorCode);

    /** Notifies the completion of commissioning. */
    void onCommissioningComplete(long nodeId, int errorCode);

    /** Notifies that the Chip connection has been closed. */
    void onNotifyChipConnectionClosed();

    /** Notifies the completion of the "close BLE connection" command. */
    void onCloseBleComplete();

    /** Notifies the listener of the error. */
    void onError(Throwable error);

    /** Notifies the Commissioner when the OpCSR for the Comissionee is generated. */
    void onOpCSRGenerationComplete(byte[] csr);
  }
}
