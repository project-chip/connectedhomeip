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

package com.tcl.chip.tvapp;

import android.util.Log;

/** Stub implement of MediaPlaybackManager, TV manufacture should have there own implements */
public class MediaPlaybackManagerStub implements MediaPlaybackManager {

  private final String TAG = MediaPlaybackManagerStub.class.getSimpleName();

  private int endpoint;

  public MediaPlaybackManagerStub(int endpoint) {
    this.endpoint = endpoint;
  }

  @Override
  public long getAttributes(int attributesId) {
    switch (attributesId) {
      case ATTRIBUTE_PLAYBACK_STATE:
        Log.d(TAG, "getAttributes CurrentState at " + endpoint);
        return PLAYBACK_STATE_PLAYING;

      case ATTRIBUTE_PLAYBACK_START_TIME:
        Log.d(TAG, "getAttributes StartTime at " + endpoint);
        return 100;

      case ATTRIBUTE_PLAYBACK_DURATION:
        Log.d(TAG, "getAttributes Duration at " + endpoint);
        return 5 * 60 * 1000;

      case ATTRIBUTE_PLAYBACK_SPEED:
        Log.d(TAG, "getAttributes SampledPosition PlaybackSpeed at " + endpoint);
        return 10000;

      case ATTRIBUTE_PLAYBACK_SEEK_RANGE_END:
        Log.d(TAG, "getAttributes SampledPosition SeekRangeEnd at " + endpoint);
        return 5 * 60 * 1000;

      case ATTRIBUTE_PLAYBACK_SEEK_RANGE_START:
        Log.d(TAG, "getAttributes SampledPosition SeekRangeStart at " + endpoint);
        return 200;
    }

    return -1;
  }

  @Override
  public int request(int cmd, long parameter) {
    switch (cmd) {
      case REQUEST_PLAY:
        Log.d(TAG, "request Play at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_PAUSE:
        Log.d(TAG, "request pause at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_STOP:
        Log.d(TAG, "request stop at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_START_OVER:
        Log.d(TAG, "request start over at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_PREVIOUS:
        Log.d(TAG, "request previous at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_NEXT:
        Log.d(TAG, "request next at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_REWIND:
        Log.d(TAG, "request rewind at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_FAST_FORWARD:
        Log.d(TAG, "request fast forward at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_SKIP_FORWARD:
        Log.d(TAG, "request skip forward " + parameter + " milliseconds at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_SKIP_BACKWARD:
        Log.d(TAG, "request skip backward " + parameter + " milliseconds at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_SEEK:
        Log.d(TAG, "request seek to " + parameter + " milliseconds at " + endpoint);
        return RESPONSE_STATUS_SUCCESS;
    }

    return RESPONSE_STATUS_NOT_ALLOWED;
  }

  @Override
  public MediaPlaybackPosition getPosition() {
    return new MediaPlaybackPosition(3 * 60 * 1000);
  }
}
