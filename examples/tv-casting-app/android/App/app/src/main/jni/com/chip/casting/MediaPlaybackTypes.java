/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
