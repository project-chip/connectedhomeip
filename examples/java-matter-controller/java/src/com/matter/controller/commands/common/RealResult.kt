/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

package com.matter.controller.commands.common

/**
 * Implements a Result where an error string is associated with its failure state.
 *
 * <p>A `Result` is just a booean of true/false that is exposed via `getResult`. This class will
 * contain either a `true` value for `Success` or a `false` value in which case the failure will
 * also have an error string explaining the reason of the failure associated with it.
 */
class RealResult(private val result: Boolean, private val error: String?) {
  constructor() : this(true, null)

  constructor(error: String?) : this(false, error)

  companion object {
    fun success(): RealResult {
      return RealResult()
    }

    fun error(error: String?): RealResult {
      return RealResult(error)
    }
  }

  fun getResult(): Boolean {
    return result
  }

  fun getError(): String? {
    return error
  }
}
