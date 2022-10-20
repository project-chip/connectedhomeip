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

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Optional;

/**
 * @brief Matter Controller command
 * @details The base class of all the commands the Matter Controller supports, which are actions
 *     that may be performed. Commands are verb-like, such as pair a Matter device or discover
 *     Matter devices from the environment.
 */
public abstract class Command {
  private String mName;
  private Optional<String> mHelpText;

  private boolean mIsInteractive = false;
  private ArrayList<Argument> mArgs = new ArrayList<Argument>();

  public Command(String commandName) {
    this.mName = commandName;
    this.mHelpText = Optional.empty();
  }

  public Command(String commandName, String helpText) {
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
    for (int i = 0; i < mArgs.size(); i++) {
      Argument arg = mArgs.get(i);
      if (arg.type == ArgumentType.Attribute) {
        return Optional.of((String) arg.value);
      }
    }

    return Optional.empty();
  }

  public final String getArgumentName(int index) throws IndexOutOfBoundsException {
    if (index < mArgs.size()) {
      return mArgs.get(index).name;
    } else {
      throw new IndexOutOfBoundsException();
    }
  }

  public final int getArgumentsCount() {
    return mArgs.size();
  }

  /**
   * @brief Get argument description if it exists
   * @return A pointer to an Optional where the argument description will be stored
   */
  public final Optional<String> getArgumentDescription(int index) throws IndexOutOfBoundsException {
    if (index < mArgs.size()) {
      return mArgs.get(index).desc;
    } else {
      throw new IndexOutOfBoundsException();
    }
  }

  /**
   * @brief Add a short Integer command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to a MutableInteger where the argv value will be stored
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(String name, short min, short max, MutableInteger out) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Number_int32;
    arg.name = name;
    arg.value = (Object) out;
    arg.min = min;
    arg.max = max;

    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add a long Integer command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to a MutableInteger where the argv value will be stored
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(String name, long min, long max, MutableInteger out) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Number_int64;
    arg.name = name;
    arg.value = (Object) out;
    arg.min = min;
    arg.max = max;

    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add an IP address command argument
   * @param name The name that will be displayed in the command help
   * @param out A pointer to a IPAddress where the argv value will be stored
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(String name, InetAddress out) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Address;
    arg.name = name;
    arg.value = (Object) out;

    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add an String command argument
   * @param name The name that will be displayed in the command help
   * @param out A pointer to a StringBuffer where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(String name, StringBuffer out, String desc) {
    Argument arg = new Argument();
    arg.type = ArgumentType.String;
    arg.name = name;
    arg.value = (Object) out;
    arg.desc = Optional.ofNullable(desc);

    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add an optional long integer command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to an Optional where the argv value will be stored, the argv value could
   *     be empty
   * @param desc The description of the argument that will be displayed in the command help
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(String name, long min, long max, Optional<Long> out, String desc) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Number_int64;
    arg.name = name;
    arg.value = (Object) out;
    arg.min = min;
    arg.max = max;
    arg.flags = Argument.kOptional;
    arg.desc = Optional.ofNullable(desc);

    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add an optional short integer command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to an Optional where the argv value will be stored, the argv value could
   *     be empty
   * @param desc The description of the argument that will be displayed in the command help
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(
      String name, short min, short max, Optional<Short> out, String desc) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Number_int16;
    arg.name = name;
    arg.value = (Object) out;
    arg.min = min;
    arg.max = max;
    arg.flags = Argument.kOptional;
    arg.desc = Optional.ofNullable(desc);

    mArgs.add(arg);
    return mArgs.size();
  }

  /**
   * @brief Add an optional boolean command argument
   * @param name The name that will be displayed in the command help
   * @param out A pointer to an Optional where the argv value will be stored, the argv value could
   *     be empty
   * @param desc The description of the argument that will be displayed in the command help
   * @return The number of arguments currently added to the command
   */
  public final int addArgument(String name, Optional<Boolean> out, String desc) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Bool;
    arg.name = name;
    arg.value = (Object) out;
    arg.flags = Argument.kOptional;
    arg.desc = Optional.ofNullable(desc);

    mArgs.add(arg);
    return mArgs.size();
  }

  public final void initArguments(int argc, String[] args) throws IllegalArgumentException {
    int argsCount = mArgs.size();

    if (argsCount != argc) {
      throw new IllegalArgumentException(
          "Wrong arguments number: " + argc + " instead of " + argsCount);
    }

    for (int i = 0; i < argsCount; i++) {
      initArgument(i, args[i]);
    }
  }

  public void initArgument(int argIndex, String argValue) throws IllegalArgumentException {
    boolean isValidArgument = false;

    Argument arg = mArgs.get(argIndex);
    switch (arg.type) {
      case Attribute:
        String str = (String) arg.value;
        isValidArgument = argValue.equals(str);
        break;
      case Number_int32:
        MutableInteger value = (MutableInteger) arg.value;
        value.setValue(Integer.parseInt(argValue));
        isValidArgument = (value.getValue() >= arg.min && value.getValue() <= arg.max);
        break;
      case Address:
        try {
          arg.value = (Object) InetAddress.getByName(argValue);
        } catch (UnknownHostException e) {
          isValidArgument = true;
        }
        break;
    }

    if (!isValidArgument) {
      throw new IllegalArgumentException("Invalid argument " + arg.name + ": " + argValue);
    }
  }

  public void resetArguments() {}

  public abstract void run() throws Exception;

  public boolean isInteractive() {
    return mIsInteractive;
  }

  void runAsInteractive() throws Exception {
    mIsInteractive = true;
    run();
  }
}
