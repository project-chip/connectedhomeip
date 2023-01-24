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

package com.matter.controller.commands.discover;

import chip.devicecontroller.ChipDeviceController;
import chip.devicecontroller.DiscoveredDevice;
import com.matter.controller.commands.common.CredentialsIssuer;
import com.matter.controller.commands.common.MatterCommand;

public final class DiscoverCommissionablesCommand extends MatterCommand {
  private static final int MAX_DISCOVERED_DEVICES = 10;

  public DiscoverCommissionablesCommand(
      ChipDeviceController controller, CredentialsIssuer credsIssuer) {
    super(controller, "commissionables", credsIssuer);
  }

  @Override
  protected final void runCommand() {
    currentCommissioner().discoverCommissionableNodes();

    // Pause for 7 seconds
    try {
      Thread.sleep(7000);
    } catch (InterruptedException e) {
      throw new RuntimeException(e);
    }

    getDiscoveredDevice();
  }

  private final void getDiscoveredDevice() {
    // Log at most MAX_DISCOVERED_DEVICES discovered devices
    for (int i = 0; i < MAX_DISCOVERED_DEVICES; i++) {
      DiscoveredDevice device = currentCommissioner().getDiscoveredDevice(i);
      if (device == null) {
        break;
      }

      logDevice(device);
    }
  }

  private final void logDevice(DiscoveredDevice device) {
    System.out.println("Discovered node:");
    System.out.format("\tDiscriminator: %d", device.discriminator);
    System.out.format("\tIP Address : %s%n", device.ipAddress);
  }
}
