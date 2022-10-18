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

public abstract class Command {
  private String mName = null;
  private String mHelpText = null;

  private boolean mIsInteractive = false;
  private ArrayList<Argument> mArgs = new ArrayList<Argument>();

  public Command(String commandName) {
    this.mName = commandName;
  }

  public Command(String commandName, String helpText) {
    this.mName = commandName;
    this.mHelpText = helpText;
  }

  public final String getName() {
    return mName;
  }

  public final String getHelpText() {
    return mHelpText;
  }

  public final String getAttribute() {
    for (int i = 0; i < mArgs.size(); i++) {
      Argument arg = mArgs.get(i);
      if (arg.type == ArgumentType.Attribute) {
        return (String) arg.value;
      }
    }

    return null;
  }

  public final String getArgumentName(int index) {
    if (index < mArgs.size()) {
      return mArgs.get(index).name;
    }

    return null;
  }

  public final int getArgumentsCount() {
    return mArgs.size();
  }

  public final String getArgumentDescription(int index) {
    if (index < mArgs.size()) {
      return mArgs.get(index).desc;
    }

    return null;
  }

  public final int addArgument(String name, StringBuffer out) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Attribute;
    arg.name = name;
    arg.value = (Object) out;

    mArgs.add(arg);
    return mArgs.size();
  }

  public final int addArgument(String name, int min, int max, MutableInteger out) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Number_int32;
    arg.name = name;
    arg.value = (Object) out;
    arg.min = min;
    arg.max = max;

    mArgs.add(arg);
    return mArgs.size();
  }

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

  public final int addArgument(String name, IPAddress out) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Address;
    arg.name = name;
    arg.value = (Object) out;

    mArgs.add(arg);
    return mArgs.size();
  }

  public final int addArgument(String name, String out, String desc) {
    Argument arg = new Argument();
    arg.type = ArgumentType.String;
    arg.name = name;
    arg.value = (Object) out;
    arg.desc = desc;

    mArgs.add(arg);
    return mArgs.size();
  }

  public final int addArgument(String name, long min, long max, Optional<Long> out, String desc) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Number_int64;
    arg.name = name;
    arg.value = (Object) out;
    arg.min = min;
    arg.max = max;
    arg.flags = Argument.kOptional;
    arg.desc = desc;

    mArgs.add(arg);
    return mArgs.size();
  }

  public final int addArgument(
      String name, short min, short max, Optional<Short> out, String desc) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Number_int16;
    arg.name = name;
    arg.value = (Object) out;
    arg.min = min;
    arg.max = max;
    arg.flags = Argument.kOptional;
    arg.desc = desc;

    mArgs.add(arg);
    return mArgs.size();
  }

  public final int addArgument(String name, Optional<Boolean> out, String desc) {
    Argument arg = new Argument();
    arg.type = ArgumentType.Bool;
    arg.name = name;
    arg.value = (Object) out;
    arg.flags = Argument.kOptional;
    arg.desc = desc;

    mArgs.add(arg);
    return mArgs.size();
  }

  public final boolean initArguments(int argc, String[] args) {
    boolean isValidCommand = false;
    int argsCount = mArgs.size();

    if (argsCount != argc) {
      System.out.println(
          "initArguments: Wrong arguments number: " + argc + " instead of " + argsCount);
      return false;
    }

    for (int i = 0; i < argsCount; i++) {
      if (!initArgument(i, args[i])) {
        System.exit(0);
      }
    }

    isValidCommand = true;

    return isValidCommand;
  }

  public boolean initArgument(int argIndex, String argValue) {
    boolean isValidArgument = false;

    Argument arg = mArgs.get(argIndex);
    switch (arg.type) {
      case Attribute:
        {
          String value = (String) arg.value;
          isValidArgument = argValue.equals(value);
          break;
        }

      case Number_int32:
        {
          MutableInteger value = (MutableInteger) arg.value;
          value.setValue(Integer.parseInt(argValue));
          isValidArgument = (value.getValue() >= arg.min && value.getValue() <= arg.max);
          break;
        }

      case Address:
        {
          IPAddress ipAddress = (IPAddress) arg.value;
          isValidArgument = ipAddress.fromString(argValue);
          break;
        }
    }

    if (!isValidArgument) {
      System.out.println("InitArgs: Invalid argument " + arg.name + ": " + argValue);
    }

    return isValidArgument;
  }

  public void resetArguments() {}

  public abstract int run();

  public boolean isInteractive() {
    return mIsInteractive;
  }

  int runAsInteractive() {
    mIsInteractive = true;
    return run();
  }
}
