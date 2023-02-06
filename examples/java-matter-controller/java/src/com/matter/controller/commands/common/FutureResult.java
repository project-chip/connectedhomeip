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

package com.matter.controller.commands.common;

import java.util.Optional;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Implements the future result that encapculates the optional realResult, application would wait
 * for realResult set by other thread wben receiving data from the other end. If the expected
 * duration elapsed without receiving the expected realResult, the runtime exception would be
 * raised.
 */
public class FutureResult {
  private Optional<RealResult> realResult = Optional.empty();
  private long timeoutMs = 0;
  private static Logger logger = Logger.getLogger(FutureResult.class.getName());

  public void setTimeoutMs(long timeoutMs) {
    this.timeoutMs = timeoutMs;
  }

  public synchronized void setRealResult(RealResult realResult) {
    if (this.realResult.isPresent()) {
      throw new RuntimeException("error, real result has been set!");
    }
    this.realResult = Optional.of(realResult);
    notifyAll();
  }

  public synchronized void waitResult() {
    long start = System.currentTimeMillis();
    while (!realResult.isPresent()) {
      try {
        if (System.currentTimeMillis() > (start + timeoutMs)) {
          throw new RuntimeException("timeout!");
        }
        wait();
      } catch (InterruptedException e) {
      }
    }

    if (!realResult.get().getResult()) {
      logger.log(Level.INFO, "error: %s", realResult.get().getError());
      throw new RuntimeException("received failure test result");
    }
  }
}
