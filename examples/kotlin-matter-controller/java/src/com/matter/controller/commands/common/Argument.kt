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

import java.net.InetAddress
import java.net.UnknownHostException
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong

// TODO Issue #29632:Refactor class Argument to have specializations for various types.
class Argument {
  val name: String
  val type: ArgumentType
  private val minValue: Long
  private val maxValue: Long
  val value: Any
  val desc: String?
  val isOptional: Boolean

  constructor(name: String, value: IPAddress, optional: Boolean) {
    this.name = name
    type = ArgumentType.ADDRESS
    minValue = 0
    maxValue = 0
    this.value = value
    desc = null
    isOptional = optional
  }

  constructor(name: String, value: StringBuffer, desc: String?, optional: Boolean) {
    this.name = name
    type = ArgumentType.STRING
    minValue = 0
    maxValue = 0
    this.value = value
    this.desc = desc
    isOptional = optional
  }

  constructor(name: String, value: AtomicBoolean, desc: String?, optional: Boolean) {
    this.name = name
    type = ArgumentType.BOOL
    minValue = 0
    maxValue = 0
    this.value = value
    this.desc = desc
    isOptional = optional
  }

  constructor(
    name: String,
    min: Short,
    max: Short,
    value: AtomicInteger,
    desc: String?,
    optional: Boolean
  ) {
    this.name = name
    type = ArgumentType.NUMBER_INT16
    minValue = min.toLong()
    maxValue = max.toLong()
    this.value = value
    this.desc = desc
    isOptional = optional
  }

  constructor(
    name: String,
    min: Int,
    max: Int,
    value: AtomicInteger,
    desc: String?,
    optional: Boolean
  ) {
    this.name = name
    type = ArgumentType.NUMBER_INT32
    minValue = min.toLong()
    maxValue = max.toLong()
    this.value = value
    this.desc = desc
    isOptional = optional
  }

  constructor(
    name: String,
    min: Short,
    max: Short,
    value: AtomicLong,
    desc: String?,
    optional: Boolean
  ) {
    this.name = name
    type = ArgumentType.NUMBER_INT32
    minValue = min.toLong()
    maxValue = max.toLong()
    this.value = value
    this.desc = desc
    isOptional = optional
  }

  constructor(
    name: String,
    min: Long,
    max: Long,
    value: AtomicLong,
    desc: String?,
    optional: Boolean
  ) {
    this.name = name
    type = ArgumentType.NUMBER_INT64
    minValue = min
    maxValue = max
    this.value = value
    this.desc = desc
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
        isValidArgument = numShort.toInt() >= minValue && numShort.toInt() <= maxValue
      }
      ArgumentType.NUMBER_INT32 -> {
        val num = this.value as AtomicInteger
        num.set(value.toInt())
        isValidArgument = num.toInt() >= minValue && num.toInt() <= maxValue
      }
      ArgumentType.NUMBER_INT64 -> {
        val numLong = this.value as AtomicLong
        numLong.set(value.toLong())
        isValidArgument = numLong.toInt() >= minValue && numLong.toInt() <= maxValue
      }
      ArgumentType.STRING -> {
        val stringBuffer = this.value as StringBuffer
        stringBuffer.append(value)
        val str = stringBuffer.toString()
        isValidArgument = value == str
      }
      ArgumentType.BOOL -> {
        val atomicBoolean = this.value as AtomicBoolean
        try {
          atomicBoolean.set(value.toBoolean())
          isValidArgument = true
        } catch (e: Exception) {
          isValidArgument = false
        }
      }
      ArgumentType.ADDRESS ->
        isValidArgument =
          try {
            val ipAddress = this.value as IPAddress
            ipAddress.address = InetAddress.getByName(value)
            true
          } catch (e: UnknownHostException) {
            false
          }
      else -> {}
    }
    require(isValidArgument) { "Invalid argument " + name + ": " + value }
  }
}
