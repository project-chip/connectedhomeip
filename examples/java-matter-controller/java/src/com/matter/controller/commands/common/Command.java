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
import java.util.Optional;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import javax.annotation.Nullable;

/**
 * @brief Matter Controller command
 * @details The base class of all the commands the Matter Controller supports, which are actions
 *     that may be performed. Commands are verb-like, such as pair a Matter device or discover
 *     Matter devices from the environment.
 */
public abstract class Command {
  private final String mName;
  private final ArrayList<Argument> mArgs = new ArrayList<Argument>();
  private final Optional<String> mHelpText;

  public Command(String commandName) {
    this.mName = commandName;
    this.mHelpText = Optional.empty();
  }

  public Command(String commandName, @Nullable String helpText) {
    this.mName = commandName;
    this.mHelpText = Optional.ofNullable(helpText);
  }

  public final String getName() {
    return mName;
  }

  public final Optional<String> getHelpText() {
    return mHelpText;
  }

  /**
   * @brief Get attribute argument if it exists, there is at most one Attribute argument per command
   * @return A pointer to an Optional where the Attribute argument will be stored
   */
  public final Optional<String> getAttribute() {
    return mArgs
        .stream()
        .filter(arg -> arg.getType() == ArgumentType.ATTRIBUTE)
        .findFirst()
        .map(arg -> (String) arg.getValue());
  }

  public final String getArgumentName(int index) {
    return mArgs.get(index).getName();
  }

  public final int getArgumentsCount() {
    return mArgs.size();
  }

  /**
   * @brief Get argument description if it exists
   * @return A pointer to an Optional where the argument description will be stored
   */
  public final Optional<String> getArgumentDescription(int index) {
    return mArgs.get(index).getDesc();
  }

  /**
   * @brief Add a bool command argument
   * @param name The name that will be displayed in the command help
   * @param out A pointer to a MutableInteger where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(String name, AtomicBoolean out, @Nullable String desc) {
    Argument arg = new Argument(name, out, desc);
    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add a short command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to a MutableInteger where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(
      String name, short min, short max, AtomicInteger out, @Nullable String desc) {
    Argument arg = new Argument(name, min, max, out, desc);
    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add an int command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to a MutableInteger where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(
      String name, int min, int max, AtomicInteger out, @Nullable String desc) {
    Argument arg = new Argument(name, min, max, out, desc);
    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add a long Integer command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to a MutableInteger where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(
      String name, long min, long max, AtomicLong out, @Nullable String desc) {
    Argument arg = new Argument(name, min, max, out, desc);
    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add an IP address command argument
   * @param name The name that will be displayed in the command help
   * @param out A pointer to a IPAddress where the argv value will be stored
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(String name, IPAddress out) {
    Argument arg = new Argument(name, out);
    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add a String command argument
   * @param name The name that will be displayed in the command help
   * @param out A pointer to a StringBuffer where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(String name, StringBuffer out, @Nullable String desc) {
    Argument arg = new Argument(name, out, desc);
    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Initialize command arguments
   * @param argc The number of arguments to a command (Does not include command itself)
   * @param args Supplied command-line arguments as an array of String objects.
   */
  public final void initArguments(int argc, String[] args) {
    int argsCount = mArgs.size();

    if (argsCount != argc) {
      throw new IllegalArgumentException(
          "Wrong arguments number: " + argc + " instead of " + argsCount);
    }

    for (int i = 0; i < argsCount; i++) {
      initArgument(i, args[i]);
    }
  }

  public void initArgument(int argIndex, String argValue) {
    mArgs.get(argIndex).setValue(argValue);
  }

  public abstract void run() throws Exception;
}
