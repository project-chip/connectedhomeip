/*
 *   Copyright (c) 2024 Project CHIP Authors
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
 */
package com.matter.casting.core;

import android.util.Log;
import com.matter.casting.support.ConnectionCallbacks;
import com.matter.casting.support.IdentificationDeclarationOptions;
import com.matter.casting.support.MatterError;
import java.net.InetAddress;
import java.util.List;
import java.util.Objects;

/**
 * A Matter Casting Player represents a Matter Commissioner that is able to play media to a physical
 * output or to a display screen which is part of the device (e.g. TV). It is discovered on the
 * local network using Matter Commissioner discovery over DNS. It contains all the information about
 * the service discovered/resolved.
 */
public class MatterCastingPlayer implements CastingPlayer {
  private static final String TAG = MatterCastingPlayer.class.getSimpleName();

  /**
   * Time (in sec) to keep the commissioning window open, if commissioning is required. Must be >= 3
   * minutes.
   */
  public static final short MIN_CONNECTION_TIMEOUT_SEC = 3 * 60;

  private boolean connected;
  private String deviceId;
  private String deviceName;
  private String hostName;
  private String instanceName;
  private List<InetAddress> ipAddresses;
  private int port;
  private int productId;
  private int vendorId;
  private long deviceType;
  private boolean supportsCommissionerGeneratedPasscode;

  protected long _cppCastingPlayer;

  public MatterCastingPlayer(
      boolean connected,
      String deviceId,
      String hostName,
      String deviceName,
      String instanceName,
      List<InetAddress> ipAddresses,
      int port,
      int productId,
      int vendorId,
      long deviceType,
      boolean supportsCommissionerGeneratedPasscode) {
    this.connected = connected;
    this.deviceId = deviceId;
    this.hostName = hostName;
    this.deviceName = deviceName;
    this.instanceName = instanceName;
    this.ipAddresses = ipAddresses;
    this.port = port;
    this.productId = productId;
    this.vendorId = vendorId;
    this.deviceType = deviceType;
    this.supportsCommissionerGeneratedPasscode = supportsCommissionerGeneratedPasscode;
  }

  /**
   * @return a boolean indicating whether a Casting Player instance is connected to the TV Casting
   *     App.
   */
  @Override
  public boolean isConnected() {
    return this.connected;
  }

  /**
   * @return a String representing the Casting Player device ID which is a concatenation of the
   *     device's IP address and port number.
   */
  @Override
  public String getDeviceId() {
    return this.deviceId;
  }

  @Override
  public String getHostName() {
    return this.hostName;
  }

  @Override
  public String getDeviceName() {
    return this.deviceName;
  }

  @Override
  public String getInstanceName() {
    return this.instanceName;
  }

  /** @return a list of valid IP addresses for this Casting PLayer. */
  @Override
  public List<InetAddress> getIpAddresses() {
    return this.ipAddresses;
  }

  @Override
  public int getPort() {
    return this.port;
  }

  @Override
  public int getVendorId() {
    return this.vendorId;
  }

  @Override
  public int getProductId() {
    return this.productId;
  }

  @Override
  public long getDeviceType() {
    return this.deviceType;
  }

  @Override
  public boolean getSupportsCommissionerGeneratedPasscode() {
    return this.supportsCommissionerGeneratedPasscode;
  }

  @Override
  public native List<Endpoint> getEndpoints();

  @Override
  public String toString() {
    return this.deviceId;
  }

  @Override
  public int hashCode() {
    return this.deviceId.hashCode();
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    MatterCastingPlayer that = (MatterCastingPlayer) o;
    return Objects.equals(this.deviceId, that.deviceId);
  }

  /**
   * @brief Verifies that a connection exists with this CastingPlayer, or triggers a new
   *     commissioning session request. If the CastingApp does not have the nodeId and fabricIndex
   *     of this CastingPlayer cached on disk, this will execute the User Directed Commissioning
   *     (UDC) process by sending an IdentificationDeclaration message to the Commissioner. For
   *     certain UDC features, where a Commissioner reply is expected, this API needs to be followed
   *     up with the continueConnecting() API defiend below. See the Matter UDC specification or
   *     parameter class definitions for details on features not included in the description below.
   * @param connectionCallbacks contains the onSuccess (Required), onFailure (Required) and
   *     onCommissionerDeclaration (Optional) callbacks defiend in ConnectCallbacks.java.
   *     <p>For example: During CastingPlayer/Commissioner-Generated passcode commissioning, the
   *     Commissioner replies with a CommissionerDeclaration message with PasscodeDialogDisplayed
   *     and CommissionerPasscode set to true. Given these Commissioner state details, the client is
   *     expected to perform some actions, detailed in the continueConnecting() API below, and then
   *     call the continueConnecting() API to complete the process.
   * @param commissioningWindowTimeoutSec (Optional) time (in sec) to keep the commissioning window
   *     open, if commissioning is required. Needs to be >= kCommissioningWindowTimeoutSec.
   * @param idOptions (Optional) Parameters in the IdentificationDeclaration message sent by the
   *     Commissionee to the Commissioner. These parameters specify the information relating to the
   *     requested commissioning session.
   *     <p>For example: To invoke the CastingPlayer/Commissioner-Generated passcode commissioning
   *     flow, the client would call this API with IdentificationDeclarationOptions containing
   *     CommissionerPasscode set to true. See IdentificationDeclarationOptions.java for a complete
   *     list of optional parameters.
   *     <p>Furthermore, attributes (such as VendorId) describe the TargetApp that the client wants
   *     to interact with after commissioning. If this value is passed in,
   *     verifyOrEstablishConnection() will force UDC, in case the desired TargetApp is not found in
   *     the on-device CastingStore.
   * @return MatterError - MatterError.NO_ERROR if request submitted successfully, otherwise a
   *     MatterError object corresponding to the error.
   */
  @Override
  public native MatterError verifyOrEstablishConnection(
      ConnectionCallbacks connectionCallbacks,
      short commissioningWindowTimeoutSec,
      IdentificationDeclarationOptions idOptions);

  /**
   * The simplified version of the verifyOrEstablishConnection() API above.
   *
   * @param connectionCallbacks contains the onSuccess (Required), onFailure (Required) and
   *     onCommissionerDeclaration (Optional) callbacks defiend in ConnectCallbacks.java.
   * @return MatterError - MatterError.NO_ERROR if request submitted successfully, otherwise a
   *     MatterError object corresponding to the error.
   */
  @Override
  public MatterError verifyOrEstablishConnection(ConnectionCallbacks connectionCallbacks) {
    Log.d(TAG, "verifyOrEstablishConnection() (ConnectionCallbacks)");
    return verifyOrEstablishConnection(connectionCallbacks, MIN_CONNECTION_TIMEOUT_SEC, null);
  }

  /**
   * @brief This is a continuation of the CastingPlayer/Commissioner-Generated passcode
   *     commissioning flow started via the verifyOrEstablishConnection() API above. It continues
   *     the UDC process by sending a second IdentificationDeclaration message to Commissioner
   *     containing CommissionerPasscode and CommissionerPasscodeReady set to true. At this point it
   *     is assumed that the following have occurred:
   *     <p>1. Client (Commissionee) has sent the first IdentificationDeclaration message, via
   *     verifyOrEstablishConnection(), to the Commissioner containing CommissionerPasscode set to
   *     true.
   *     <p>2. Commissioner generated and displayed a passcode.
   *     <p>3. The Commissioner replied with a CommissionerDecelration message with
   *     PasscodeDialogDisplayed and CommissionerPasscode set to true.
   *     <p>4. Client has handled the Commissioner's CommissionerDecelration message.
   *     <p>5. Client prompted user to input Passcode from Commissioner.
   *     <p>6. Client has updated the CastingApp's AppParameters DataProvider<CommissionableData>
   *     via the following function call: DataProvider.updateCommissionableDataSetupPasscode(long
   *     setupPasscode, int discriminator). This allows continueConnecting() to update the
   *     commissioning session's PAKE verifier with the user entered passcode.
   *     <p>Note: The same connectionCallbacks and commissioningWindowTimeoutSec parameters passed
   *     into verifyOrEstablishConnection() will be used.
   * @return MatterError - MatterError.NO_ERROR if request submitted successfully, otherwise a
   *     MatterError object corresponding to the error.
   */
  public MatterError continueConnecting() {
    Log.d(TAG, "continueConnecting()");
    // Update AndroidChipPlatform's CommissionableData with the user entered passcode.
    MatterError err = CastingApp.getInstance().updateAndroidChipPlatformWithCommissionableData();
    if (err != MatterError.NO_ERROR) {
      Log.e(TAG, "continueConnecting() Error updating AndroidChipPlatform with CommissionableData");
      return err;
    }
    Log.d(TAG, "continueConnecting() calling continueConnectingNative()");
    return continueConnectingNative();
  }

  public native MatterError continueConnectingNative();

  /**
   * @brief This cancels the CastingPlayer/Commissioner-Generated passcode commissioning flow
   *     started via the VerifyOrEstablishConnection() API above. It constructs and sends an
   *     IdentificationDeclaration message to the CastingPlayer/Commissioner containing
   *     CancelPasscode set to true. It is used to indicate that the user, and thus the
   *     Client/Commissionee, have cancelled the commissioning process. This indicates that the
   *     CastingPlayer/Commissioner can dismiss any dialogs corresponding to commissioning, such as
   *     a Passcode input dialog or a Passcode display dialog.
   *     <p>Note: stopConnecting() does not call the onSuccess() callback passed to the
   *     VerifyOrEstablishConnection() API above since no connection is established.
   * @return MatterError - MatterError.NO_ERROR if request submitted successfully, otherwise a
   *     MatterError object corresponding to the error.
   */
  public native MatterError stopConnecting();

  @Override
  public native void disconnect();

  /**
   * @brief Get CastingPlayer's current ConnectionState.
   * @throws IllegalArgumentException or NullPointerException when native layer returns invalid
   *     state.
   * @return Current ConnectionState.
   */
  @Override
  public ConnectionState getConnectionState() {
    return ConnectionState.valueOf(getConnectionStateNative());
  }

  /*
   * @brief Get the Current ConnectionState of a CastingPlayer from the native layer.
   *
   * @returns A String representation of the CastingPlayer's current connectation.
   *          Possible return values are
   *            - "NOT_CONNECTED"
   *            - "CONNECTING"
   *            - "CONNECTED"
   *            - Error message or NULL on error
   */
  @Override
  public native String getConnectionStateNative();
}
