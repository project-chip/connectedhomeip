/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
