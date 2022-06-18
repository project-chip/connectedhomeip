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

public interface KeypadInputManager {

  int KEY_CODE_SELECT = 0;
  int KEY_CODE_UP = 1;
  int KEY_CODE_DOWN = 2;
  int KEY_CODE_LEFT = 3;
  int KEY_CODE_RIGHT = 4;
  int KEY_CODE_RIGHT_UP = 5;
  int KEY_CODE_RIGHT_DOWN = 6;
  int KEY_CODE_LEFT_UP = 7;
  int KEY_CODE_LEFT_DOWN = 8;
  int KEY_CODE_ROOT_MENU = 9;
  int KEY_CODE_SETUP_MENU = 10;
  int KEY_CODE_CONTENTS_MENU = 11;
  int KEY_CODE_FAVORITE_MENU = 12;
  int KEY_CODE_EXIT = 13;
  int KEY_CODE_MEDIA_TOP_MENU = 16;
  int KEY_CODE_MEDIA_CONTEXT_SENSITIVE_MENU = 17;
  int KEY_CODE_NUMBER_ENTRY_MODE = 29;
  int KEY_CODE_NUMBER11 = 30;
  int KEY_CODE_NUMBER12 = 31;
  int KEY_CODE_NUMBER0_OR_NUMBER10 = 32;
  int KEY_CODE_NUMBERS1 = 33;
  int KEY_CODE_NUMBERS2 = 34;
  int KEY_CODE_NUMBERS3 = 35;
  int KEY_CODE_NUMBERS4 = 36;
  int KEY_CODE_NUMBERS5 = 37;
  int KEY_CODE_NUMBERS6 = 38;
  int KEY_CODE_NUMBERS7 = 39;
  int KEY_CODE_NUMBERS8 = 40;
  int KEY_CODE_NUMBERS9 = 41;
  int KEY_CODE_DOT = 42;
  int KEY_CODE_ENTER = 43;
  int KEY_CODE_CLEAR = 44;
  int KEY_CODE_NEXT_FAVORITE = 47;
  int KEY_CODE_CHANNEL_UP = 48;
  int KEY_CODE_CHANNEL_DOWN = 49;
  int KEY_CODE_PREVIOUS_CHANNEL = 50;
  int KEY_CODE_SOUND_SELECT = 51;
  int KEY_CODE_INPUT_SELECT = 52;
  int KEY_CODE_DISPLAY_INFORMATION = 53;
  int KEY_CODE_HELP = 54;
  int KEY_CODE_PAGE_UP = 55;
  int KEY_CODE_PAGE_DOWN = 56;
  int KEY_CODE_POWER = 64;
  int KEY_CODE_VOLUME_UP = 65;
  int KEY_CODE_VOLUME_DOWN = 66;
  int KEY_CODE_MUTE = 67;
  int KEY_CODE_PLAY = 68;
  int KEY_CODE_STOP = 69;
  int KEY_CODE_PAUSE = 70;
  int KEY_CODE_RECORD = 71;
  int KEY_CODE_REWIND = 72;
  int KEY_CODE_FAST_FORWARD = 73;
  int KEY_CODE_EJECT = 74;
  int KEY_CODE_FORWARD = 75;
  int KEY_CODE_BACKWARD = 76;
  int KEY_CODE_STOP_RECORD = 77;
  int KEY_CODE_PAUSE_RECORD = 78;
  int KEY_CODE_RESERVED = 79;
  int KEY_CODE_ANGLE = 80;
  int KEY_CODE_SUB_PICTURE = 81;
  int KEY_CODE_VIDEO_ON_DEMAND = 82;
  int KEY_CODE_ELECTRONIC_PROGRAM_GUIDE = 83;
  int KEY_CODE_TIMER_PROGRAMMING = 84;
  int KEY_CODE_INITIAL_CONFIGURATION = 85;
  int KEY_CODE_SELECT_BROADCAST_TYPE = 86;
  int KEY_CODE_SELECT_SOUND_PRESENTATION = 87;
  int KEY_CODE_PLAY_FUNCTION = 96;
  int KEY_CODE_PAUSE_PLAY_FUNCTION = 97;
  int KEY_CODE_RECORD_FUNCTION = 98;
  int KEY_CODE_PAUSE_RECORD_FUNCTION = 99;
  int KEY_CODE_STOP_FUNCTION = 100;
  int KEY_CODE_MUTE_FUNCTION = 101;
  int KEY_CODE_RESTORE_VOLUME_FUNCTION = 102;
  int KEY_CODE_TUNE_FUNCTION = 103;
  int KEY_CODE_SELECT_MEDIA_FUNCTION = 104;
  int KEY_CODE_SELECT_AV_INPUT_FUNCTION = 105;
  int KEY_CODE_SELECT_AUDIO_INPUT_FUNCTION = 106;
  int KEY_CODE_POWER_TOGGLE_FUNCTION = 107;
  int KEY_CODE_POWER_OFF_FUNCTION = 108;
  int KEY_CODE_POWER_ON_FUNCTION = 109;
  int KEY_CODE_F1_BLUE = 113;
  int KEY_CODE_F2_RED = 114;
  int KEY_CODE_F3_GREEN = 115;
  int KEY_CODE_F4_YELLOW = 116;
  int KEY_CODE_F5 = 117;
  int KEY_CODE_DATA = 118;

  int KEY_STATUS_SUCCESS = 0;
  int KEY_STATUS_UNSUPPORTED_KEY = 1;
  int KEY_STATUS_INVALID_KEY_IN_CURRENT_STATE = 1;

  /**
   * Inject a key code to system
   *
   * @param keyCode keys in KEY_CODE_XXX
   * @return KEY_STATUS_XXX above
   */
  int sendKey(int keyCode);
}
