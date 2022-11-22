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

public abstract class PairingCommand extends MatterCommand {
  private PairingModeType mPairingMode = PairingModeType.NONE;
  private PairingNetworkType mNetworkType = PairingNetworkType.NONE;
  private DiscoveryFilterType mFilterType = DiscoveryFilterType.NONE;
  private final IPAddress mRemoteAddr;
  private final AtomicLong mNodeId = new AtomicLong();
  private final AtomicLong mDiscoveryFilterCode = new AtomicLong();
  private final AtomicInteger mTimeout = new AtomicInteger();
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
      this.mRemoteAddr = new IPAddress(InetAddress.getByName("0.0.0.0"));
    } catch (UnknownHostException e) {
      throw new RuntimeException(e);
    }

    addArgument("node-id", 0, Long.MAX_VALUE, mNodeId, null);

    switch (networkType) {
      case NONE:
      case ETHERNET:
        break;
      case WIFI:
        addArgument("ssid", mSSID, null);
        addArgument("password", mPassword, null);
        break;
      case THREAD:
        addArgument("operationalDataset", mOperationalDataset, null);
        break;
    }

    switch (mode) {
      case NONE:
        break;
      case CODE:
      case CODE_PASE_ONLY:
        Only:
        addArgument("payload", mOnboardingPayload, null);
        addArgument("discover-once", mDiscoverOnce, null);
        addArgument("use-only-onnetwork-discovery", mUseOnlyOnNetworkDiscovery, null);
        break;
      case BLE:
        addArgument("setup-pin-code", 0, 134217727, mSetupPINCode, null);
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null);
        break;
      case ON_NETWORK:
        addArgument("setup-pin-code", 0, 134217727, mSetupPINCode, null);
        break;
      case SOFT_AP:
        AP:
        addArgument("setup-pin-code", 0, 134217727, mSetupPINCode, null);
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null);
        addArgument("device-remote-ip", mRemoteAddr);
        addArgument("device-remote-port", (short) 0, Short.MAX_VALUE, mRemotePort, null);
        break;
      case ETHERNET:
        addArgument("setup-pin-code", 0, 134217727, mSetupPINCode, null);
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null);
        addArgument("device-remote-ip", mRemoteAddr);
        addArgument("device-remote-port", (short) 0, Short.MAX_VALUE, mRemotePort, null);
        break;
    }

    switch (filterType) {
      case NONE:
        break;
      case SHORT_DISCRIMINATOR:
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null);
        break;
      case LONG_DISCRIMINATOR:
        addArgument("discriminator", (short) 0, (short) 4096, mDiscriminator, null);
        break;
      case VENDOR_ID:
        addArgument("vendor-id", (short) 0, Short.MAX_VALUE, mDiscoveryFilterCode, null);
        break;
      case COMPRESSED_FABRIC_ID:
        addArgument("fabric-id", 0, Long.MAX_VALUE, mDiscoveryFilterCode, null);
        break;
      case COMMISSIONING_MODE:
      case COMMISSIONER:
        break;
      case DEVICE_TYPE:
        addArgument("device-type", (short) 0, Short.MAX_VALUE, mDiscoveryFilterCode, null);
        break;
      case INSTANCE_NAME:
        addArgument("name", mDiscoveryFilterInstanceName, null);
        break;
    }

    addArgument("timeout", (short) 0, Short.MAX_VALUE, mTimeout, null);
  }
}
