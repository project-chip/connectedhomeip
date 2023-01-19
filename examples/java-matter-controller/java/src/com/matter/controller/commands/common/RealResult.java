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

/**
 * Implements a Result where an error string is associated with its failure state.
 *
 * <p>A `Result` is just a booean of true/false that is exposed via `getResult`. This class will
 * contain either a `true` value for `Success` or a `false` value in which case the failure will
 * also have an error string explaining the reason of the failure associated with it.
 */
public class RealResult {
  private boolean result;
  private String error;

  public RealResult() {
    this.result = true;
  }

  public RealResult(String error) {
    this.result = false;
    this.error = error;
  }

  public static RealResult Success() {
    return new RealResult();
  }

  public static RealResult Error(String error) {
    return new RealResult(error);
  }

  public boolean getResult() {
    return result;
  }

  public String getError() {
    return error;
  }
}
