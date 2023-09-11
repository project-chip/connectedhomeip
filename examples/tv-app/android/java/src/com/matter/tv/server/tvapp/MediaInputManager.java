/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.tv.server.tvapp;

public interface MediaInputManager {

  /** Return media inputs supported by the device */
  MediaInputInfo[] getInputList();

  /** @return The index field of the currently selected {@link MediaInputInfo} */
  int getCurrentInput();

  /**
   * change the media input on the device to the input at a specific index in the Input List.
   *
   * @param index The index in {@link MediaInputInfo}
   */
  boolean selectInput(int index);

  /** Display the active status of the input list on screen. */
  boolean showInputStatus();

  /** Hide the input list from the screen. */
  boolean hideInputStatus();

  /**
   * Rename the input at a specific index in the Input List. Updates to the input name SHALL appear
   * in the device's settings menus.
   *
   * @param index The index in {@link MediaInputInfo}
   * @param name The new input name
   */
  boolean renameInput(int index, String name);
}
