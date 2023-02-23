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

import java.net.InetAddress
import java.net.UnknownHostException
import java.util.Optional
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong
import javax.annotation.Nullable

class Argument {
  val name: String
  val type: ArgumentType
  private val mMin: Long
  private val mMax: Long
  val value: Any
  val desc: Optional<String>
  val isOptional: Boolean

  constructor(name: String, value: IPAddress, optional: Boolean) {
    this.name = name
    type = ArgumentType.ADDRESS
    mMin = 0
    mMax = 0
    this.value = value
    desc = Optional.empty()
    isOptional = optional
  }

  constructor(name: String, value: StringBuffer, @Nullable desc: String?, optional: Boolean) {
    this.name = name
    type = ArgumentType.STRING
    mMin = 0
    mMax = 0
    this.value = value
    this.desc = Optional.ofNullable(desc)
    isOptional = optional
  }

  constructor(name: String, value: AtomicBoolean, @Nullable desc: String?, optional: Boolean) {
    this.name = name
    type = ArgumentType.BOOL
    mMin = 0
    mMax = 0
    this.value = value
    this.desc = Optional.ofNullable(desc)
    isOptional = optional
  }

  constructor(
    name: String,
    min: Short,
    max: Short,
    value: AtomicInteger,
    @Nullable desc: String?,
    optional: Boolean
  ) {
    this.name = name
    type = ArgumentType.NUMBER_INT16
    mMin = min.toLong()
    mMax = max.toLong()
    this.value = value
    this.desc = Optional.ofNullable(desc)
    isOptional = optional
  }

  constructor(
    name: String, min: Int, max: Int, value: AtomicInteger, @Nullable desc: String?, optional: Boolean
  ) {
    this.name = name
    type = ArgumentType.NUMBER_INT32
    mMin = min.toLong()
    mMax = max.toLong()
    this.value = value
    this.desc = Optional.ofNullable(desc)
    isOptional = optional
  }

  constructor(
    name: String, min: Long, max: Long, value: AtomicLong, @Nullable desc: String?, optional: Boolean
  ) {
    this.name = name
    type = ArgumentType.NUMBER_INT64
    mMin = min
    mMax = max
    this.value = value
    this.desc = Optional.ofNullable(desc)
    isOptional = optional
  }

  fun setValue(value: String) {
    var isValidArgument = false
    when (type) {
      ArgumentType.ATTRIBUTE -> {
        val str = this.value as String
        isValidArgument = value == str
      }

      ArgumentType.NUMBER_INT16 -> {
        val numShort = this.value as AtomicInteger
        numShort.set(value.toInt())
        isValidArgument = numShort.toInt() >= mMin && numShort.toInt() <= mMax
      }

      ArgumentType.NUMBER_INT32 -> {
        val num = this.value as AtomicInteger
        num.set(value.toInt())
        isValidArgument = num.toInt() >= mMin && num.toInt() <= mMax
      }

      ArgumentType.NUMBER_INT64 -> {
        val numLong = this.value as AtomicLong
        numLong.set(value.toLong())
        isValidArgument = numLong.toInt() >= mMin && numLong.toInt() <= mMax
      }

      ArgumentType.ADDRESS -> isValidArgument = try {
        val ipAddress = this.value as IPAddress
        ipAddress.setAddress(InetAddress.getByName(value))
        true
      } catch (e: UnknownHostException) {
        false
      }

      else -> {
      }
    }
    require(isValidArgument) { "Invalid argument " + name + ": " + value }
  }
}