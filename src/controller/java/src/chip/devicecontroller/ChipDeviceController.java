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
import chip.devicecontroller.ChipDeviceController.CompletionListener;
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback;
import chip.devicecontroller.model.AttributeWriteRequest;
import chip.devicecontroller.model.ChipAttributePath;
import chip.devicecontroller.model.ChipEventPath;
import chip.devicecontroller.model.DataVersionFilter;
import chip.devicecontroller.model.InvokeElement;
import java.time.ZoneOffset;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Optional;
import java.util.TimeZone;
import javax.annotation.Nonnull;
import javax.annotation.Nullable;

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

  // temp, for kotlin_library
  public long getDeviceControllerPtr() {
    return deviceControllerPtr;
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

  /**
   * If DeviceAttestationDelegate is setted, then it will always be called when device attestation
   * completes. In case the device attestation fails, the client can decide to continue or stop the
   * commissioning.
   *
   * <p>When {@link DeviceAttestationDelegate#onDeviceAttestationCompleted(long, long,
   * AttestationInfo, int)} is received, {@link #continueCommissioning(long, boolean)} must be
   * called.
   *
   * @param failSafeExpiryTimeoutSecs the value to set for the fail-safe timer before
   *     onDeviceAttestationCompleted is invoked. The unit is seconds.
   * @param deviceAttestationDelegate the delegate for device attestation completed with device info
   *     for additional verification.
   */
  public void setDeviceAttestationDelegate(
      int failSafeExpiryTimeoutSecs, DeviceAttestationDelegate deviceAttestationDelegate) {
    setDeviceAttestationDelegate(
        deviceControllerPtr, failSafeExpiryTimeoutSecs, deviceAttestationDelegate);
  }

  /**
   * Set the delegate of attestation trust store for device attestation.
   *
   * <p>It will replace the built-in attestation trust store, please make sure you have the required
   * paa certificates before commissioning.
   *
   * @param attestationTrustStoreDelegate Delegate for attestation trust store
   * @param cdTrustKeys certification Declaration Trust Keys
   */
  public void setAttestationTrustStoreDelegate(
      AttestationTrustStoreDelegate attestationTrustStoreDelegate,
      @Nullable List<byte[]> cdTrustKeys) {
    setAttestationTrustStoreDelegate(
        deviceControllerPtr, attestationTrustStoreDelegate, cdTrustKeys);
  }

  public void setAttestationTrustStoreDelegate(
      AttestationTrustStoreDelegate attestationTrustStoreDelegate) {
    setAttestationTrustStoreDelegate(deviceControllerPtr, attestationTrustStoreDelegate, null);
  }

  /**
   * Set the delegate of OTA Provider for firmware Update
   *
   * @param aOTAProviderDelegate Delegate for OTA Provider
   */
  public void startOTAProvider(OTAProviderDelegate aOTAProviderDelegate) {
    startOTAProvider(deviceControllerPtr, aOTAProviderDelegate);
  }

  /** Disable OTA Provider server cluster */
  public void finishOTAProvider() {
    finishOTAProvider(deviceControllerPtr);
  }

  /** Set the delegate of ICD check in */
  public void setICDCheckInDelegate(ICDCheckInDelegate delegate) {
    setICDCheckInDelegate(deviceControllerPtr, new ICDCheckInDelegateWrapper(delegate));
  }

  /* This method was deprecated. Please use {@link ChipDeviceController.pairDevice(BluetoothGatt, int, long, long, CommissionParameters)}. */
  @Deprecated
  public void pairDevice(
      BluetoothGatt bleServer,
      int connId,
      long deviceId,
      long setupPincode,
      NetworkCredentials networkCredentials) {
    pairDevice(bleServer, connId, deviceId, setupPincode, null, networkCredentials, null);
  }

  /* This method was deprecated. Please use {@link ChipDeviceController.pairDevice(BluetoothGatt, int, long, long, CommissionParameters)}. */
  @Deprecated
  public void pairDevice(
      BluetoothGatt bleServer,
      int connId,
      long deviceId,
      long setupPincode,
      NetworkCredentials networkCredentials,
      ICDRegistrationInfo registrationInfo) {
    pairDevice(
        bleServer, connId, deviceId, setupPincode, null, networkCredentials, registrationInfo);
  }

  /* This method was deprecated. Please use {@link ChipDeviceController.pairDevice(BluetoothGatt, int, long, long, CommissionParameters)}. */
  @Deprecated
  public void pairDevice(
      BluetoothGatt bleServer,
      int connId,
      long deviceId,
      long setupPincode,
      @Nullable byte[] csrNonce,
      NetworkCredentials networkCredentials) {
    pairDevice(bleServer, connId, deviceId, setupPincode, csrNonce, networkCredentials, null);
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
   * @param networkCredentials the credentials (Wi-Fi or Thread) to be provisioned
   * @param icdRegistrationInfo the informations for ICD registration. For detailed information
   *     {@link ICDRegistrationInfo}. If this value is null when commissioning an ICD device, {@link
   *     CompletionListener.onICDRegistrationInfoRequired} is called to request the
   *     ICDRegistrationInfo value. This method was deprecated. Please use {@link
   *     ChipDeviceController.pairDevice(BluetoothGatt, int, long, long, CommissionParameters)}.
   */
  @Deprecated
  public void pairDevice(
      BluetoothGatt bleServer,
      int connId,
      long deviceId,
      long setupPincode,
      @Nullable byte[] csrNonce,
      NetworkCredentials networkCredentials,
      @Nullable ICDRegistrationInfo icdRegistrationInfo) {
    CommissionParameters params =
        new CommissionParameters.Builder()
            .setCsrNonce(csrNonce)
            .setNetworkCredentials(networkCredentials)
            .setICDRegistrationInfo(icdRegistrationInfo)
            .build();
    pairDevice(bleServer, connId, deviceId, setupPincode, params);
  }

  /**
   * Pair a device connected through BLE.
   *
   * @param bleServer the BluetoothGatt representing the BLE connection to the device
   * @param connId the BluetoothGatt Id representing the BLE connection to the device
   * @param deviceId the node ID to assign to the device
   * @param setupPincode the pincode for the device
   * @param params Parameters representing commissioning arguments. see detailed in {@link
   *     CommissionParameters}
   */
  public void pairDevice(
      BluetoothGatt bleServer,
      int connId,
      long deviceId,
      long setupPincode,
      @Nonnull CommissionParameters params) {
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
          deviceControllerPtr,
          deviceId,
          connectionId,
          setupPincode,
          params.getCsrNonce(),
          params.getNetworkCredentials(),
          params.getICDRegistrationInfo());
    } else {
      Log.e(TAG, "Bluetooth connection already in use.");
      completionListener.onError(new Exception("Bluetooth connection already in use."));
    }
  }

  /* This method was deprecated. Please use {@link ChipDeviceController.pairDeviceWithAddress(long, String, int, int, long, CommissionParameters)}. */
  public void pairDeviceWithAddress(
      long deviceId,
      String address,
      int port,
      int discriminator,
      long pinCode,
      @Nullable byte[] csrNonce) {
    pairDeviceWithAddress(
        deviceControllerPtr, deviceId, address, port, discriminator, pinCode, csrNonce, null);
  }

  /**
   * Pair a device connected using IP Address.
   *
   * @param deviceId the node ID to assign to the device
   * @param address IP Address of the connecting device
   * @param port the port of the connecting device
   * @param discriminator the discriminator for connecting device
   * @param pinCode the pincode for connecting device
   * @param csrNonce the 32-byte CSR nonce to use, or null if we want to use an internally randomly
   *     generated CSR nonce.
   * @param icdRegistrationInfo the informations for ICD registration. For detailed information
   *     {@link ICDRegistrationInfo}. If this value is null when commissioning an ICD device, {@link
   *     CompletionListener.onICDRegistrationInfoRequired} is called to request the
   *     ICDRegistrationInfo value. This method was deprecated. Please use {@link
   *     ChipDeviceController.pairDeviceWithAddress(long, String, int, int, long,
   *     CommissionParameters)}.
   */
  @Deprecated
  public void pairDeviceWithAddress(
      long deviceId,
      String address,
      int port,
      int discriminator,
      long pinCode,
      @Nullable byte[] csrNonce,
      @Nullable ICDRegistrationInfo icdRegistrationInfo) {
    pairDeviceWithAddress(
        deviceControllerPtr,
        deviceId,
        address,
        port,
        discriminator,
        pinCode,
        csrNonce,
        icdRegistrationInfo);
  }

  /**
   * Pair a device connected using IP Address.
   *
   * @param deviceId the node ID to assign to the device
   * @param address IP Address of the connecting device
   * @param port the port of the connecting device
   * @param discriminator the discriminator for connecting device
   * @param pinCode the pincode for connecting device
   * @param params Parameters representing commissioning arguments. see detailed in {@link
   *     CommissionParameters}
   */
  public void pairDeviceWithAddress(
      long deviceId,
      String address,
      int port,
      int discriminator,
      long pinCode,
      @Nonnull CommissionParameters params) {
    if (params.getNetworkCredentials() != null) {
      Log.e(TAG, "Invalid parameter : NetworkCredentials");
      completionListener.onError(new Exception("Invalid parameter : NetworkCredentials"));
      return;
    }
    pairDeviceWithAddress(
        deviceControllerPtr,
        deviceId,
        address,
        port,
        discriminator,
        pinCode,
        params.getCsrNonce(),
        params.getICDRegistrationInfo());
  }

  /* This method was deprecated. Please use {@link ChipDeviceController.pairDeviceWithCode(long, String, boolean, boolean, CommissionParameters)}. */
  @Deprecated
  public void pairDeviceWithCode(
      long deviceId,
      String setupCode,
      boolean discoverOnce,
      boolean useOnlyOnNetworkDiscovery,
      @Nullable byte[] csrNonce,
      @Nullable NetworkCredentials networkCredentials) {
    pairDeviceWithCode(
        deviceControllerPtr,
        deviceId,
        setupCode,
        discoverOnce,
        useOnlyOnNetworkDiscovery,
        csrNonce,
        networkCredentials,
        null);
  }

  /**
   * Pair a device connected using the scanned QR code or manual entry code.
   *
   * @param deviceId the node ID to assign to the device
   * @param setupCode the scanned QR code or manual entry code
   * @param discoverOnce the flag to enable/disable PASE auto retry mechanism
   * @param useOnlyOnNetworkDiscovery the flag to indicate the commissionable device is available on
   *     the network
   * @param csrNonce the 32-byte CSR nonce to use, or null if we want to use an internally randomly
   *     generated CSR nonce.
   * @param networkCredentials the credentials (Wi-Fi or Thread) to be provisioned
   * @param icdRegistrationInfo the informations for ICD registration. For detailed information
   *     {@link ICDRegistrationInfo}. If this value is null when commissioning an ICD device, {@link
   *     CompletionListener.onICDRegistrationInfoRequired} is called to request the
   *     ICDRegistrationInfo value.
   *     <p>This method was deprecated. Please use {@link
   *     ChipDeviceController.pairDeviceWithCode(long, String, boolean, boolean,
   *     CommissionParameters)}.
   */
  @Deprecated
  public void pairDeviceWithCode(
      long deviceId,
      String setupCode,
      boolean discoverOnce,
      boolean useOnlyOnNetworkDiscovery,
      @Nullable byte[] csrNonce,
      @Nullable NetworkCredentials networkCredentials,
      @Nullable ICDRegistrationInfo icdRegistrationInfo) {
    pairDeviceWithCode(
        deviceControllerPtr,
        deviceId,
        setupCode,
        discoverOnce,
        useOnlyOnNetworkDiscovery,
        csrNonce,
        networkCredentials,
        icdRegistrationInfo);
  }

  /**
   * Pair a device connected using the scanned QR code or manual entry code.
   *
   * @param deviceId the node ID to assign to the device
   * @param setupCode the scanned QR code or manual entry code
   * @param discoverOnce the flag to enable/disable PASE auto retry mechanism
   * @param useOnlyOnNetworkDiscovery the flag to indicate the commissionable device is available on
   *     the network
   * @param params Parameters representing commissioning arguments. see detailed in {@link
   *     CommissionParameters}
   */
  public void pairDeviceWithCode(
      long deviceId,
      String setupCode,
      boolean discoverOnce,
      boolean useOnlyOnNetworkDiscovery,
      @Nonnull CommissionParameters params) {
    pairDeviceWithCode(
        deviceControllerPtr,
        deviceId,
        setupCode,
        discoverOnce,
        useOnlyOnNetworkDiscovery,
        params.getCsrNonce(),
        params.getNetworkCredentials(),
        params.getICDRegistrationInfo());
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
   * Establish a secure PASE connection using the scanned QR code or manual entry code.
   *
   * @param deviceId the ID of the node to connect to
   * @param setupCode the scanned QR code or manual entry code
   * @param useOnlyOnNetworkDiscovery the flag to indicate the commissionable device is available on
   *     the network
   */
  public void establishPaseConnection(
      long deviceId, String setupCode, boolean useOnlyOnNetworkDiscovery) {
    Log.d(TAG, "Establishing PASE connection using Code: " + setupCode);
    establishPaseConnectionByCode(
        deviceControllerPtr, deviceId, setupCode, useOnlyOnNetworkDiscovery);
  }

  /**
   * Initiates the automatic commissioning flow using the specified network credentials. It is
   * expected that a secure session has already been established via {@link
   * #establishPaseConnection(long, int, long)}.
   *
   * @param deviceId the ID of the node to be commissioned
   * @param networkCredentials the credentials (Wi-Fi or Thread) to be provisioned
   *     <p>This method was deprecated. Please use {@link
   *     ChipDeviceController.commissionDevice(long, CommissionParameters)}.
   */
  @Deprecated
  public void commissionDevice(long deviceId, @Nullable NetworkCredentials networkCredentials) {
    commissionDevice(deviceControllerPtr, deviceId, /* csrNonce= */ null, networkCredentials, null);
  }

  /**
   * Initiates the automatic commissioning flow using the specified network credentials. It is
   * expected that a secure session has already been established via {@link
   * #establishPaseConnection(long, int, long)}.
   *
   * @param deviceId the ID of the node to be commissioned
   * @param csrNonce a nonce to be used for the CSR request
   * @param networkCredentials the credentials (Wi-Fi or Thread) to be provisioned
   *     <p>This method was deprecated. Please use {@link
   *     ChipDeviceController.commissionDevice(long, CommissionParameters)}.
   */
  @Deprecated
  public void commissionDevice(
      long deviceId, @Nullable byte[] csrNonce, @Nullable NetworkCredentials networkCredentials) {
    commissionDevice(deviceControllerPtr, deviceId, csrNonce, networkCredentials, null);
  }

  /**
   * Initiates the automatic commissioning flow using the specified network credentials. It is
   * expected that a secure session has already been established via {@link
   * #establishPaseConnection(long, int, long)}.
   *
   * @param deviceId the ID of the node to be commissioned
   * @param params Parameters representing commissioning arguments. see detailed in {@link
   *     CommissionParameters}
   */
  public void commissionDevice(long deviceId, @Nonnull CommissionParameters params) {
    commissionDevice(
        deviceControllerPtr,
        deviceId,
        params.getCsrNonce(),
        params.getNetworkCredentials(),
        params.getICDRegistrationInfo());
  }

  /**
   * This function instructs the commissioner to proceed to the next stage of commissioning after
   * attestation is reported.
   *
   * @param devicePtr a pointer to the device which is being commissioned.
   * @param ignoreAttestationFailure whether to ignore device attestation failure.
   */
  public void continueCommissioning(long devicePtr, boolean ignoreAttestationFailure) {
    continueCommissioning(deviceControllerPtr, devicePtr, ignoreAttestationFailure);
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
  public long onNOCChainGeneration(ControllerParams params) {
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

  /**
   * Update the ICD registration information held by the commissioner for the current commissioning
   * session.
   *
   * <p>Its expected that this method will be called in response the onICDRegistrationInfoRequired
   * callbacks.
   *
   * @param ICDRegistrationInfo the ICD registration information to use in commissioning
   */
  public void updateCommissioningICDRegistrationInfo(ICDRegistrationInfo icdRegistrationInfo) {
    updateCommissioningICDRegistrationInfo(deviceControllerPtr, icdRegistrationInfo);
  }

  public void unpairDevice(long deviceId) {
    unpairDevice(deviceControllerPtr, deviceId);
  }

  public void unpairDeviceCallback(long deviceId, UnpairDeviceCallback callback) {
    unpairDeviceCallback(deviceControllerPtr, deviceId, callback);
  }

  /**
   * This function stops a pairing or commissioning process that is in progress.
   *
   * @param deviceId The remote device Id.
   */
  public void stopDevicePairing(long deviceId) {
    stopDevicePairing(deviceControllerPtr, deviceId);
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

  public long getGroupDevicePointer(int groupId) {
    return getGroupDevicePointer(deviceControllerPtr, groupId);
  }

  public native void releaseGroupDevicePointer(long devicePtr);

  public List<Integer> getAvailableGroupIds() {
    return getAvailableGroupIds(deviceControllerPtr);
  }

  public String getGroupName(int groupId) {
    return getGroupName(deviceControllerPtr, groupId);
  }

  public Optional<Integer> findKeySetId(int groupId) {
    return findKeySetId(deviceControllerPtr, groupId);
  }

  public boolean addGroup(int groupId, String groupName) {
    return addGroup(deviceControllerPtr, groupId, groupName);
  }

  public boolean removeGroup(int groupId) {
    return removeGroup(deviceControllerPtr, groupId);
  }

  public List<Integer> getKeySetIds() {
    return getKeySetIds(deviceControllerPtr);
  }

  public Optional<GroupKeySecurityPolicy> getKeySecurityPolicy(int keySetId) {
    return getKeySecurityPolicy(deviceControllerPtr, keySetId)
        .map(id -> GroupKeySecurityPolicy.value(id));
  }

  public boolean bindKeySet(int groupId, int keySetId) {
    return bindKeySet(deviceControllerPtr, groupId, keySetId);
  }

  public boolean unbindKeySet(int groupId, int keySetId) {
    return unbindKeySet(deviceControllerPtr, groupId, keySetId);
  }

  public boolean addKeySet(
      int keySetId, GroupKeySecurityPolicy keyPolicy, long validityTime, byte[] epochKey) {
    return addKeySet(deviceControllerPtr, keySetId, keyPolicy.getID(), validityTime, epochKey);
  }

  public boolean removeKeySet(int keySetId) {
    return removeKeySet(deviceControllerPtr, keySetId);
  }

  public void onConnectDeviceComplete() {
    completionListener.onConnectDeviceComplete();
  }

  public void onStatusUpdate(int status) {
    if (completionListener != null) {
      completionListener.onStatusUpdate(status);
    }
  }

  public void onPairingComplete(long errorCode) {
    if (completionListener != null) {
      completionListener.onPairingComplete(errorCode);
    }
  }

  public void onCommissioningComplete(long nodeId, long errorCode) {
    if (completionListener != null) {
      completionListener.onCommissioningComplete(nodeId, errorCode);
    }
  }

  public void onCommissioningStatusUpdate(long nodeId, String stage, long errorCode) {
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

  public void onScanNetworksFailure(long errorCode) {
    if (scanNetworksListener != null) {
      scanNetworksListener.onScanNetworksFailure(errorCode);
    }
  }

  public void onScanNetworksSuccess(
      Integer networkingStatus,
      Optional<String> debugText,
      Optional<ArrayList<WiFiScanResult>> wiFiScanResults,
      Optional<ArrayList<ThreadScanResult>> threadScanResults) {
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

  public void onPairingDeleted(long errorCode) {
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

  public void onICDRegistrationInfoRequired() {
    if (completionListener != null) {
      completionListener.onICDRegistrationInfoRequired();
    }
  }

  public void onICDRegistrationComplete(long errorCode, ICDDeviceInfo icdDeviceInfo) {
    if (completionListener != null) {
      completionListener.onICDRegistrationComplete(errorCode, icdDeviceInfo);
    }
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

  /** Get device Controller's Node ID. */
  public long getControllerNodeId() {
    return getControllerNodeId(deviceControllerPtr);
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
      long devicePtr,
      int duration,
      long iteration,
      int discriminator,
      @Nullable Long setupPinCode) {
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
      @Nullable Long setupPinCode,
      OpenCommissioningCallback callback) {
    return openPairingWindowWithPINCallback(
        deviceControllerPtr, devicePtr, duration, iteration, discriminator, setupPinCode, callback);
  }

  public int getFabricIndex() {
    return getFabricIndex(deviceControllerPtr);
  }

  public List<ICDClientInfo> getICDClientInfo() {
    return getICDClientInfo(getFabricIndex(deviceControllerPtr));
  }

  /**
   * Returns the ICD Client Information
   *
   * @param fabricIndex the fabric index to check
   */
  public List<ICDClientInfo> getICDClientInfo(int fabricIndex) {
    return ChipICDClient.getICDClientInfo(fabricIndex);
  }

  /* Shuts down all active subscriptions. */
  public void shutdownSubscriptions() {
    ChipInteractionClient.shutdownSubscriptions(deviceControllerPtr, null, null, null);
  }

  /* Shuts down all active subscriptions for the fabric at the given fabricIndex */
  public void shutdownSubscriptions(int fabricIndex) {
    ChipInteractionClient.shutdownSubscriptions(
        deviceControllerPtr, Integer.valueOf(fabricIndex), null, null);
  }

  /**
   * Shuts down all subscriptions for a particular node.
   *
   * @param fabricIndex the fabric index of to which the node belongs
   * @param peerNodeId the node ID of the device for which subscriptions should be canceled
   */
  public void shutdownSubscriptions(int fabricIndex, long peerNodeId) {
    ChipInteractionClient.shutdownSubscriptions(
        deviceControllerPtr, Integer.valueOf(fabricIndex), Long.valueOf(peerNodeId), null);
  }

  /**
   * Shuts down all subscriptions for a particular node.
   *
   * @param fabricIndex the fabric index of to which the node belongs
   * @param peerNodeId the node ID of the device for which subscriptions should be canceled
   * @param subscriptionId the ID of the subscription on the node which should be canceled
   */
  public void shutdownSubscriptions(int fabricIndex, long peerNodeId, long subscriptionId) {
    ChipInteractionClient.shutdownSubscriptions(
        deviceControllerPtr,
        Integer.valueOf(fabricIndex),
        Long.valueOf(peerNodeId),
        Long.valueOf(subscriptionId));
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

  public void startDnssd() {
    startDnssd(deviceControllerPtr);
  }

  public void stopDnssd() {
    stopDnssd(deviceControllerPtr);
  }

  /**
   * @brief Auto-Resubscribe to the given attribute path with keepSubscriptions and isFabricFiltered
   * @param SubscriptionEstablishedCallback Callback when a subscribe response has been received and
   *     processed
   * @param ReportCallback Callback when a report data has been received and processed for the given
   *     paths.
   * @param devicePtr connected device pointer
   * @param attributePaths a list of attribute paths
   * @param minInterval the requested minimum interval boundary floor in seconds
   * @param maxInterval the requested maximum interval boundary ceiling in seconds
   * @param imTimeoutMs im interaction time out value, it would override the default value in c++ im
   *     layer if this value is non-zero.
   */
  public void subscribeToAttributePath(
      SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ReportCallback reportCallback,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      int minInterval,
      int maxInterval,
      int imTimeoutMs) {
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(subscriptionEstablishedCallback, reportCallback, null);
    ChipInteractionClient.subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        null,
        null,
        minInterval,
        maxInterval,
        false,
        false,
        imTimeoutMs,
        null,
        ChipICDClient.isPeerICDClient(
            ChipInteractionClient.getFabricIndex(devicePtr),
            ChipInteractionClient.getRemoteDeviceId(devicePtr)));
  }

  /**
   * @brief Auto-Resubscribe to the given event path with keepSubscriptions and isFabricFiltered
   * @param SubscriptionEstablishedCallback Callback when a subscribe response has been received and
   *     processed
   * @param ReportCallback Callback when a report data has been received and processed for the given
   *     paths.
   * @param devicePtr connected device pointer
   * @param eventPaths a list of event paths
   * @param minInterval the requested minimum interval boundary floor in seconds
   * @param maxInterval the requested maximum interval boundary ceiling in seconds
   * @param imTimeoutMs im interaction time out value, it would override the default value in c++ im
   *     layer if this value is non-zero.
   */
  public void subscribeToEventPath(
      SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ReportCallback reportCallback,
      long devicePtr,
      List<ChipEventPath> eventPaths,
      int minInterval,
      int maxInterval,
      int imTimeoutMs) {
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(subscriptionEstablishedCallback, reportCallback, null);
    ChipInteractionClient.subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        null,
        eventPaths,
        null,
        minInterval,
        maxInterval,
        false,
        false,
        imTimeoutMs,
        null,
        ChipICDClient.isPeerICDClient(
            ChipInteractionClient.getFabricIndex(devicePtr),
            ChipInteractionClient.getRemoteDeviceId(devicePtr)));
  }

  public void subscribeToEventPath(
      SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ReportCallback reportCallback,
      long devicePtr,
      List<ChipEventPath> eventPaths,
      int minInterval,
      int maxInterval,
      int imTimeoutMs,
      @Nullable Long eventMin) {
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(subscriptionEstablishedCallback, reportCallback, null);
    ChipInteractionClient.subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        null,
        eventPaths,
        null,
        minInterval,
        maxInterval,
        false,
        false,
        imTimeoutMs,
        eventMin,
        ChipICDClient.isPeerICDClient(
            ChipInteractionClient.getFabricIndex(devicePtr),
            ChipInteractionClient.getRemoteDeviceId(devicePtr)));
  }

  /**
   * @brief Auto-Resubscribe to the given attribute/event path with keepSubscriptions and
   *     isFabricFiltered
   */
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
      boolean isFabricFiltered,
      int imTimeoutMs) {
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(
            subscriptionEstablishedCallback, reportCallback, resubscriptionAttemptCallback);
    ChipInteractionClient.subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        eventPaths,
        null,
        minInterval,
        maxInterval,
        keepSubscriptions,
        isFabricFiltered,
        imTimeoutMs,
        null,
        ChipICDClient.isPeerICDClient(
            ChipInteractionClient.getFabricIndex(devicePtr),
            ChipInteractionClient.getRemoteDeviceId(devicePtr)));
  }

  /**
   * @brief Auto-Resubscribe to the given attribute/event/dataVersionFilter path with
   *     keepSubscriptions and isFabricFiltered
   * @param SubscriptionEstablishedCallback Callback when a subscribe response has been received and
   *     processed
   * @param ResubscriptionAttemptCallback Callback when a resubscirption haoppens, the termination
   *     cause is provided to help inform subsequent re-subscription logic.
   * @param ReportCallback Callback when a report data has been received and processed for the given
   *     paths.
   * @param devicePtr connected device pointer
   * @param attributePaths a list of attribute paths
   * @param eventPaths a list of event paths
   * @param dataVersionFilters a list of data version filter
   * @param minInterval the requested minimum interval boundary floor in seconds
   * @param maxInterval the requested maximum interval boundary ceiling in seconds
   * @param keepSubscriptions If KeepSubscriptions is FALSE, all existing or pending subscriptions
   *     on the publisher for this subscriber SHALL be terminated.
   * @param isFabricFiltered limits the data read within fabric-scoped lists to the accessing fabric
   * @param imTimeoutMs im interaction time out value, it would override the default value in c++ im
   *     layer if this value is non-zero.
   */
  public void subscribeToPath(
      SubscriptionEstablishedCallback subscriptionEstablishedCallback,
      ResubscriptionAttemptCallback resubscriptionAttemptCallback,
      ReportCallback reportCallback,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      List<DataVersionFilter> dataVersionFilters,
      int minInterval,
      int maxInterval,
      boolean keepSubscriptions,
      boolean isFabricFiltered,
      int imTimeoutMs) {
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(
            subscriptionEstablishedCallback, reportCallback, resubscriptionAttemptCallback);
    ChipInteractionClient.subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        eventPaths,
        dataVersionFilters,
        minInterval,
        maxInterval,
        keepSubscriptions,
        isFabricFiltered,
        imTimeoutMs,
        null,
        ChipICDClient.isPeerICDClient(
            ChipInteractionClient.getFabricIndex(devicePtr),
            ChipInteractionClient.getRemoteDeviceId(devicePtr)));
  }

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
      boolean isFabricFiltered,
      int imTimeoutMs,
      @Nullable Long eventMin) {
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(
            subscriptionEstablishedCallback, reportCallback, resubscriptionAttemptCallback);
    ChipInteractionClient.subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        eventPaths,
        null,
        minInterval,
        maxInterval,
        keepSubscriptions,
        isFabricFiltered,
        imTimeoutMs,
        eventMin,
        ChipICDClient.isPeerICDClient(
            ChipInteractionClient.getFabricIndex(devicePtr),
            ChipInteractionClient.getRemoteDeviceId(devicePtr)));
  }

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
      boolean isFabricFiltered,
      int imTimeoutMs,
      @Nullable Long eventMin,
      @Nullable Boolean isPeerLIT) {
    ReportCallbackJni jniCallback =
        new ReportCallbackJni(
            subscriptionEstablishedCallback, reportCallback, resubscriptionAttemptCallback);
    ChipInteractionClient.subscribe(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        eventPaths,
        null,
        minInterval,
        maxInterval,
        keepSubscriptions,
        isFabricFiltered,
        imTimeoutMs,
        eventMin,
        isPeerLIT != null
            ? isPeerLIT
            : ChipICDClient.isPeerICDClient(
                ChipInteractionClient.getFabricIndex(devicePtr),
                ChipInteractionClient.getRemoteDeviceId(devicePtr)));
  }

  /**
   * @brief read the given attribute path with isFabricFiltered
   * @param ReportCallback Callback when a report data has been received and processed for the given
   *     paths.
   * @param devicePtr connected device pointer
   * @param attributePaths a list of attribute paths
   * @param imTimeoutMs im interaction time out value, it would override the default value in c++ im
   *     layer if this value is non-zero.
   */
  public void readAttributePath(
      ReportCallback callback,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      int imTimeoutMs) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
    ChipInteractionClient.read(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        null,
        null,
        true,
        imTimeoutMs,
        null);
  }

  /**
   * @brief read the given event path with isFabricFiltered
   * @param ReportCallback Callback when a report data has been received and processed for the given
   *     paths.
   * @param devicePtr connected device pointer
   * @param eventPaths a list of event paths
   * @param imTimeoutMs im interaction time out value, it would override the default value in c++ im
   *     layer if this value is non-zero.
   */
  public void readEventPath(
      ReportCallback callback, long devicePtr, List<ChipEventPath> eventPaths, int imTimeoutMs) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
    ChipInteractionClient.read(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        null,
        eventPaths,
        null,
        true,
        imTimeoutMs,
        null);
  }

  /** Read the given event path. */
  public void readEventPath(
      ReportCallback callback,
      long devicePtr,
      List<ChipEventPath> eventPaths,
      int imTimeoutMs,
      @Nullable Long eventMin) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
    ChipInteractionClient.read(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        null,
        eventPaths,
        null,
        true,
        imTimeoutMs,
        eventMin);
  }

  /** Read the given attribute/event path with isFabricFiltered flag. */
  public void readPath(
      ReportCallback callback,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      boolean isFabricFiltered,
      int imTimeoutMs) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
    ChipInteractionClient.read(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        eventPaths,
        null,
        isFabricFiltered,
        imTimeoutMs,
        null);
  }

  /**
   * @brief read the given attribute/event/dataVersionFilter path
   * @param ReportCallback Callback when a report data has been received and processed for the given
   *     paths.
   * @param devicePtr connected device pointer
   * @param attributePaths a list of attribute paths
   * @param eventPaths a list of event paths
   * @param dataVersionFilters a list of data version filter
   * @param imTimeoutMs im interaction time out value, it would override the default value in c++ im
   *     layer if this value is non-zero.
   */
  public void readPath(
      ReportCallback callback,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      List<DataVersionFilter> dataVersionFilters,
      boolean isFabricFiltered,
      int imTimeoutMs) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
    ChipInteractionClient.read(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        eventPaths,
        dataVersionFilters,
        isFabricFiltered,
        imTimeoutMs,
        null);
  }

  /** Read the given attribute/event path with isFabricFiltered flag. */
  public void readPath(
      ReportCallback callback,
      long devicePtr,
      List<ChipAttributePath> attributePaths,
      List<ChipEventPath> eventPaths,
      boolean isFabricFiltered,
      int imTimeoutMs,
      @Nullable Long eventMin) {
    ReportCallbackJni jniCallback = new ReportCallbackJni(null, callback, null);
    ChipInteractionClient.read(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributePaths,
        eventPaths,
        null,
        isFabricFiltered,
        imTimeoutMs,
        eventMin);
  }

  /**
   * @brief Write a list of attributes into target device
   * @param WriteAttributesCallback Callback when a write response has been received and processed
   *     for the given path.
   * @param devicePtr connected device pointer
   * @param attributeList a list of attributes
   * @param timedRequestTimeoutMs this is timed request if this value is larger than 0
   * @param imTimeoutMs im interaction time out value, it would override the default value in c++ im
   *     layer if this value is non-zero.
   */
  public void write(
      WriteAttributesCallback callback,
      long devicePtr,
      List<AttributeWriteRequest> attributeList,
      int timedRequestTimeoutMs,
      int imTimeoutMs) {
    WriteAttributesCallbackJni jniCallback = new WriteAttributesCallbackJni(callback);
    ChipInteractionClient.write(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        attributeList,
        timedRequestTimeoutMs,
        imTimeoutMs);
  }

  /**
   * @brief Invoke command to target device
   * @param InvokeCallback Callback when an invoke response has been received and processed for the
   *     given invoke command.
   * @param devicePtr connected device pointer
   * @param invokeElement invoke command's path and arguments
   * @param timedRequestTimeoutMs this is timed request if this value is larger than 0
   * @param imTimeoutMs im interaction time out value, it would override the default value in c++ im
   *     layer if this value is non-zero.
   */
  public void invoke(
      InvokeCallback callback,
      long devicePtr,
      InvokeElement invokeElement,
      int timedRequestTimeoutMs,
      int imTimeoutMs) {
    InvokeCallbackJni jniCallback = new InvokeCallbackJni(callback);
    ChipInteractionClient.invoke(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        invokeElement,
        timedRequestTimeoutMs,
        imTimeoutMs);
  }

  /**
   * @brief ExtendableInvoke command to target device
   * @param ExtendableInvokeCallback Callback when invoke responses have been received and processed
   *     for the given batched invoke commands.
   * @param devicePtr connected device pointer
   * @param invokeElementList invoke element list
   * @param timedRequestTimeoutMs this is timed request if this value is larger than 0
   * @param imTimeoutMs im interaction time out value, it would override the default value in c++ im
   *     layer if this value is non-zero.
   */
  public void extendableInvoke(
      ExtendableInvokeCallback callback,
      long devicePtr,
      List<InvokeElement> invokeElementList,
      int timedRequestTimeoutMs,
      int imTimeoutMs) {
    ExtendableInvokeCallbackJni jniCallback = new ExtendableInvokeCallbackJni(callback);
    ChipInteractionClient.extendableInvoke(
        deviceControllerPtr,
        jniCallback.getCallbackHandle(),
        devicePtr,
        invokeElementList,
        timedRequestTimeoutMs,
        imTimeoutMs);
  }

  /** Create a root (self-signed) X.509 DER encoded certificate */
  public static byte[] createRootCertificate(
      KeypairDelegate keypair, long issuerId, @Nullable Long fabricId) {
    // current time
    Calendar start = Calendar.getInstance(TimeZone.getTimeZone(ZoneOffset.UTC));
    Calendar end = Calendar.getInstance(TimeZone.getTimeZone(ZoneOffset.UTC));
    // current time + 10 years
    end.add(Calendar.YEAR, 10);
    return createRootCertificate(keypair, issuerId, fabricId, start, end);
  }

  public static native byte[] createRootCertificate(
      KeypairDelegate keypair,
      long issuerId,
      @Nullable Long fabricId,
      Calendar validityStart,
      Calendar validityEnd);

  /** Create an intermediate X.509 DER encoded certificate */
  public static byte[] createIntermediateCertificate(
      KeypairDelegate rootKeypair,
      byte[] rootCertificate,
      byte[] intermediatePublicKey,
      long issuerId,
      @Nullable Long fabricId) {
    // current time
    Calendar start = Calendar.getInstance(TimeZone.getTimeZone(ZoneOffset.UTC));
    // current time + 10 years
    Calendar end = Calendar.getInstance(TimeZone.getTimeZone(ZoneOffset.UTC));
    end.add(Calendar.YEAR, 10);
    return createIntermediateCertificate(
        rootKeypair, rootCertificate, intermediatePublicKey, issuerId, fabricId, start, end);
  }

  public static native byte[] createIntermediateCertificate(
      KeypairDelegate rootKeypair,
      byte[] rootCertificate,
      byte[] intermediatePublicKey,
      long issuerId,
      @Nullable Long fabricId,
      Calendar validityStart,
      Calendar validityEnd);

  /**
   * Create an X.509 DER encoded certificate that has the right fields to be a valid Matter
   * operational certificate.
   *
   * <p>signingKeypair and signingCertificate are the root or intermediate that is signing the
   * operational certificate.
   *
   * <p>caseAuthenticatedTags may be null to indicate no CASE Authenticated Tags should be used. If
   * caseAuthenticatedTags is not null, it must contain at most 3 numbers, which are expected to be
   * 32-bit unsigned Case Authenticated Tag values.
   */
  public static byte[] createOperationalCertificate(
      KeypairDelegate signingKeypair,
      byte[] signingCertificate,
      byte[] operationalPublicKey,
      long fabricId,
      long nodeId,
      List<Integer> caseAuthenticatedTags) {
    // current time
    Calendar start = Calendar.getInstance(TimeZone.getTimeZone(ZoneOffset.UTC));
    // current time + 10 years
    Calendar end = Calendar.getInstance(TimeZone.getTimeZone(ZoneOffset.UTC));
    end.add(Calendar.YEAR, 10);
    return createOperationalCertificate(
        signingKeypair,
        signingCertificate,
        operationalPublicKey,
        fabricId,
        nodeId,
        caseAuthenticatedTags,
        start,
        end);
  }

  public static native byte[] createOperationalCertificate(
      KeypairDelegate signingKeypair,
      byte[] signingCertificate,
      byte[] operationalPublicKey,
      long fabricId,
      long nodeId,
      List<Integer> caseAuthenticatedTags,
      Calendar validityStart,
      Calendar validityEnd);

  /**
   * Extract the public key from the given PKCS#10 certificate signing request. This is the public
   * key that a certificate issued in response to the request would need to have.
   */
  public static native byte[] publicKeyFromCSR(byte[] csr);

  /**
   * Converts a given X.509v3 certificate into a Matter certificate.
   *
   * @throws ChipDeviceControllerException if there was an issue during encoding (e.g. out of
   *     memory, invalid certificate format)
   */
  public native byte[] convertX509CertToMatterCert(byte[] x509Cert);

  /**
   * Extract skid from paa cert.
   *
   * <p>This method was deprecated. Please use {@link DeviceAttestation.extractSkidFromPaaCert}.
   */
  @Deprecated
  public byte[] extractSkidFromPaaCert(byte[] paaCert) {
    return DeviceAttestation.extractSkidFromPaaCert(paaCert);
  }

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

  public static native byte[] validateAndExtractCSR(byte[] csrElements, byte[] csrNonce);

  private native PaseVerifierParams computePaseVerifier(
      long deviceControllerPtr, long devicePtr, long setupPincode, long iterations, byte[] salt);

  private native long newDeviceController(ControllerParams params);

  private native void setDeviceAttestationDelegate(
      long deviceControllerPtr, int failSafeExpiryTimeoutSecs, DeviceAttestationDelegate delegate);

  private native void setAttestationTrustStoreDelegate(
      long deviceControllerPtr,
      AttestationTrustStoreDelegate delegate,
      @Nullable List<byte[]> cdTrustKeys);

  private native void startOTAProvider(long deviceControllerPtr, OTAProviderDelegate delegate);

  private native void finishOTAProvider(long deviceControllerPtr);

  private native void setICDCheckInDelegate(
      long deviceControllerPtr, ICDCheckInDelegateWrapper delegate);

  private native void pairDevice(
      long deviceControllerPtr,
      long deviceId,
      int connectionId,
      long pinCode,
      @Nullable byte[] csrNonce,
      NetworkCredentials networkCredentials,
      @Nullable ICDRegistrationInfo icdRegistrationInfo);

  private native void pairDeviceWithAddress(
      long deviceControllerPtr,
      long deviceId,
      String address,
      int port,
      int discriminator,
      long pinCode,
      @Nullable byte[] csrNonce,
      @Nullable ICDRegistrationInfo icdRegistrationInfo);

  private native void pairDeviceWithCode(
      long deviceControllerPtr,
      long deviceId,
      String setupCode,
      boolean discoverOnce,
      boolean useOnlyOnNetworkDiscovery,
      @Nullable byte[] csrNonce,
      @Nullable NetworkCredentials networkCredentials,
      @Nullable ICDRegistrationInfo icdRegistrationInfo);

  private native void establishPaseConnection(
      long deviceControllerPtr, long deviceId, int connId, long setupPincode);

  private native void establishPaseConnectionByAddress(
      long deviceControllerPtr, long deviceId, String address, int port, long setupPincode);

  private native void establishPaseConnectionByCode(
      long deviceControllerPtr, long deviceId, String setupCode, boolean useOnlyOnNetworkDiscovery);

  private native void commissionDevice(
      long deviceControllerPtr,
      long deviceId,
      @Nullable byte[] csrNonce,
      @Nullable NetworkCredentials networkCredentials,
      @Nullable ICDRegistrationInfo icdRegistrationInfo);

  private native void continueCommissioning(
      long deviceControllerPtr, long devicePtr, boolean ignoreAttestationFailure);

  private native void unpairDevice(long deviceControllerPtr, long deviceId);

  private native void unpairDeviceCallback(
      long deviceControllerPtr, long deviceId, UnpairDeviceCallback callback);

  private native void stopDevicePairing(long deviceControllerPtr, long deviceId);

  private native long getDeviceBeingCommissionedPointer(long deviceControllerPtr, long nodeId);

  private native void getConnectedDevicePointer(
      long deviceControllerPtr, long deviceId, long callbackHandle);

  private native void releaseOperationalDevicePointer(long devicePtr);

  private native long getGroupDevicePointer(long deviceControllerPtr, int groupId);

  private native List<Integer> getAvailableGroupIds(long deviceControllerPtr);

  private native String getGroupName(long deviceControllerPtr, int groupId);

  private native Optional<Integer> findKeySetId(long deviceControllerPtr, int groupId);

  private native boolean addGroup(long deviceControllerPtr, int groupId, String groupName);

  private native boolean removeGroup(long deviceControllerPtr, int groupId);

  private native List<Integer> getKeySetIds(long deviceControllerPtr);

  private native Optional<Integer> getKeySecurityPolicy(long deviceControllerPtr, int keySetId);

  private native boolean bindKeySet(long deviceControllerPtr, int groupId, int keySetId);

  private native boolean unbindKeySet(long deviceControllerPtr, int groupId, int keySetId);

  private native boolean addKeySet(
      long deviceControllerPtr, int keySetId, int keyPolicy, long validityTime, byte[] epochKey);

  private native boolean removeKeySet(long deviceControllerPtr, int keySetId);

  private native void deleteDeviceController(long deviceControllerPtr);

  private native String getIpAddress(long deviceControllerPtr, long deviceId);

  private native NetworkLocation getNetworkLocation(long deviceControllerPtr, long deviceId);

  private native long getCompressedFabricId(long deviceControllerPtr);

  private native long getControllerNodeId(long deviceControllerPtr);

  private native void discoverCommissionableNodes(long deviceControllerPtr);

  private native DiscoveredDevice getDiscoveredDevice(long deviceControllerPtr, int idx);

  private native boolean openPairingWindow(long deviceControllerPtr, long devicePtr, int duration);

  private native boolean openPairingWindowWithPIN(
      long deviceControllerPtr,
      long devicePtr,
      int duration,
      long iteration,
      int discriminator,
      @Nullable Long setupPinCode);

  private native boolean openPairingWindowCallback(
      long deviceControllerPtr, long devicePtr, int duration, OpenCommissioningCallback callback);

  private native boolean openPairingWindowWithPINCallback(
      long deviceControllerPtr,
      long devicePtr,
      int duration,
      long iteration,
      int discriminator,
      @Nullable Long setupPinCode,
      OpenCommissioningCallback callback);

  private native byte[] getAttestationChallenge(long deviceControllerPtr, long devicePtr);

  private native void setUseJavaCallbackForNOCRequest(
      long deviceControllerPtr, boolean useCallback);

  private native void updateCommissioningNetworkCredentials(
      long deviceControllerPtr, NetworkCredentials networkCredentials);

  private native void updateCommissioningICDRegistrationInfo(
      long deviceControllerPtr, ICDRegistrationInfo icdRegistrationInfo);

  private native long onNOCChainGeneration(long deviceControllerPtr, ControllerParams params);

  private native int getFabricIndex(long deviceControllerPtr);

  private native void shutdownCommissioning(long deviceControllerPtr);

  private native void startDnssd(long deviceControllerPtr);

  private native void stopDnssd(long deviceControllerPtr);

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
    void onScanNetworksFailure(long errorCode);

    void onScanNetworksSuccess(
        Integer networkingStatus,
        Optional<String> debugText,
        Optional<ArrayList<WiFiScanResult>> wiFiScanResults,
        Optional<ArrayList<ThreadScanResult>> threadScanResults);
  }

  /** Interface to listen for callbacks from CHIPDeviceController. */
  public interface CompletionListener {

    /** Notifies the completion of "ConnectDevice" command. */
    void onConnectDeviceComplete();

    /** Notifies the pairing status. */
    void onStatusUpdate(int status);

    /** Notifies the completion of pairing. */
    void onPairingComplete(long errorCode);

    /** Notifies the deletion of pairing session. */
    void onPairingDeleted(long errorCode);

    /** Notifies the completion of commissioning. */
    void onCommissioningComplete(long nodeId, long errorCode);

    /** Notifies the completion of each stage of commissioning. */
    void onReadCommissioningInfo(
        int vendorId, int productId, int wifiEndpointId, int threadEndpointId);

    /** Notifies the completion of each stage of commissioning. */
    void onCommissioningStatusUpdate(long nodeId, String stage, long errorCode);

    /** Notifies that the Chip connection has been closed. */
    void onNotifyChipConnectionClosed();

    /** Notifies the completion of the "close BLE connection" command. */
    void onCloseBleComplete();

    /** Notifies the listener of the error. */
    void onError(Throwable error);

    /** Notifies the Commissioner when the OpCSR for the Comissionee is generated. */
    void onOpCSRGenerationComplete(byte[] csr);

    /**
     * Notifies when the ICD registration information (ICD symmetric key, check-in node ID and
     * monitored subject) is required.
     */
    void onICDRegistrationInfoRequired();

    /** Notifies when the registration flow for the ICD completes. */
    void onICDRegistrationComplete(long errorCode, ICDDeviceInfo icdDeviceInfo);
  }
}
