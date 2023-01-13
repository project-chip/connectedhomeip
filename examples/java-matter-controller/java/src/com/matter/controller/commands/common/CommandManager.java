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

package com.matter.controller.commands.common;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.logging.Level;
import java.util.logging.Logger;

public final class CommandManager {
  private final ArrayList<Command> mCommandMgr = new ArrayList<Command>();
  private final Map<String, ArrayList<Command>> mClusters =
      new HashMap<String, ArrayList<Command>>();
  private static Logger logger = Logger.getLogger(CommandManager.class.getName());

  public final void register(String clusterName, ArrayList<Command> commandsList) {
    mClusters.put(clusterName, commandsList);
  }

  public final void run(String[] args) throws Exception {
    Command command;

    if (args.length < 1) {
      logger.log(Level.INFO, "Missing cluster name");
      showClusters();
      return;
    }

    ArrayList<Command> commands = mClusters.get(args[0]);
    if (commands == null) {
      logger.log(Level.INFO, "Unknown cluster: " + args[0]);
      showClusters();
      return;
    }

    if (args.length < 2) {
      logger.log(Level.INFO, "Missing command name");
      showCluster(args[0], commands);
      return;
    }

    if (!isGlobalCommand(args[0])) {
      command = getCommand(commands, args[1]);
      if (command == null) {
        System.out.printf("Unknown command: %s", args[1]);
        showCluster(args[0], commands);
        throw new IllegalArgumentException();
      }
    } else if (isEventCommand(args[1])) {
      if (args.length < 3) {
        logger.log(Level.INFO, "Missing event name");
        showClusterEvents(args[0], args[1], commands);
        throw new IllegalArgumentException();
      }

      command = getGlobalCommand(commands, args[1], args[2]);
      if (command == null) {
        logger.log(Level.INFO, "Unknown event: " + args[2]);
        showClusterEvents(args[0], args[1], commands);
        throw new IllegalArgumentException();
      }
    } else {
      if (args.length < 3) {
        logger.log(Level.INFO, "Missing attribute name");
        showClusterAttributes(args[0], args[1], commands);
        throw new IllegalArgumentException();
      }

      command = getGlobalCommand(commands, args[1], args[2]);
      if (command == null) {
        logger.log(Level.INFO, "Unknown attribute: " + args[2]);
        showClusterAttributes(args[0], args[1], commands);
        throw new IllegalArgumentException();
      }
    }

    // need skip over binary and command name and only get arguments
    String[] temp = Arrays.copyOfRange(args, 2, args.length);

    command.initArguments(temp.length, temp);
    showCommand(args[0], command);
    command.run();
  }

  private boolean isAttributeCommand(String commandName) {
    return commandName.equals("read")
        || commandName.equals("write")
        || commandName.equals("subscribe");
  }

  private boolean isEventCommand(String commandName) {
    return commandName.equals("read-event") || commandName.equals("subscribe-event");
  }

  private boolean isGlobalCommand(String commandName) {
    return isAttributeCommand(commandName) || isEventCommand(commandName);
  }

  private Command getCommand(ArrayList<Command> commands, String commandName) {
    for (Command command : commands) {
      if (commandName.equals(command.getName())) {
        return command;
      }
    }

    return null;
  }

  private Command getGlobalCommand(
      ArrayList<Command> commands, String commandName, String attributeName) {
    for (Command command : commands) {
      if (commandName.equals(command.getName()) && attributeName.equals(command.getAttribute())) {
        return command;
      }
    }

    return null;
  }

  private void showClusters() {
    logger.log(Level.INFO, "Usage:");
    logger.log(
        Level.INFO, "  java-matter-controller cluster_name command_name [param1 param2 ...]");
    logger.log(Level.INFO, "\n");
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");
    logger.log(
        Level.INFO,
        "  | Clusters:                                                                           |");
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");

    for (String key : mClusters.keySet()) {
      System.out.printf("  | * %-82s|\n", key.toLowerCase());
    }

    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");
  }

  private void showCluster(String clusterName, ArrayList<Command> commands) {
    logger.log(Level.INFO, "Usage:");
    logger.log(
        Level.INFO,
        "  java-matter-controller " + clusterName + " command_name [param1 param2 ...]");
    logger.log(Level.INFO, "\n");
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");
    logger.log(
        Level.INFO,
        "  | Commands:                                                                           |");
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");
    boolean readCommand = false;
    boolean writeCommand = false;
    boolean subscribeCommand = false;
    boolean readEventCommand = false;
    boolean subscribeEventCommand = false;

    for (Command command : commands) {
      boolean shouldPrint = true;
      String cmdName = command.getName();
      if (isGlobalCommand(cmdName)) {
        if (cmdName.equals("read") && !readCommand) {
          readCommand = true;
        } else if (cmdName.equals("write") && !writeCommand) {
          writeCommand = true;
        } else if (cmdName.equals("subscribe") && !subscribeCommand) {
          subscribeCommand = true;
        } else if (cmdName.equals("read-event") && !readEventCommand) {
          readEventCommand = true;
        } else if (cmdName.equals("subscribe-event") && !subscribeEventCommand) {
          subscribeEventCommand = true;
        } else {
          shouldPrint = false;
        }
      }

      if (shouldPrint) {
        System.out.printf("  | * %-82s|\n", cmdName);
      }
    }
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+\n");
  }

  private void showClusterAttributes(
      String clusterName, String commandName, ArrayList<Command> commands) {
    logger.log(Level.INFO, "Usage:");
    System.out.printf(
        "  java-matter-controller %s %s attribute-name [param1 param2 ...]\n",
        clusterName, commandName);
    logger.log(Level.INFO, "\n");
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");
    logger.log(
        Level.INFO,
        "  | Attributes:                                                                         |");
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");
    for (Command command : commands) {
      if (commandName.equals(command.getName())) {
        System.out.printf("  | * %-82s|\n", command.getAttribute().get());
      }
    }
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");
  }

  private void showClusterEvents(
      String clusterName, String commandName, ArrayList<Command> commands) {
    logger.log(Level.INFO, "Usage:");
    System.out.printf(
        "  java-matter-controller %s %s event-name [param1 param2 ...]\n",
        clusterName, commandName);
    logger.log(Level.INFO, "\n");
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");
    logger.log(
        Level.INFO,
        "  | Events:                                                                             |");
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");

    for (Command command : commands) {
      if (commandName.equals(command.getName())) {
        System.out.printf("  | * %-82s|\n", command.getAttribute().get());
      }
    }
    logger.log(
        Level.INFO,
        "  +-------------------------------------------------------------------------------------+");
  }

  private void showCommand(String clusterName, Command command) {
    logger.log(Level.INFO, "Usage:");

    String arguments = command.getName();
    String description = "";

    int argumentsCount = command.getArgumentsCount();
    for (int i = 0; i < argumentsCount; i++) {
      String arg = "";
      boolean isOptional = command.getArgumentIsOptional(i);
      if (isOptional) {
        arg += "[--";
      }
      arg += command.getArgumentName(i);
      if (isOptional) {
        arg += "]";
      }
      arguments += " ";
      arguments += arg;

      Optional<String> argDescription = command.getArgumentDescription(i);
      if (argDescription.isPresent()) {
        description += "\n";
        description += arg;
        description += ":\n  ";
        description += argDescription.get();
        description += "\n";
      }
    }
    System.out.format("  java-matter-controller %s %s\n", clusterName, arguments);

    Optional<String> helpText = command.getHelpText();
    if (helpText.isPresent()) {
      System.out.format("\n%s\n", helpText.get());
    }

    if (!description.isEmpty()) {
      logger.log(Level.INFO, description);
    }
  }
}
