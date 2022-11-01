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

public class MutableInteger {
  private long value;

  public MutableInteger() {
    this.value = 0;
  }

  public MutableInteger(boolean value) {
    this.value = value ? 1 : 0;
  }

  public MutableInteger(short value) {
    this.value = value;
  }

  public MutableInteger(int value) {
    this.value = value;
  }

  public MutableInteger(long value) {
    this.value = value;
  }

  public void setValue(boolean value) {
    this.value = value ? 1 : 0;
  }

  public void setValue(short value) {
    this.value = value;
  }

  public void setValue(int value) {
    this.value = value;
  }

  public void setValue(long value) {
    this.value = value;
  }

  public boolean boolValue() {
    return (this.value == 0) ? false : true;
  }

  public short shortValue() {
    return (short) value;
  }

  public int intValue() {
    return (int) value;
  }

  public long longValue() {
    return value;
  }

  @Override
  public String toString() {
    return String.valueOf(value);
  }
}
