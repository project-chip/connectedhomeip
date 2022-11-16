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

import com.matter.controller.commands.common.*;
import com.matter.controller.commands.discover.*;
import com.matter.controller.commands.pairing.*;
import java.util.ArrayList;

public class Main {
  private static void registerCommandsDiscover(
      CommandManager commandManager, CredentialsIssuer credentialsIssuer) {
    ArrayList<Command> clusterCommands = new ArrayList<Command>();
    DiscoverCommand discoverCommand = new DiscoverCommand(credentialsIssuer);
    DiscoverCommissionablesCommand discoverCommissionablesCommand =
        new DiscoverCommissionablesCommand(credentialsIssuer);
    DiscoverCommissionersCommand discoverCommissionersCommand =
        new DiscoverCommissionersCommand(credentialsIssuer);
    clusterCommands.add(discoverCommand);
    clusterCommands.add(discoverCommissionablesCommand);
    clusterCommands.add(discoverCommissionersCommand);

    commandManager.register("discover", clusterCommands);
  }

  private static void registerCommandsPairing(
      CommandManager commandManager, CredentialsIssuer credentialsIssuer) {
    ArrayList<Command> clusterCommands = new ArrayList<Command>();
    UnpairCommand unpairCommand = new UnpairCommand(credentialsIssuer);
    PairCodeCommand pairCodeCommand = new PairCodeCommand(credentialsIssuer);
    PairCodePaseCommand pairCodePaseCommand = new PairCodePaseCommand(credentialsIssuer);
    PairCodeWifiCommand pairCodeWifiCommand = new PairCodeWifiCommand(credentialsIssuer);
    PairCodeThreadCommand pairCodeThreadCommand = new PairCodeThreadCommand(credentialsIssuer);
    PairEthernetCommand pairEthernetCommand = new PairEthernetCommand(credentialsIssuer);
    PairOnNetworkCommand pairOnNetworkCommand = new PairOnNetworkCommand(credentialsIssuer);
    PairOnNetworkShortCommand pairOnNetworkShortCommand =
        new PairOnNetworkShortCommand(credentialsIssuer);
    PairOnNetworkLongCommand pairOnNetworkLongCommand =
        new PairOnNetworkLongCommand(credentialsIssuer);
    PairOnNetworkVendorCommand pairOnNetworkVendorCommand =
        new PairOnNetworkVendorCommand(credentialsIssuer);
    PairOnNetworkCommissioningModeCommand pairOnNetworkCommissioningModeCommand =
        new PairOnNetworkCommissioningModeCommand(credentialsIssuer);
    PairOnNetworkCommissionerCommand pairOnNetworkCommissionerCommand =
        new PairOnNetworkCommissionerCommand(credentialsIssuer);
    PairOnNetworkDeviceTypeCommand pairOnNetworkDeviceTypeCommand =
        new PairOnNetworkDeviceTypeCommand(credentialsIssuer);
    PairOnNetworkInstanceNameCommand pairOnNetworkInstanceNameCommand =
        new PairOnNetworkInstanceNameCommand(credentialsIssuer);
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
    /* TODO: uncomment when SDK integration is done
    ChipDeviceController controller =
        new ChipDeviceController(
            ControllerParams.newBuilder()
                .setUdpListenPort(0)
                .setControllerVendorId(0xFFF1)
                .build());
    */

    CredentialsIssuer credentialsIssuer = new CredentialsIssuer();
    CommandManager commandManager = new CommandManager();

    registerCommandsDiscover(commandManager, credentialsIssuer);
    registerCommandsPairing(commandManager, credentialsIssuer);

    try {
      commandManager.run(args);
    } catch (IllegalArgumentException e) {
      System.out.println("Arguments init failed with exception: " + e.getMessage());
    } catch (Exception e) {
      System.out.println("Run command failed with exception: " + e.getMessage());
    }
  }
}
