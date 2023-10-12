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

/**
 * Represents a result that can either indicate success or failure with an associated error message.
 *
 * In the context of RealResult, success is represented by [Success] and failure by [Error]. When
 * there is an error, an error message explains the reason for the failure.
 */
sealed class RealResult {
  data class Error(val error: String) : RealResult()

  object Success : RealResult()

  companion object {
    fun success(): RealResult {
      return Success
    }

    fun error(error: String?): RealResult {
      return error?.let { Error(it) } ?: Success
    }
  }
}
