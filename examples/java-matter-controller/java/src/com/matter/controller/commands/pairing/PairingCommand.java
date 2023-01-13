/*
 *   Copyright (c) 2022 Project CHIP Authors
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

package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;
import com.matter.controller.commands.common.IPAddress;
import com.matter.controller.commands.common.MatterCommand;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import java.util.logging.Level;
import java.util.logging.Logger;

public abstract class PairingCommand extends MatterCommand
    implements ChipDeviceController.CompletionListener {
  private PairingModeType mPairingMode = PairingModeType.NONE;
  private PairingNetworkType mNetworkType = PairingNetworkType.NONE;
  private DiscoveryFilterType mFilterType = DiscoveryFilterType.NONE;
  private final IPAddress mRemoteAddr;
  private final AtomicLong mNodeId = new AtomicLong();
  private final AtomicLong mDiscoveryFilterCode = new AtomicLong();
  private final AtomicLong mTimeoutMillis = new AtomicLong();
  private final AtomicBoolean mDiscoverOnce = new AtomicBoolean();
  private final AtomicBoolean mUseOnlyOnNetworkDiscovery = new AtomicBoolean();
  private final AtomicInteger mRemotePort = new AtomicInteger();
  private final AtomicInteger mDiscriminator = new AtomicInteger();
  private final AtomicInteger mSetupPINCode = new AtomicInteger();
  private final StringBuffer mOperationalDataset = new StringBuffer();
  private final StringBuffer mSSID = new StringBuffer();
  private final StringBuffer mPassword = new StringBuffer();
  private final StringBuffer mOnboardingPayload = new StringBuffer();
  private final StringBuffer mDiscoveryFilterInstanceName = new StringBuffer();

  private static Logger logger = Logger.getLogger(PairingCommand.class.getName());

  public long getNodeId() {
    return mNodeId.get();
  }

  public int getSetupPINCode() {
    return mSetupPINCode.get();
  }

  public int getDiscriminator() {
    return mDiscriminator.get();
  }

  public long getTimeoutMillis() {
    return mTimeoutMillis.get();
  }

  @Override
  public void onConnectDeviceComplete() {
    logger.log(Level.INFO, "onConnectDeviceComplete");
  }

  @Override
  public void onStatusUpdate(int status) {
    logger.log(Level.INFO, "onStatusUpdate with status: " + status);
  }

  @Override
  public void onPairingComplete(int errorCode) {
    logger.log(Level.INFO, "onPairingComplete with error code: " + errorCode);
    if (errorCode != 0) {
      setFailure("onPairingComplete failure");
    }
  }

  @Override
  public void onPairingDeleted(int errorCode) {
    logger.log(Level.INFO, "onPairingDeleted with error code: " + errorCode);
  }

  @Override
  public void onCommissioningComplete(long nodeId, int errorCode) {
    logger.log(Level.INFO, "onCommissioningComplete with error code: " + errorCode);
    if (errorCode == 0) {
      setSuccess();
    } else {
      setFailure("onCommissioningComplete failure");
    }
  }

  @Override
  public void onReadCommissioningInfo(
      int vendorId, int productId, int wifiEndpointId, int threadEndpointId) {
    logger.log(Level.INFO, "onReadCommissioningInfo");
  }

  @Override
  public void onCommissioningStatusUpdate(long nodeId, String stage, int errorCode) {
    logger.log(Level.INFO, "onCommissioningStatusUpdate");
  }

  @Override
  public void onNotifyChipConnectionClosed() {
    logger.log(Level.INFO, "onNotifyChipConnectionClosed");
  }

  @Override
  public void onCloseBleComplete() {
    logger.log(Level.INFO, "onCloseBleComplete");
  }

  @Override
  public void onError(Throwable error) {
    setFailure(error.toString());
    logger.log(Level.INFO, "onError with error: " + error.toString());
  }

  @Override
  public void onOpCSRGenerationComplete(byte[] csr) {
    logger.log(Level.INFO, "onOpCSRGenerationComplete");
    for (int i = 0; i < csr.length; i++) {
      System.out.print(csr[i] + " ");
    }
  }

  public IPAddress getRemoteAddr() {
    return mRemoteAddr;
  }

  public PairingCommand(
      ChipDeviceController controller,
      String commandName,
      PairingModeType mode,
      PairingNetworkType networkType,
      CredentialsIssuer credsIssuer) {
    this(controller, commandName, mode, networkType, credsIssuer, DiscoveryFilterType.NONE);
  }

  public PairingCommand(
      ChipDeviceController controller,
      String commandName,
      PairingModeType mode,
      PairingNetworkType networkType,
      CredentialsIssuer credsIssuer,
      DiscoveryFilterType filterType) {
    super(controller, commandName, credsIssuer);
    this.mPairingMode = mode;
    this.mNetworkType = networkType;
    this.mFilterType = filterType;

    try {
      this.mRemoteAddr = new IPAddress(InetAddress.getByName("::1"));
    } catch (UnknownHostException e) {
      throw new RuntimeException(e);
    }

    addArgument("node-id", 0, Long.MAX_VALUE, mNodeId, null, false);

    switch (networkType) {
      case NONE:
      case ETHERNET:
        break;
      case WIFI:
        addArgument("ssid", mSSID, null, false);
        addArgument("password", mPassword, null, false);
        break;
      case THREAD:
        addArgument("operationalDataset", mOperationalDataset, null, false);
        break;
    }

    switch (mode) {
      case NONE:
        break;
      case CODE:
      case CODE_PASE_ONLY:
        Only:
        addArgument("payload", mOnboardingPayload, null, false);
        addArgument("discover-once", mDiscoverOnce, null, false);
        addArgument("use-only-onnetwork-discovery", mUseOnlyOnNetworkDiscovery, null, false);
        break;
      case BLE:
        addArgument("setup-pin-code", 0, 134217727, mSetupPINCode, null, false);
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null, false);
        break;
      case ON_NETWORK:
        addArgument("setup-pin-code", 0, 134217727, mSetupPINCode, null, false);
        break;
      case SOFT_AP:
        AP:
        addArgument("setup-pin-code", 0, 134217727, mSetupPINCode, null, false);
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null, false);
        addArgument("device-remote-ip", mRemoteAddr, false);
        addArgument("device-remote-port", (short) 0, Short.MAX_VALUE, mRemotePort, null, false);
        break;
      case ETHERNET:
        addArgument("setup-pin-code", 0, 134217727, mSetupPINCode, null, false);
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null, false);
        addArgument("device-remote-ip", mRemoteAddr, false);
        addArgument("device-remote-port", (short) 0, Short.MAX_VALUE, mRemotePort, null, false);
        break;
    }

    switch (filterType) {
      case NONE:
        break;
      case SHORT_DISCRIMINATOR:
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null, false);
        break;
      case LONG_DISCRIMINATOR:
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null, false);
        break;
      case VENDOR_ID:
        addArgument("vendor-id", (short) 0, Short.MAX_VALUE, mDiscoveryFilterCode, null, false);
        break;
      case COMPRESSED_FABRIC_ID:
        addArgument("fabric-id", 0, Long.MAX_VALUE, mDiscoveryFilterCode, null, false);
        break;
      case COMMISSIONING_MODE:
      case COMMISSIONER:
        break;
      case DEVICE_TYPE:
        addArgument("device-type", (short) 0, Short.MAX_VALUE, mDiscoveryFilterCode, null, false);
        break;
      case INSTANCE_NAME:
        addArgument("name", mDiscoveryFilterInstanceName, null, false);
        break;
    }

    addArgument("timeout", (long) 0, Long.MAX_VALUE, mTimeoutMillis, null, false);
  }
}
