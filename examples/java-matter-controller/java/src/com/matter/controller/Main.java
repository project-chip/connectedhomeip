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
import com.matter.controller.commands.common.ChipDeviceController;
import com.matter.controller.commands.common.Command;
import com.matter.controller.commands.common.CredentialIssuerCommands;
import java.util.Arrays;

public class Main {
  private static void ShowUsage(Command[] commands) {
    String arguments = "";
    String attributes = "";

    for (int i = 0; i < commands.length; i++) {
      Command command = commands[i];

      arguments += "    ";
      arguments += command.getName();

      int argumentsCount = command.getArgumentsCount();
      for (int j = 0; j < argumentsCount; j++) {
        arguments += " ";
        arguments += command.getArgumentName(j);
      }

      arguments += "\n";

      if ("read" == command.getName()) {
        attributes += "    " + command.getAttribute() + "\n";
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

  private static int runCommand(
      ChipDeviceController dc, CredentialIssuerCommands credIssuerCmds, String[] args) {
    // Start list of available commands
    On on = new On(credIssuerCmds);
    Off off = new Off(credIssuerCmds);

    Command[] commands = {on, off};
    // End list of available commands

    if (args.length == 0) {
      ShowUsage(commands);
      return 0;
    }

    int err = 0;

    for (int i = 0; i < commands.length; i++) {
      Command cmd = commands[i];

      if (cmd.getName().equals(args[0])) {
        String[] temp = Arrays.copyOfRange(args, 1, args.length);

        if (cmd.initArguments(args.length - 1, temp) == false) {
          System.out.println("Invalid arguments number");
          err = -1;
        }

        if (cmd.run() != 0) {
          System.out.println("run command failure");
          err = -1;
        }

        break;
      }
    }

    return err;
  }

  public static void main(String[] args) {
    ChipDeviceController controller =
        new ChipDeviceController(
            ControllerParams.newBuilder()
                .setUdpListenPort(0)
                .setControllerVendorId(0xFFF1)
                .build());

    ChipDeviceController dc = new ChipDeviceController();
    CredentialIssuerCommands credIssuerCmds = new CredentialIssuerCommands();

    runCommand(dc, credIssuerCmds, args);
  }
}
