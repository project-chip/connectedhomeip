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

package com.matter.controller;

import chip.devicecontroller.ChipDeviceController;
import chip.devicecontroller.ControllerParams;
import com.matter.controller.commands.common.*;
import com.matter.controller.commands.discover.*;
import com.matter.controller.commands.pairing.*;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Main {
  private static Logger logger = Logger.getLogger(Main.class.getName());

  private static void registerCommandsDiscover(
      ChipDeviceController controller,
      CommandManager commandManager,
      CredentialsIssuer credentialsIssuer) {
    ArrayList<Command> clusterCommands = new ArrayList<Command>();
    DiscoverCommand discoverCommand = new DiscoverCommand(controller, credentialsIssuer);
    DiscoverCommissionablesCommand discoverCommissionablesCommand =
        new DiscoverCommissionablesCommand(controller, credentialsIssuer);
    DiscoverCommissionersCommand discoverCommissionersCommand =
        new DiscoverCommissionersCommand(controller, credentialsIssuer);
    clusterCommands.add(discoverCommand);
    clusterCommands.add(discoverCommissionablesCommand);
    clusterCommands.add(discoverCommissionersCommand);

    commandManager.register("discover", clusterCommands);
  }

  private static void registerCommandsPairing(
      ChipDeviceController controller,
      CommandManager commandManager,
      CredentialsIssuer credentialsIssuer) {
    ArrayList<Command> clusterCommands = new ArrayList<Command>();
    UnpairCommand unpairCommand = new UnpairCommand(controller, credentialsIssuer);
    PairCodeCommand pairCodeCommand = new PairCodeCommand(controller, credentialsIssuer);
    PairCodePaseCommand pairCodePaseCommand =
        new PairCodePaseCommand(controller, credentialsIssuer);
    PairCodeWifiCommand pairCodeWifiCommand =
        new PairCodeWifiCommand(controller, credentialsIssuer);
    PairCodeThreadCommand pairCodeThreadCommand =
        new PairCodeThreadCommand(controller, credentialsIssuer);
    PairEthernetCommand pairEthernetCommand =
        new PairEthernetCommand(controller, credentialsIssuer);
    PairOnNetworkCommand pairOnNetworkCommand =
        new PairOnNetworkCommand(controller, credentialsIssuer);
    PairOnNetworkShortCommand pairOnNetworkShortCommand =
        new PairOnNetworkShortCommand(controller, credentialsIssuer);
    PairOnNetworkLongCommand pairOnNetworkLongCommand =
        new PairOnNetworkLongCommand(controller, credentialsIssuer);
    PairOnNetworkVendorCommand pairOnNetworkVendorCommand =
        new PairOnNetworkVendorCommand(controller, credentialsIssuer);
    PairOnNetworkCommissioningModeCommand pairOnNetworkCommissioningModeCommand =
        new PairOnNetworkCommissioningModeCommand(controller, credentialsIssuer);
    PairOnNetworkCommissionerCommand pairOnNetworkCommissionerCommand =
        new PairOnNetworkCommissionerCommand(controller, credentialsIssuer);
    PairOnNetworkDeviceTypeCommand pairOnNetworkDeviceTypeCommand =
        new PairOnNetworkDeviceTypeCommand(controller, credentialsIssuer);
    PairOnNetworkInstanceNameCommand pairOnNetworkInstanceNameCommand =
        new PairOnNetworkInstanceNameCommand(controller, credentialsIssuer);
    clusterCommands.add(unpairCommand);
    clusterCommands.add(pairCodeCommand);
    clusterCommands.add(pairCodePaseCommand);
    clusterCommands.add(pairCodeWifiCommand);
    clusterCommands.add(pairCodeThreadCommand);
    clusterCommands.add(pairEthernetCommand);
    clusterCommands.add(pairOnNetworkCommand);
    clusterCommands.add(pairOnNetworkShortCommand);
    clusterCommands.add(pairOnNetworkLongCommand);
    clusterCommands.add(pairOnNetworkVendorCommand);
    clusterCommands.add(pairOnNetworkCommissioningModeCommand);
    clusterCommands.add(pairOnNetworkCommissionerCommand);
    clusterCommands.add(pairOnNetworkDeviceTypeCommand);
    clusterCommands.add(pairOnNetworkInstanceNameCommand);

    commandManager.register("pairing", clusterCommands);
  }

  public static void main(String[] args) {
    ChipDeviceController controller =
        new ChipDeviceController(
            ControllerParams.newBuilder()
                .setUdpListenPort(0)
                .setControllerVendorId(0xFFF1)
                .build());

    CredentialsIssuer credentialsIssuer = new CredentialsIssuer();
    CommandManager commandManager = new CommandManager();

    registerCommandsDiscover(controller, commandManager, credentialsIssuer);
    registerCommandsPairing(controller, commandManager, credentialsIssuer);

    try {
      commandManager.run(args);
    } catch (IllegalArgumentException e) {
      logger.log(Level.INFO, "Arguments init failed with exception: " + e.getMessage());
      System.exit(1);
    } catch (Exception e) {
      logger.log(Level.INFO, "Run command failed with exception: " + e.getMessage());
      System.exit(1);
    }
    controller.shutdownCommissioning();
  }
}
