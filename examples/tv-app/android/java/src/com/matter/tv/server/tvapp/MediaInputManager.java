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
