/*
 *   Copyright (c) 2020 Project CHIP Authors
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

package com.google.chip.chiptool.commissioner.thread;

import io.openthread.commissioner.ByteArray;

public class CommissionerUtils {

  public static byte[] getByteArray(ByteArray byteArray) {
    byte[] ret = new byte[byteArray.size()];
    for (int i = 0; i < byteArray.size(); ++i) {
      ret[i] = byteArray.get(i);
    }
    return ret;
  }

  public static String getHexString(byte[] byteArray) {
    StringBuilder strbuilder = new StringBuilder();

    for (byte b : byteArray) {
      strbuilder.append(String.format("%02x", b));
    }
    return strbuilder.toString();
  }

  public static String getHexString(ByteArray byteArray) {
    return getHexString(getByteArray(byteArray));
  }
}
