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
import com.matter.controller.commands.common.Command;
import com.matter.controller.commands.common.CredentialsIssuer;
import java.util.Arrays;

public class Main {
  private static void ShowUsage(Command[] commands) {
    StringBuffer arguments = new StringBuffer();
    StringBuffer attributes = new StringBuffer();

    for (Command command : commands) {
      arguments.append("    ");
      arguments.append(command.getName());

      int argumentsCount = command.getArgumentsCount();
      for (int j = 0; j < argumentsCount; j++) {
        arguments.append(" ");
        arguments.append(command.getArgumentName(j));
      }

      arguments.append("\n");

      if ("read".equals(command.getName()) && command.getAttribute().isPresent()) {
        attributes.append("    " + command.getAttribute().get() + "\n");
      }
    }

    System.out.println(
        String.format(
            "Usage: \n"
                + "  java_matter_controller command [params]\n\n"
                + "  Supported commands and their parameters:\n%s\n"
                + "  Supported attribute names for the 'read' command:\n%s",
            arguments, attributes));
  }

  private static void runCommand(CredentialsIssuer credIssuerCmds, String[] args) {

    // TODO::Start list of available commands, this hard coded list need to be replaced by command
    // registration mechanism.
    Command[] commands = {new On(credIssuerCmds), new Off(credIssuerCmds)};
    // End list of available commands

    if (args.length == 0) {
      ShowUsage(commands);
      return;
    }

    for (Command cmd : commands) {
      if (cmd.getName().equals(args[0])) {
        String[] temp = Arrays.copyOfRange(args, 1, args.length);

        try {
          cmd.initArguments(args.length - 1, temp);
          cmd.run();
        } catch (IllegalArgumentException e) {
          System.out.println("Arguments init failed with exception: " + e.getMessage());
        } catch (Exception e) {
          System.out.println("Run command failed with exception: " + e.getMessage());
        }
        break;
      }
    }
  }

  public static void main(String[] args) {
    ChipDeviceController controller =
        new ChipDeviceController(
            ControllerParams.newBuilder()
                .setUdpListenPort(0)
                .setControllerVendorId(0xFFF1)
                .build());

    CredentialsIssuer credentialsIssuer = new CredentialsIssuer();

    runCommand(credentialsIssuer, args);
  }
}
