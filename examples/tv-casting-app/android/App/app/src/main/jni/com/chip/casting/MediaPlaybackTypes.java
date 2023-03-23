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
package com.chip.casting;

public class MediaPlaybackTypes {
  public enum PlaybackStateEnum {
    Playing,
    Paused,
    NotPlaying,
    Buffering,
    Unknown
  }

  public static class PlaybackPosition {
    public Long updatedAt;
    public Long position;

    public PlaybackPosition(Long updatedAt) {
      this.updatedAt = updatedAt;
    }

    public PlaybackPosition(Long updatedAt, Long position) {
      this.updatedAt = updatedAt;
      this.position = position;
    }

    @Override
    public String toString() {
      StringBuilder output = new StringBuilder();
      output.append("PlaybackPosition {\n");
      output.append("\tupdatedAt: ");
      output.append(updatedAt);
      output.append("\n");
      output.append("\tposition: ");
      output.append(position);
      output.append("\n");
      output.append("}\n");
      return output.toString();
    }
  }
}
