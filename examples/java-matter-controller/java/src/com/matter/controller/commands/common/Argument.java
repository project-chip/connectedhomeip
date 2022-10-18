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

public class Argument {
  public static final byte kOptional = (1 << 0);
  public static final byte kNullable = (1 << 1);
  public String name;
  public ArgumentType type;
  public long min;
  public long max;
  public byte flags;
  public Object value;
  public String desc;

  public boolean isOptional() {
    return ((flags & kOptional) != 0);
  }

  public boolean isNullable() {
    return ((flags & kNullable) != 0);
  }
}
