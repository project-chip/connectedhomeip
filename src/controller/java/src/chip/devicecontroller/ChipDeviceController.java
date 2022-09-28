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
import chip.devicecontroller.model.ChipEventPath;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/** Controller to interact with the CHIP device. */
public class ChipDeviceController {
  private static final String TAG = ChipDeviceController.class.getSimpleName();
  private long deviceControllerPtr;
  private int connectionId;
  private CompletionListener completionListener;
  private ScanNetworksListener scanNetworksListener;
  private NOCChainIssuer nocChainIssuer;

  /**
   * To load class and jni, we need to new AndroidChipPlatform after jni load but before new
   * ChipDeviceController
   */
  public static void loadJni() {
    return;
  }

  /**
   * Returns a new {@link ChipDeviceController} with the specified parameters. you must set a vendor
   * ID, ControllerParams.newBuilder().setControllerVendorId(0xFFF4).build() 0xFFF4 is a test vendor
   * ID
   */
  public ChipDeviceController(ControllerParams params) {
    if (params == null) {
      throw new NullPointerException("params cannot be null");
    }
    deviceControllerPtr = newDeviceController(params);
  }

  public void setCompletionListener(CompletionListener listener) {
    completionListener = listener;
  }

  public void setScanNetworksListener(ScanNetworksListener listener) {
    scanNetworksListener = listener;
  }

  /**
   * Sets this DeviceController to use the given issuer for issuing operational certs and verifying
   * the DAC. By default, the DeviceController uses an internal, OperationalCredentialsDelegate (see
   * AndroidOperationalCredentialsIssuer).
   *
   * <p>When a NOCChainIssuer is set for this controller, then onNOCChainGenerationNeeded will be
   * called when the NOC CSR needs to be signed and DAC verified. This allows for custom credentials
   * issuer and DAC verifier implementations, for example, when a proprietary cloud API will perform
   * DAC verification and the CSR signing.
   *
   * <p>When a NOCChainIssuer is set for this controller, the PartialDACVerifier will be used rather
   * than the DefaultDACVerifier.
   *
   * @param issuer
   */
  public void setNOCChainIssuer(NOCChainIssuer issuer) {
    setUseJavaCallbackForNOCRequest(deviceControllerPtr, issuer != null);
    nocChainIssuer = issuer;
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

  /**
   * When a NOCChainIssuer is set for this controller, then onNOCChainGenerationNeeded will be
   * called when the NOC CSR needs to be signed. This allows for custom credentials issuer
   * implementations, for example, when a proprietary cloud API will perform the CSR signing.
   *
   * <p>The commissioning workflow will stop upon the onNOCChainGenerationNeeded callback and resume
   * once onNOCChainGeneration is called.
   *
   * <p>The following fields on the ControllerParams object MUST be populated: rootCertificate,
   * intermediateCertificate, operationalCertificate
   *
   * <p>If ipk and adminSubject are set on the ControllerParams object, then they will be used in
   * the AddNOC command set to the commissionee. If they are not populated, then the values provided
   * in the ChipDeviceController initialization will be used.
   *
   * @param params
   * @return CHIP_ERROR error code (0 is no error)
   */
  public int onNOCChainGeneration(ControllerParams params) {
    return onNOCChainGeneration(deviceControllerPtr, params);
  }

  /**
   * Update the network credentials held by the commissioner for the current commissioning session.
   * The updated values will be used by the commissioner if the network credentials haven't already
   * been sent to the device.
   *
   * <p>Its expected that this method will be called in response to the NetworkScan or the
   * ReadCommissioningInfo callbacks.
   *
   * @param networkCredentials the credentials (Wi-Fi or Thread) to use in commissioning
   */
  public void updateCommissioningNetworkCredentials(NetworkCredentials networkCredentials) {
    updateCommissioningNetworkCredentials(deviceControllerPtr, networkCredentials);
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

  public void releaseConnectedDevicePointer(long devicePtr) {
    releaseOperationalDevicePointer(devicePtr);
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

  public void onCommissioningStatusUpdate(long nodeId, String stage, int errorCode) {
    if (completionListener != null) {
      completionListener.onCommissioningStatusUpdate(nodeId, stage, errorCode);
    }
  }

  public void onReadCommissioningInfo(
      int vendorId, int productId, int wifiEndpointId, int threadEndpointId) {
    if (completionListener != null) {
      completionListener.onReadCommissioningInfo(
          vendorId, productId, wifiEndpointId, threadEndpointId);
    }
  }

  public void onScanNetworksFailure(int errorCode) {
    if (scanNetworksListener != null) {
      scanNetworksListener.onScanNetworksFailure(errorCode);
    }
  }

  public void onScanNetworksSuccess(
      Integer networkingStatus,
      Optional<String> debugText,
      Optional<ArrayList<ChipStructs.NetworkCommissioningClusterWiFiInterfaceScanResult>>
          wiFiScanResults,
      Optional<ArrayList<ChipStructs.NetworkCommissioningClusterThreadInterfaceScanResult>>
          threadScanResults) {
    if (scanNetworksListener != null) {
      scanNetworksListener.onScanNetworksSuccess(
          networkingStatus, debugText, wiFiScanResults, threadScanResults);
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

  public void onNOCChainGenerationNeeded(CSRInfo csrInfo, AttestationInfo attestationInfo) {
    if (nocChainIssuer != null) {
      nocChainIssuer.onNOCChainGenerationNeeded(csrInfo, attestationInfo);
    }
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

  public boolean openPairingWindowCallback(
      long devicePtr, int duration, OpenCommissioningCallback callback) {
    return openPairingWindowCallback(deviceControllerPtr, devicePtr, duration, callback);
  }

  public boolean openPairingWindowWithPINCallback(
      long devicePtr,
      int duration,
      long iteration,
      int discriminator,
      long setupPinCode,
      OpenCommissioningCallback callback) {
    return openPairingWindowWithPINCallback(
        deviceControllerPtr, devicePtr, duration, iteration, discriminator, setupPinCode, callback);
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
        new ReportCallbackJni(subscriptionEstablishedCallback, reportCallback, null);
    subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        null,
        minInterval,
        maxInterval,
        false,
        false);
  }

  /** Subscribe to the given event path */
  public void subscribeToEventPath(
      SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ReportCallback reportCallback,
      long devicePtr,
      List<ChipEventPath> eventPaths,
      int minInterval,
      int maxInterval) {
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(subscriptionEstablishedCallback, reportCallback, null);
    subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        null,
        eventPaths,
        minInterval,
        maxInterval,
        false,
        false);
  }

  /** Subscribe to the given attribute/event path with keepSubscriptions and isFabricFiltered. */
  public void subscribeToPath(
      SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ResubscriptionAttemptCallback resubscriptionAttemptCallback,
      ReportCallback reportCallback,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      int minInterval,
      int maxInterval,
      boolean keepSubscriptions,
      boolean isFabricFiltered) {
    // TODO: pass resubscriptionAttemptCallback to ReportCallbackJni since jni layer is not ready
    // for auto-resubscribe
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(subscriptionEstablishedCallback, reportCallback, null);
    subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        eventPaths,
        minInterval,
        maxInterval,
        keepSubscriptions,
        isFabricFiltered);
  }

  /** Read the given attribute path. */
  public void readPath(
      ReportCallback callback, long devicePtr, List<ChipAttributePath> attributePaths) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
    read(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        null,
        true);
  }

  /** Read the given event path. */
  public void readEventPath(
      ReportCallback callback, long devicePtr, List<ChipEventPath> eventPaths) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
    read(deviceControllerPtr, jniCallback.getCallbackHandle(), devicePtr, null, eventPaths, true);
  }

  /** Read the given attribute/event path with isFabricFiltered flag. */
  public void readPath(
      ReportCallback callback,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      boolean isFabricFiltered) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
    read(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        eventPaths,
        isFabricFiltered);
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

  private native void subscribe(
      long deviceControllerPtr,
      long callbackHandle,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      int minInterval,
      int maxInterval,
      boolean keepSubscriptions,
      boolean isFabricFiltered);

  private native void read(
      long deviceControllerPtr,
      long callbackHandle,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      boolean isFabricFiltered);

  private native long newDeviceController(ControllerParams params);

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

  private native void releaseOperationalDevicePointer(long devicePtr);

  private native void deleteDeviceController(long deviceControllerPtr);

  private native String getIpAddress(long deviceControllerPtr, long deviceId);

  private native NetworkLocation getNetworkLocation(long deviceControllerPtr, long deviceId);

  private native long getCompressedFabricId(long deviceControllerPtr);

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

  private native boolean openPairingWindowCallback(
      long deviceControllerPtr, long devicePtr, int duration, OpenCommissioningCallback callback);

  private native boolean openPairingWindowWithPINCallback(
      long deviceControllerPtr,
      long devicePtr,
      int duration,
      long iteration,
      int discriminator,
      long setupPinCode,
      OpenCommissioningCallback callback);

  private native byte[] getAttestationChallenge(long deviceControllerPtr, long devicePtr);

  private native void setUseJavaCallbackForNOCRequest(
      long deviceControllerPtr, boolean useCallback);

  private native void updateCommissioningNetworkCredentials(
      long deviceControllerPtr, NetworkCredentials networkCredentials);

  private native int onNOCChainGeneration(long deviceControllerPtr, ControllerParams params);

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

  /** Interface to implement custom operational credentials issuer (NOC chain generation). */
  public interface NOCChainIssuer {
    /**
     * When a NOCChainIssuer is set for this controller, then onNOCChainGenerationNeeded will be
     * called when the DAC chain must be verified and NOC chain needs to be issued from a CSR. This
     * allows for custom credentials issuer and DAC verifier implementations, for example, when a
     * proprietary cloud API will perform DAC verification and the NOC chain issuance from CSR.
     *
     * <p>When a NOCChainIssuer is set for this controller, the PartialDACVerifier will be used
     * rather than the DefaultDACVerifier.
     *
     * <p>The commissioning workflow will stop upon the onNOCChainGenerationNeeded callback and
     * resume once onNOCChainGeneration is called.
     *
     * <p>The following fields on the ControllerParams object passed to onNOCChainGeneration MUST be
     * populated: rootCertificate, intermediateCertificate, operationalCertificate
     *
     * <p>If ipk and adminSubject are set on the ControllerParams object, then they will be used in
     * the AddNOC command set to the commissionee. If they are not populated, then the values
     * provided in the ChipDeviceController initialization will be used.
     *
     * <p>All csr and attestation fields are provided to allow for custom attestestation checks.
     */
    void onNOCChainGenerationNeeded(CSRInfo csrInfo, AttestationInfo attestationInfo);
  }

  /**
   * Interface to listen for scan networks callbacks from CHIPDeviceController.
   *
   * <p>Set the AttemptNetworkScanWiFi or AttemptNetworkScanThread to configure the enable/disable
   * WiFi or Thread network scan during commissioning in the the default CommissioningDelegate used
   * by the ChipDeviceCommissioner.
   *
   * <p>When the callbacks onScanNetworksFailure or onScanNetworksSuccess are invoked, the
   * commissioning flow has reached the kNeedsNetworkCreds and will wait to advance until this
   * device controller's updateCommissioningNetworkCredentials method is called with the desired
   * network credentials set.
   */
  public interface ScanNetworksListener {
    /** Notifies when scan networks call fails. */
    void onScanNetworksFailure(int errorCode);

    void onScanNetworksSuccess(
        Integer networkingStatus,
        Optional<String> debugText,
        Optional<ArrayList<ChipStructs.NetworkCommissioningClusterWiFiInterfaceScanResult>>
            wiFiScanResults,
        Optional<ArrayList<ChipStructs.NetworkCommissioningClusterThreadInterfaceScanResult>>
            threadScanResults);
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

    /** Notifies the completion of each stage of commissioning. */
    void onReadCommissioningInfo(
        int vendorId, int productId, int wifiEndpointId, int threadEndpointId);

    /** Notifies the completion of each stage of commissioning. */
    void onCommissioningStatusUpdate(long nodeId, String stage, int errorCode);

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
