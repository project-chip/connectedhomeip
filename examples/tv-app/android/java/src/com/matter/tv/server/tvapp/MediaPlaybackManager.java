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

public interface MediaPlaybackManager {

  /** The current playback state of media in PLAYBACK_STATE_XXXX */
  int ATTRIBUTE_PLAYBACK_STATE = 0x0000;

  /** The start time of the media in milliseconds (UTC time) */
  int ATTRIBUTE_PLAYBACK_START_TIME = 0x0001;

  /** The duration, in milliseconds or 0 if the duration is not applicable */
  int ATTRIBUTE_PLAYBACK_DURATION = 0x0002;

  /**
   * The speed at which the current media is being played. 10000 means 1X, 5000 means 0.5X and 20000
   * means 2X
   */
  int ATTRIBUTE_PLAYBACK_SPEED = 0x0003;

  /** The earliest valid position to which a client MAY seek back, in milliseconds */
  int ATTRIBUTE_PLAYBACK_SEEK_RANGE_END = 0x0004;

  /** The furthest forward valid position to which a client MAY seek forward, in milliseconds */
  int ATTRIBUTE_PLAYBACK_SEEK_RANGE_START = 0x0005;

  int PLAYBACK_STATE_PLAYING = 0;
  int PLAYBACK_STATE_PAUSED = 1;
  int PLAYBACK_STATE_NOT_PLAYING = 2;
  int PLAYBACK_STATE_BUFFERING = 3;

  /**
   * Play media. If content is currently in a FastForward or Rewind state. Play SHALL return media
   * to normal playback speed.
   */
  int REQUEST_PLAY = 0;

  /** Pause playback of the media. */
  int REQUEST_PAUSE = 1;

  /** Stop playback of the media */
  int REQUEST_STOP = 2;

  /** Start Over with the current media playback item. */
  int REQUEST_START_OVER = 3;

  /** Go back to the previous media playback item. */
  int REQUEST_PREVIOUS = 4;

  /** Go forward to the next media playback item. */
  int REQUEST_NEXT = 5;

  /**
   * Start playback of the media backward in case the media is currently playing in the forward
   * direction or is not playing.
   */
  int REQUEST_REWIND = 6;

  /**
   * Start playback of the media in the forward direction in case the media is currently playing in
   * the backward direction or is not playing. If the playback is already happening in the forward
   * direction receipt of this command SHALL increase the speed of the media playback. Different
   * "fast-forward" speeds MAY be be reflected on the media playback device based upon the number of
   * sequential calls to this function and the capability of the device.
   */
  int REQUEST_FAST_FORWARD = 7;

  /** Skip forward in the media by the given number of milliseconds */
  int REQUEST_SKIP_FORWARD = 8;

  /** Skip backward in the media by the given number of milliseconds */
  int REQUEST_SKIP_BACKWARD = 9;

  /**
   * Change the playback position in the media to the given position by the given number of
   * milliseconds
   */
  int REQUEST_SEEK = 10;

  int RESPONSE_STATUS_SUCCESS = 0;
  int RESPONSE_STATUS_INVALID_STATE_FOR_COMMAND = 1;
  int RESPONSE_STATUS_NOT_ALLOWED = 2;
  int RESPONSE_STATUS_NOT_ACTIVE = 3;
  int RESPONSE_STATUS_SPEED_OUT_OF_RANGE = 4;
  int RESPONSE_STATUS_SEEK_OUT_OF_RANGE = 5;

  /**
   * Get the attribute by id in ATTRIBUTE_PLAYBACK_XXX
   *
   * @return The value of return defines in ATTRIBUTE_PLAYBACK_XXX, or -1 on error
   */
  long getAttributes(int attributesId);

  /**
   * Request for cmd in REQUEST_XXX, the parameters for request is defined in REQUEST_XXX
   *
   * @param parameter The means of parameter is defined in RESPONSE_STATUS_XXX or meaningless if not
   *     defined.
   * @return the response status defined in RESPONSE_STATUS_XXX
   */
  int request(int cmd, long parameter);

  /** Get the position of playback (Position field) at the time (UpdateAt field) specified */
  MediaPlaybackPosition getPosition();
}
