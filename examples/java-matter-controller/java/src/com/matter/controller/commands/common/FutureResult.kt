/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

package com.matter.controller.commands.common

import java.util.logging.Level
import java.util.logging.Logger

/**
 * Implements the future result that encapculates the optional realResult, application would wait
 * for realResult set by other thread wben receiving data from the other end. If the expected
 * duration elapsed without receiving the expected realResult, the runtime exception would be
 * raised.
 */
class RealResultException(message: String) : RuntimeException(message)

class FutureResult {
  private var realResult: RealResult? = null
  private var timeoutMs: Long = 0
  private val logger = Logger.getLogger(FutureResult::class.java.name)
  private val lock = Object()

  fun setTimeoutMs(timeoutMs: Long) {
    this.timeoutMs = timeoutMs
  }

  fun setRealResult(realResult: RealResult) {
    synchronized(lock) {
      if (this.realResult != null) {
        throw RealResultException("Error, real result has been set!")
      }
      this.realResult = realResult
      lock.notifyAll()
    }
  }

  fun waitResult() {
    val start = System.currentTimeMillis()
    synchronized(lock) {
      while (realResult == null) {
        try {
          if (System.currentTimeMillis() > start + timeoutMs) {
            throw RealResultException("Timeout!")
          }
          lock.wait()
        } catch (e: InterruptedException) {
          logger.log(Level.INFO, "Wait Result failed with exception: " + e.message)
        }
      }
      if (realResult?.getResult() == false) {
        logger.log(Level.INFO, "Error: ${realResult?.getError()}")
        throw RealResultException("Received failure test result")
      }
    }
  }

  fun clear() {
    synchronized(lock) { realResult = null }
  }
}
