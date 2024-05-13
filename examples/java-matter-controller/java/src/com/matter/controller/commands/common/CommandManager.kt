/*
 *   Copyright (c) 2023 Project CHIP Authors
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
package com.matter.controller.commands.common

import java.util.Arrays
import java.util.HashMap
import java.util.logging.Level
import java.util.logging.Logger
import kotlin.collections.MutableMap

class CommandManager {
  private val clusters: MutableMap<String, List<Command>> = HashMap()

  fun register(clusterName: String, commandsList: List<Command>) {
    clusters[clusterName] = commandsList
  }

  @Throws(Exception::class)
  fun run(args: Array<String>) {
    val command: Command?
    if (args.size < 1) {
      logger.log(Level.INFO, "Missing cluster name")
      showHelpInfo()
      return
    }
    val commands = clusters[args[0]]
    if (commands == null) {
      logger.log(Level.INFO, "Unknown cluster: " + args[0])
      showHelpInfo()
      return
    }
    if (args.size < 2) {
      logger.log(Level.INFO, "Missing command name")
      showCluster(args[0], commands)
      return
    }
    if (!isGlobalCommand(args[0])) {
      command = getCommand(commands, args[1])
      if (command == null) {
        System.out.printf("Unknown command: %s", args[1])
        showCluster(args[0], commands)
        throw IllegalArgumentException()
      }
    } else if (isEventCommand(args[1])) {
      if (args.size < 3) {
        logger.log(Level.INFO, "Missing event name")
        showClusterEvents(args[0], args[1], commands)
        throw IllegalArgumentException()
      }
      command = getGlobalCommand(commands, args[1], args[2])
      if (command == null) {
        logger.log(Level.INFO, "Unknown event: " + args[2])
        showClusterEvents(args[0], args[1], commands)
        throw IllegalArgumentException()
      }
    } else {
      if (args.size < 3) {
        logger.log(Level.INFO, "Missing attribute name")
        showClusterAttributes(args[0], args[1], commands)
        throw IllegalArgumentException()
      }
      command = getGlobalCommand(commands, args[1], args[2])
      if (command == null) {
        logger.log(Level.INFO, "Unknown attribute: " + args[2])
        showClusterAttributes(args[0], args[1], commands)
        throw IllegalArgumentException()
      }
    }

    // need skip over binary and command name and only get arguments
    val temp = Arrays.copyOfRange(args, 2, args.size)
    try {
      command.setArgumentValues(temp)
    } catch (e: IllegalArgumentException) {
      logger.log(Level.INFO, "Arguments init failed with exception: " + e.message)
      showCommand(args[0], command)
      System.exit(1)
    }
    command.run()
  }

  private fun isAttributeCommand(commandName: String): Boolean {
    return commandName == "read" || commandName == "write" || commandName == "subscribe"
  }

  private fun isEventCommand(commandName: String): Boolean {
    return commandName == "read-event" || commandName == "subscribe-event"
  }

  private fun isGlobalCommand(commandName: String): Boolean {
    return isAttributeCommand(commandName) || isEventCommand(commandName)
  }

  private fun getCommand(commands: List<Command>, commandName: String): Command? {
    for (command in commands) {
      if (commandName == command.name) {
        return command
      }
    }
    return null
  }

  private fun getGlobalCommand(
    commands: List<Command>,
    commandName: String,
    attributeName: String
  ): Command? {
    for (command in commands) {
      if (commandName == command.name && attributeName == command.getAttribute()) {
        return command
      }
    }
    return null
  }

  private fun showHelpInfo() {
    logger.log(Level.INFO, "Usage:")
    logger.log(Level.INFO, "  java-matter-controller cluster_name command_name [param1 param2 ...]")
    logger.log(Level.INFO, "\n")
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
    logger.log(
      Level.INFO,
      "  | Clusters:                                                                           |"
    )
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
    for (key in clusters.keys) {
      System.out.printf("  | * %-82s|\n", key)
    }
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
  }

  private fun showCluster(clusterName: String, commands: List<Command>) {
    logger.log(Level.INFO, "Usage:")
    logger.log(Level.INFO, "  java-matter-controller $clusterName command_name [param1 param2 ...]")
    logger.log(Level.INFO, "\n")
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
    logger.log(
      Level.INFO,
      "  | Commands:                                                                           |"
    )
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
    var readCommand = false
    var writeCommand = false
    var subscribeCommand = false
    var readEventCommand = false
    var subscribeEventCommand = false
    for (command in commands) {
      var shouldPrint = true
      val cmdName = command.name
      if (isGlobalCommand(cmdName)) {
        if (cmdName == "read" && !readCommand) {
          readCommand = true
        } else if (cmdName == "write" && !writeCommand) {
          writeCommand = true
        } else if (cmdName == "subscribe" && !subscribeCommand) {
          subscribeCommand = true
        } else if (cmdName == "read-event" && !readEventCommand) {
          readEventCommand = true
        } else if (cmdName == "subscribe-event" && !subscribeEventCommand) {
          subscribeEventCommand = true
        } else {
          shouldPrint = false
        }
      }
      if (shouldPrint) {
        System.out.printf("  | * %-82s|\n", cmdName)
      }
    }
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+\n"
    )
  }

  private fun showClusterAttributes(
    clusterName: String,
    commandName: String,
    commands: List<Command>
  ) {
    logger.log(Level.INFO, "Usage:")
    System.out.printf(
      "  java-matter-controller %s %s attribute-name [param1 param2 ...]\n",
      clusterName,
      commandName
    )
    logger.log(Level.INFO, "\n")
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
    logger.log(
      Level.INFO,
      "  | Attributes:                                                                         |"
    )
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
    for (command in commands) {
      if (commandName == command.name) {
        System.out.printf("  | * %-82s|\n", command.getAttribute())
      }
    }
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
  }

  private fun showClusterEvents(clusterName: String, commandName: String, commands: List<Command>) {
    logger.log(Level.INFO, "Usage:")
    System.out.printf(
      "  java-matter-controller %s %s event-name [param1 param2 ...]\n",
      clusterName,
      commandName
    )
    logger.log(Level.INFO, "\n")
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
    logger.log(
      Level.INFO,
      "  | Events:                                                                             |"
    )
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
    for (command in commands) {
      if (commandName == command.name) {
        System.out.printf("  | * %-82s|\n", command.getAttribute())
      }
    }
    logger.log(
      Level.INFO,
      "  +-------------------------------------------------------------------------------------+"
    )
  }

  private fun showCommand(clusterName: String, command: Command) {
    logger.log(Level.INFO, "Usage:")
    var arguments: String? = command.name
    var description = ""
    val argumentsCount = command.getArgumentsCount()
    for (i in 0 until argumentsCount) {
      var arg = ""
      val isOptional = command.getArgumentIsOptional(i)
      if (isOptional) {
        arg += "[--"
      }
      arg += command.getArgumentName(i)
      if (isOptional) {
        arg += "]"
      }
      arguments += " "
      arguments += arg
      val argDescription = command.getArgumentDescription(i)
      if (argDescription != null) {
        description += "\n"
        description += arg
        description += ":\n  "
        description += argDescription
        description += "\n"
      }
    }
    System.out.format("  java-matter-controller %s %s\n", clusterName, arguments)
    val helpText = command.helpText
    if (helpText != null) {
      System.out.format("\n%s\n", helpText)
    }
    if (!description.isEmpty()) {
      logger.log(Level.INFO, description)
    }
  }

  companion object {
    private val logger = Logger.getLogger(CommandManager::class.java.name)
  }
}
