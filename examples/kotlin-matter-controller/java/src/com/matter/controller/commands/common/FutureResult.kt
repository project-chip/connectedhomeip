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

import java.util.concurrent.TimeoutException
import java.util.logging.Level
import java.util.logging.Logger

/**
 * Implements the future result that encapculates the optional realResult, application would wait
 * for realResult set by other thread wben receiving data from the other end. If the expected
 * duration elapsed without receiving the expected realResult, the runtime exception would be
 * raised.
 */
class FutureResult {
  private var realResult: RealResult? = null
  private val lock = Object()
  var timeoutMs: Long = 0

  fun setRealResult(realResult: RealResult) {
    synchronized(lock) {
      if (this.realResult != null) {
        throw TimeoutException("Error, real result has been set!")
      }
      this.realResult = realResult
      lock.notifyAll()
    }
  }

  fun waitResult() {
    val start = System.currentTimeMillis()
    synchronized(lock) {
      while (realResult == null) {
        val remainingTime = timeoutMs - (System.currentTimeMillis() - start)
        if (remainingTime <= 0) {
          throw TimeoutException("Timeout!")
        }

        try {
          lock.wait(remainingTime)
        } catch (e: InterruptedException) {
          logger.log(Level.INFO, "Wait Result failed with exception: " + e.message)
        }
      }

      val errorResult = realResult as? RealResult.Error
      if (errorResult != null) {
        logger.log(Level.INFO, "Error: ${errorResult.error}")
        throw TimeoutException("Received failure test result")
      }
    }
  }

  fun clear() {
    synchronized(lock) { realResult = null }
  }

  companion object {
    private val logger = Logger.getLogger(FutureResult::class.java.name)
  }
}
