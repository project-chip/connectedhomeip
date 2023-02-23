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
package com.matter.controller.commands.common

import java.util.ArrayList
import java.util.Optional
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong
import javax.annotation.Nullable

/**
 * @brief Matter Controller command
 * @details The base class of all the commands the Matter Controller supports, which are actions
 * that may be performed. Commands are verb-like, such as pair a Matter device or discover
 * Matter devices from the environment.
 */
abstract class Command {
  val mName: String
  private val mArgs = ArrayList<Argument>()
  val mHelpText: Optional<String>

  constructor(commandName: String) {
    mName = commandName
    mHelpText = Optional.empty()
  }

  constructor(commandName: String, @Nullable helpText: String?) {
    mName = commandName
    mHelpText = Optional.ofNullable(helpText)
  }

  fun getName(): String {
    return mName
  }

  fun getHelpText(): Optional<String> {
    return mHelpText
  }

  fun getArgumentName(index: Int): String {
    return mArgs[index].name
  }

  fun getArgumentsCount(): Int {
    return mArgs.size
  }

  fun getArgumentIsOptional(index: Int): Boolean {
    return mArgs[index].isOptional
  }

  /**
   * @brief Get attribute argument if it exists, there is at most one Attribute argument per command
   * @return A pointer to an Optional where the Attribute argument will be stored
   */
  fun getAttribute(): Optional<String> {
    return mArgs.stream()
      .filter { arg: Argument -> arg.type === ArgumentType.ATTRIBUTE }
      .findFirst()
      .map { arg: Argument -> arg.value as String }
  }

  /**
   * @brief Get argument description if it exists
   * @return A pointer to an Optional where the argument description will be stored
   */
  fun getArgumentDescription(index: Int): Optional<String> {
    return mArgs[index].desc
  }

  fun addArgumentToList(arg: Argument) {
    if (arg.isOptional || mArgs.isEmpty()) {
      // Safe to just append to the end of a list.
      mArgs.add(arg)
      return
    }

    // mandatory arg needs to be inserted before the optional arguments.
    var index = 0
    while (index < mArgs.size && !mArgs[index].isOptional) {
      index++
    }

    // Insert before the first optional arg.
    mArgs.add(index, arg)
  }

  /**
   * @brief Add a bool command argument
   * @param name The name that will be displayed in the command help
   * @param out A pointer to a AtomicBoolean where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @param optional Indicate if an optional argument
   * @return The number of arguments currently added to the command
   */
  fun addArgument(
    name: String?, out: AtomicBoolean?, @Nullable desc: String?, optional: Boolean
  ) {
    val arg = Argument(name!!, out!!, desc, optional)
    addArgumentToList(arg)
  }

  /**
   * @brief Add a short command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to a AtomicInteger where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @param optional Indicate if an optional argument
   * @return The number of arguments currently added to the command
   */
  fun addArgument(
    name: String?,
    min: Short,
    max: Short,
    out: AtomicInteger?,
    @Nullable desc: String?,
    optional: Boolean
  ) {
    val arg = Argument(name!!, min, max, out!!, desc, optional)
    addArgumentToList(arg)
  }

  /**
   * @brief Add an int command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to a AtomicInteger where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @param optional Indicate if an optional argument
   * @return The number of arguments currently added to the command
   */
  fun addArgument(
    name: String?, min: Int, max: Int, out: AtomicInteger?, @Nullable desc: String?, optional: Boolean
  ) {
    val arg = Argument(name!!, min, max, out!!, desc, optional)
    addArgumentToList(arg)
  }

  /**
   * @brief Add a long Integer command argument
   * @param name The name that will be displayed in the command help
   * @param min The minimum value of the argv value
   * @param max The minimum value of the argv value
   * @param out A pointer to a AtomicLong where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @param optional Indicate if an optional argument
   * @return The number of arguments currently added to the command
   */
  fun addArgument(
    name: String?, min: Long, max: Long, out: AtomicLong?, @Nullable desc: String?, optional: Boolean
  ) {
    val arg = Argument(name!!, min, max, out!!, desc, optional)
    addArgumentToList(arg)
  }

  /**
   * @brief Add an IP address command argument
   * @param name The name that will be displayed in the command help
   * @param out A pointer to a IPAddress where the argv value will be stored
   * @param optional Indicate if an optional argument
   * @return The number of arguments currently added to the command
   */
  fun addArgument(name: String?, out: IPAddress?, optional: Boolean) {
    val arg = Argument(name!!, out!!, optional)
    addArgumentToList(arg)
  }

  /**
   * @brief Add a String command argument
   * @param name The name that will be displayed in the command help
   * @param out A pointer to a StringBuffer where the argv value will be stored
   * @param desc The description of the argument that will be displayed in the command help
   * @param optional Indicate if an optional argument
   * @return The number of arguments currently added to the command
   */
  fun addArgument(
    name: String?, out: StringBuffer?, @Nullable desc: String?, optional: Boolean
  ) {
    val arg = Argument(name!!, out!!, desc, optional)
    addArgumentToList(arg)
  }

  /**
   * @brief Initialize command arguments
   * @param argc The number of arguments to a command (Does not include command itself)
   * @param args Supplied command-line arguments as an array of String objects.
   */
  fun initArguments(argc: Int, args: Array<String>) {
    var mandatoryArgsCount = 0
    var currentIndex = 0
    for (arg in mArgs) {
      if (!arg.isOptional) {
        mandatoryArgsCount++
      }
    }
    require(argc >= mandatoryArgsCount) { "initArguments: Wrong arguments number: $argc instead of $mandatoryArgsCount" }

    // Initialize mandatory arguments
    for (i in 0 until mandatoryArgsCount) {
      initArgument(currentIndex++, args[i])
    }

    // Initialize optional arguments
    // Optional arguments expect a name and a value, so i is increased by 2 on every step.
    var i = mandatoryArgsCount
    while (i < argc) {

      // optional arguments starts with OPTIONAL_ARGUMENT_PREFIX
      require(
        !(args[i].length <= OPTIONAL_ARGUMENT_PREFIX_LENGTH
                && !args[i].startsWith(OPTIONAL_ARGUMENT_PREFIX))
      ) { "initArguments: Invalid optional argument: " + args[i] }
      if (args[i]
          .substring(OPTIONAL_ARGUMENT_PREFIX_LENGTH)
        == mArgs[currentIndex].name
      ) {
        require(i + 1 < argc) { "initArguments: Optional argument " + args[i] + " missing value" }
        initArgument(currentIndex++, args[i + 1])
      }
      i += 2
    }
  }

  fun initArgument(argIndex: Int, argValue: String?) {
    mArgs[argIndex].setValue(argValue!!)
  }

  @Throws(Exception::class)
  abstract fun run()

  companion object {
    private const val OPTIONAL_ARGUMENT_PREFIX = "--"
    private const val OPTIONAL_ARGUMENT_PREFIX_LENGTH = 2
  }
}