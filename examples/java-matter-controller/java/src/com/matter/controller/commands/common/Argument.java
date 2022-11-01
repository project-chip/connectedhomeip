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

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Optional;
import javax.annotation.Nullable;

public class Argument {
  private String mName;
  private ArgumentType mType;
  private long mMin;
  private long mMax;
  private Object mValue;
  private Optional<String> mDesc = Optional.empty();

  public Argument(String name, IPAddress value) {
    this.mName = name;
    this.mType = ArgumentType.ADDRESS;
    this.mValue = (Object) value;
  }

  public Argument(String name, StringBuffer value, @Nullable String desc) {
    this.mName = name;
    this.mType = ArgumentType.STRING;
    this.mValue = (Object) value;
    this.mDesc = Optional.ofNullable(desc);
  }

  public Argument(String name, MutableInteger value, @Nullable String desc) {
    this.mName = name;
    this.mType = ArgumentType.BOOL;
    this.mValue = (Object) value;
    this.mDesc = Optional.ofNullable(desc);
  }

  public Argument(String name, short min, short max, MutableInteger value, @Nullable String desc) {
    this.mName = name;
    this.mType = ArgumentType.NUMBER_INT16;
    this.mMin = min;
    this.mMax = max;
    this.mValue = (Object) value;
    this.mDesc = Optional.ofNullable(desc);
  }

  public Argument(String name, int min, int max, MutableInteger value, @Nullable String desc) {
    this.mName = name;
    this.mType = ArgumentType.NUMBER_INT32;
    this.mMin = min;
    this.mMax = max;
    this.mValue = (Object) value;
    this.mDesc = Optional.ofNullable(desc);
  }

  public Argument(String name, long min, long max, MutableInteger value, @Nullable String desc) {
    this.mName = name;
    this.mType = ArgumentType.NUMBER_INT64;
    this.mMin = min;
    this.mMax = max;
    this.mValue = (Object) value;
    this.mDesc = Optional.ofNullable(desc);
  }

  public String getName() {
    return mName;
  }

  public ArgumentType getType() {
    return mType;
  }

  public Object getValue() {
    return mValue;
  }

  public Optional<String> getDesc() {
    return mDesc;
  }

  public void setValue(String value) {
    boolean isValidArgument = false;

    switch (mType) {
      case ATTRIBUTE:
        String str = (String) mValue;
        isValidArgument = value.equals(str);
        break;
      case NUMBER_INT32:
        MutableInteger num = (MutableInteger) mValue;
        num.setValue(Integer.parseInt(value));
        isValidArgument = (num.intValue() >= mMin && num.intValue() <= mMax);
        break;
      case ADDRESS:
        try {
          IPAddress ipAddress = (IPAddress) mValue;
          ipAddress.setAddress(InetAddress.getByName(value));
        } catch (UnknownHostException e) {
          isValidArgument = true;
        }
        break;
    }

    if (!isValidArgument) {
      throw new IllegalArgumentException("Invalid argument " + mName + ": " + value);
    }
  }
}
