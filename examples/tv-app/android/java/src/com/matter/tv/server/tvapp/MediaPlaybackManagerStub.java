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

import android.util.Log;

/** Stub implement of MediaPlaybackManager, TV manufacture should have there own implements */
public class MediaPlaybackManagerStub implements MediaPlaybackManager {

  private final String TAG = MediaPlaybackManagerStub.class.getSimpleName();

  private int endpoint;
  private int playbackState = PLAYBACK_STATE_PLAYING;
  private int playbackSpeed = 1;
  private long playbackPosition = 0;
  private long playbackDuration = 5 * 60 * 1000;
  private long startTime = 100;

  private static int playbackMaxForwardSpeed = 10;
  private static int playbackMaxRewindSpeed = -10;

  private static MediaTrack[] audioTracks = {
    new MediaTrack("activeAudioTrackId_0", "languageCode1", "displayName1"),
    new MediaTrack("activeAudioTrackId_1", "languageCode2", "displayName2")
  };
  private static MediaTrack activeAudioTrack = audioTracks[0];

  private static MediaTrack activeTextTrack = null;
  private static MediaTrack[] textTracks = {
    new MediaTrack("activeTextTrackId_0", "languageCode1", "displayName1"),
    new MediaTrack("activeTextTrackId_1", "languageCode2", "displayName2")
  };

  public MediaPlaybackManagerStub(int endpoint) {
    this.endpoint = endpoint;
  }

  @Override
  public long getAttributes(int attributesId) {
    switch (attributesId) {
      case ATTRIBUTE_PLAYBACK_STATE:
        Log.d(TAG, "getAttributes CurrentState at " + endpoint);
        return playbackState;

      case ATTRIBUTE_PLAYBACK_START_TIME:
        Log.d(TAG, "getAttributes StartTime at " + endpoint);
        return startTime;

      case ATTRIBUTE_PLAYBACK_DURATION:
        Log.d(TAG, "getAttributes Duration at " + endpoint);
        return playbackDuration;

      case ATTRIBUTE_PLAYBACK_SPEED:
        Log.d(
            TAG,
            "getAttributes SampledPosition PlaybackSpeed " + playbackSpeed + " at " + endpoint);
        return playbackSpeed;

      case ATTRIBUTE_PLAYBACK_SEEK_RANGE_END:
        Log.d(
            TAG,
            "getAttributes SampledPosition SeekRangeEnd " + playbackDuration + " at " + endpoint);
        return playbackDuration;

      case ATTRIBUTE_PLAYBACK_SEEK_RANGE_START:
        Log.d(TAG, "getAttributes SampledPosition SeekRangeStart " + startTime + " at " + endpoint);
        return startTime;

        // TODO: add audio/text track available/active attributes
    }

    return -1;
  }

  @Override
  public int request(int cmd, long parameter) {
    long newPosition;
    switch (cmd) {
      case REQUEST_PLAY:
        Log.d(TAG, "request Play at " + endpoint);
        playbackState = PLAYBACK_STATE_PLAYING;
        playbackSpeed = 1;

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_PAUSE:
        Log.d(TAG, "request pause at " + endpoint);
        playbackState = PLAYBACK_STATE_PAUSED;
        playbackSpeed = 0;

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_STOP:
        Log.d(TAG, "request stop at " + endpoint);
        playbackState = PLAYBACK_STATE_NOT_PLAYING;
        playbackSpeed = 0;

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_START_OVER:
        Log.d(TAG, "request start over at " + endpoint);
        playbackPosition = 0;

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_PREVIOUS:
        Log.d(TAG, "request previous at " + endpoint);
        playbackState = PLAYBACK_STATE_PLAYING;
        playbackSpeed = 1;
        playbackPosition = 0;

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_NEXT:
        Log.d(TAG, "request next at " + endpoint);
        playbackState = PLAYBACK_STATE_PLAYING;
        playbackSpeed = 1;
        playbackPosition = 0;

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_REWIND:
        Log.d(TAG, "request rewind at " + endpoint);
        if (playbackSpeed == playbackMaxRewindSpeed) {
          return RESPONSE_STATUS_SPEED_OUT_OF_RANGE;
        }
        playbackState = PLAYBACK_STATE_PLAYING;
        playbackSpeed =
            (playbackSpeed >= 0 ? -1 : Math.max(playbackSpeed * 2, playbackMaxRewindSpeed));

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_FAST_FORWARD:
        Log.d(TAG, "request fast forward at " + endpoint);
        if (playbackSpeed == playbackMaxForwardSpeed) {
          return RESPONSE_STATUS_SPEED_OUT_OF_RANGE;
        }
        playbackState = PLAYBACK_STATE_PLAYING;
        playbackSpeed =
            (playbackSpeed <= 0 ? 1 : Math.min(playbackSpeed * 2, playbackMaxForwardSpeed));

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_SKIP_FORWARD:
        Log.d(TAG, "request skip forward " + parameter + " milliseconds at " + endpoint);
        newPosition = playbackPosition + parameter;
        playbackPosition =
            (newPosition > playbackDuration)
                ? playbackDuration
                : (newPosition >= 0 ? newPosition : 0);

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_SKIP_BACKWARD:
        Log.d(TAG, "request skip backward " + parameter + " milliseconds at " + endpoint);
        newPosition = playbackPosition - parameter;
        playbackPosition =
            (newPosition > playbackDuration)
                ? playbackDuration
                : (newPosition >= 0 ? newPosition : 0);

        return RESPONSE_STATUS_SUCCESS;

      case REQUEST_SEEK:
        Log.d(TAG, "request seek to " + parameter + " milliseconds at " + endpoint);

        if (parameter > playbackDuration) {
          return RESPONSE_STATUS_SEEK_OUT_OF_RANGE;
        } else {
          playbackPosition = parameter;
        }

        return RESPONSE_STATUS_SUCCESS;
    }

    return RESPONSE_STATUS_NOT_ALLOWED;
  }

  @Override
  public MediaPlaybackPosition getPosition() {
    Log.d(TAG, "getPosition " + playbackPosition);
    return new MediaPlaybackPosition(playbackPosition);
  }

  @Override
  public MediaTrack[] getAvailableTracks(boolean audio) {
    return (audio ? audioTracks : textTracks);
  }

  @Override
  public int activateTrack(boolean audio, String id) {
    if (audio) {
      for (MediaTrack track : audioTracks) {
        if (track.id.equalsIgnoreCase(id)) {
          activeAudioTrack = track;
          return RESPONSE_STATUS_SUCCESS;
        }
      }
      return RESPONSE_STATUS_NOT_ALLOWED; // TODO
    } else {
      for (MediaTrack track : textTracks) {
        if (track.id.equalsIgnoreCase(id)) {
          activeTextTrack = track;
          return RESPONSE_STATUS_SUCCESS;
        }
      }
      return RESPONSE_STATUS_NOT_ALLOWED; // TODO
    }
  }

  @Override
  public int deactivateTextTrack() {
    activeTextTrack = null;
    return RESPONSE_STATUS_SUCCESS;
  }

  @Override
  public MediaTrack getActiveTrack(boolean audio) {
    return (audio ? activeAudioTrack : activeTextTrack);
  }
}
