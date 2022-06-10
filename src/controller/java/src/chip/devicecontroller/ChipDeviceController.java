/*
 *   Copyright (c) 2020-2022 Project CHIP Authors
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
import chip.devicecontroller.model.ChipAttributePath;
import java.util.List;

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

  public void pairDevice(
      BluetoothGatt bleServer,
      int connId,
      long deviceId,
      long setupPincode,
      NetworkCredentials networkCredentials) {
    pairDevice(bleServer, connId, deviceId, setupPincode, null, networkCredentials);
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
      @Nullable byte[] csrNonce,
      NetworkCredentials networkCredentials) {
    if (connectionId == 0) {
      connectionId = connId;

      if (connectionId == 0) {
        Log.e(TAG, "Failed to add Bluetooth connection.");
        completionListener.onError(new Exception("Failed to add Bluetooth connection."));
        return;
      }

      Log.d(TAG, "Bluetooth connection added with ID: " + connectionId);
      Log.d(TAG, "Pairing device with ID: " + deviceId);
      pairDevice(
          deviceControllerPtr, deviceId, connectionId, setupPincode, csrNonce, networkCredentials);
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

  public void establishPaseConnection(long deviceId, int connId, long setupPincode) {
    if (connectionId == 0) {
      connectionId = connId;

      if (connectionId == 0) {
        Log.e(TAG, "Failed to add Bluetooth connection.");
        completionListener.onError(new Exception("Failed to add Bluetooth connection."));
        return;
      }

      Log.d(TAG, "Bluetooth connection added with ID: " + connectionId);
      Log.d(TAG, "Establishing PASE connection with ID: " + deviceId);
      establishPaseConnection(deviceControllerPtr, deviceId, connId, setupPincode);
    } else {
      Log.e(TAG, "Bluetooth connection already in use.");
      completionListener.onError(new Exception("Bluetooth connection already in use."));
    }
  }

  /**
   * Establish a secure PASE connection to the given device via IP address.
   *
   * @param deviceId the ID of the node to connect to
   * @param address the IP address at which the node is located
   * @param port the port at which the node is located
   * @param setupPincode the pincode for this node
   */
  public void establishPaseConnection(long deviceId, String address, int port, long setupPincode) {
    Log.d(TAG, "Establishing PASE connection with ID: " + deviceId);
    establishPaseConnectionByAddress(deviceControllerPtr, deviceId, address, port, setupPincode);
  }

  /**
   * Initiates the automatic commissioning flow using the specified network credentials. It is
   * expected that a secure session has already been established via {@link
   * #establishPaseConnection(long, int, long)}.
   *
   * @param deviceId the ID of the node to be commissioned
   * @param networkCredentials the credentials (Wi-Fi or Thread) to be provisioned
   */
  public void commissionDevice(long deviceId, @Nullable NetworkCredentials networkCredentials) {
    commissionDevice(deviceControllerPtr, deviceId, /* csrNonce= */ null, networkCredentials);
  }

  /**
   * Initiates the automatic commissioning flow using the specified network credentials. It is
   * expected that a secure session has already been established via {@link
   * #establishPaseConnection(long, int, long)}.
   *
   * @param deviceId the ID of the node to be commissioned
   * @param csrNonce a nonce to be used for the CSR request
   * @param networkCredentials the credentials (Wi-Fi or Thread) to be provisioned
   */
  public void commissionDevice(
      long deviceId, @Nullable byte[] csrNonce, @Nullable NetworkCredentials networkCredentials) {
    commissionDevice(deviceControllerPtr, deviceId, csrNonce, networkCredentials);
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
   * <p>The native code invoked by this method creates a strong reference to the provided callback,
   * which is released only when GetConnectedDeviceCallback has returned success or failure.
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

  /**
   * Returns the {@link NetworkLocation} at which the given {@code deviceId} has been found.
   *
   * @param deviceId the 64-bit node ID of the device
   * @throws ChipDeviceControllerException if the device location could not be resolved
   */
  public NetworkLocation getNetworkLocation(long deviceId) {
    return getNetworkLocation(deviceControllerPtr, deviceId);
  }

  public long getCompressedFabricId() {
    return getCompressedFabricId(deviceControllerPtr);
  }

  /**
   * Returns the compressed fabric ID based on the given root certificate and node operational
   * credentials.
   *
   * @param rcac the root certificate (in Matter cert form)
   * @param noc the NOC (in Matter cert form)
   * @see #convertX509CertToMatterCert(byte[])
   */
  public native long generateCompressedFabricId(byte[] rcac, byte[] noc);

  public void updateDevice(long fabricId, long deviceId) {
    updateDevice(deviceControllerPtr, fabricId, deviceId);
  }

  /**
   * Get commmissionible Node. Commmissionible Node results are able to get using {@link
   * ChipDeviceController.getDiscoveredDevice}.
   */
  public void discoverCommissionableNodes() {
    discoverCommissionableNodes(deviceControllerPtr);
  }

  public DiscoveredDevice getDiscoveredDevice(int idx) {
    return getDiscoveredDevice(deviceControllerPtr, idx);
  }

  public boolean openPairingWindow(long devicePtr, int duration) {
    return openPairingWindow(deviceControllerPtr, devicePtr, duration);
  }

  public boolean openPairingWindowWithPIN(
      long devicePtr, int duration, long iteration, int discriminator, long setupPinCode) {
    return openPairingWindowWithPIN(
        deviceControllerPtr, devicePtr, duration, iteration, discriminator, setupPinCode);
  }

  /* Shutdown all cluster attribute subscriptions for a given device */
  public void shutdownSubscriptions(long devicePtr) {
    shutdownSubscriptions(deviceControllerPtr, devicePtr);
  }

  /**
   * Returns an attestation challenge for the given device, for which there must be an existing
   * secure session.
   *
   * @param devicePtr a pointer to the device from which to retrieve the challenge
   * @throws ChipDeviceControllerException if there is no secure session for the given device
   */
  public byte[] getAttestationChallenge(long devicePtr) {
    return getAttestationChallenge(deviceControllerPtr, devicePtr);
  }

  /** Subscribe to the given attribute path. */
  public void subscribeToPath(
      SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ReportCallback reportCallback,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      int minInterval,
      int maxInterval) {
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(subscriptionEstablishedCallback, reportCallback);
    subscribeToPath(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        minInterval,
        maxInterval);
  }

  /** Read the given attribute path. */
  public void readPath(
      ReportCallback callback, long devicePtr, List<ChipAttributePath> attributePaths) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback);
    readPath(deviceControllerPtr, jniCallback.getCallbackHandle(), devicePtr, attributePaths);
  }

  /**
   * Converts a given X.509v3 certificate into a Matter certificate.
   *
   * @throws ChipDeviceControllerException if there was an issue during encoding (e.g. out of
   *     memory, invalid certificate format)
   */
  public native byte[] convertX509CertToMatterCert(byte[] x509Cert);

  /**
   * Generates a new PASE verifier for the given setup PIN code.
   *
   * @param devicePtr a pointer to the device object for which to generate the PASE verifier
   * @param setupPincode the PIN code to use
   * @param iterations the number of iterations for computing the verifier
   * @param salt the 16-byte salt
   */
  public PaseVerifierParams computePaseVerifier(
      long devicePtr, long setupPincode, long iterations, byte[] salt) {
    return computePaseVerifier(deviceControllerPtr, devicePtr, setupPincode, iterations, salt);
  }

  public void shutdownCommissioning() {
    shutdownCommissioning(deviceControllerPtr);
  }

  private native PaseVerifierParams computePaseVerifier(
      long deviceControllerPtr, long devicePtr, long setupPincode, long iterations, byte[] salt);

  private native void subscribeToPath(
      long deviceControllerPtr,
      long callbackHandle,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      int minInterval,
      int maxInterval);

  public native void readPath(
      long deviceControllerPtr,
      long callbackHandle,
      long devicePtr,
      List<ChipAttributePath> attributePaths);

  private native long newDeviceController();

  private native void pairDevice(
      long deviceControllerPtr,
      long deviceId,
      int connectionId,
      long pinCode,
      @Nullable byte[] csrNonce,
      NetworkCredentials networkCredentials);

  private native void pairDeviceWithAddress(
      long deviceControllerPtr,
      long deviceId,
      String address,
      int port,
      int discriminator,
      long pinCode,
      @Nullable byte[] csrNonce);

  private native void establishPaseConnection(
      long deviceControllerPtr, long deviceId, int connId, long setupPincode);

  private native void establishPaseConnectionByAddress(
      long deviceControllerPtr, long deviceId, String address, int port, long setupPincode);

  private native void commissionDevice(
      long deviceControllerPtr,
      long deviceId,
      @Nullable byte[] csrNonce,
      @Nullable NetworkCredentials networkCredentials);

  private native void unpairDevice(long deviceControllerPtr, long deviceId);

  private native long getDeviceBeingCommissionedPointer(long deviceControllerPtr, long nodeId);

  private native void getConnectedDevicePointer(
      long deviceControllerPtr, long deviceId, long callbackHandle);

  private native boolean disconnectDevice(long deviceControllerPtr, long deviceId);

  private native void deleteDeviceController(long deviceControllerPtr);

  private native String getIpAddress(long deviceControllerPtr, long deviceId);

  private native NetworkLocation getNetworkLocation(long deviceControllerPtr, long deviceId);

  private native long getCompressedFabricId(long deviceControllerPtr);

  private native void updateDevice(long deviceControllerPtr, long fabricId, long deviceId);

  private native void discoverCommissionableNodes(long deviceControllerPtr);

  private native DiscoveredDevice getDiscoveredDevice(long deviceControllerPtr, int idx);

  private native boolean openPairingWindow(long deviceControllerPtr, long devicePtr, int duration);

  private native boolean openPairingWindowWithPIN(
      long deviceControllerPtr,
      long devicePtr,
      int duration,
      long iteration,
      int discriminator,
      long setupPinCode);

  private native byte[] getAttestationChallenge(long deviceControllerPtr, long devicePtr);

  private native void shutdownSubscriptions(long deviceControllerPtr, long devicePtr);

  private native void shutdownCommissioning(long deviceControllerPtr);

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
