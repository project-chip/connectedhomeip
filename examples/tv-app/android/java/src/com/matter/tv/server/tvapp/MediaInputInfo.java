/*
 *   Copyright (c) 2021 Project CHIP Authors
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
package com.matter.tv.server.tvapp;

public class MediaInputInfo {

  public static final int INPUT_TYPE_INTERNAL = 0;
  public static final int INPUT_TYPE_AUX = 1;
  public static final int INPUT_TYPE_COAX = 2;
  public static final int INPUT_TYPE_COMPOSITE = 3;
  public static final int INPUT_TYPE_HDMI = 4;
  public static final int INPUT_TYPE_INPUT = 5;
  public static final int INPUT_TYPE_LINE = 6;
  public static final int INPUT_TYPE_OPTICAL = 7;
  public static final int INPUT_TYPE_VIDEO = 8;
  public static final int INPUT_TYPE_SCART = 9;
  public static final int INPUT_TYPE_USB = 10;
  public static final int INPUT_TYPE_OTHER = 11;

  /** The unique index into the list of Inputs. */
  public int index;

  /** The type of input in INPUT_TYPE_XXX */
  public int type;

  /**
   * The input name, such as "HDMI 1". This field may be blank, but SHOULD be provided when known.
   */
  public String name;

  /**
   * The user editable input description, such as "Living room Playstation". This field may be
   * blank, but SHOULD be provided when known.
   */
  public String description;
}
