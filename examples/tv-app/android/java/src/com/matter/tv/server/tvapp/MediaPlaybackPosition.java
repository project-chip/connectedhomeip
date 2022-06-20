package com.matter.tv.server.tvapp;

import java.util.Date;

public class MediaPlaybackPosition {
  private long updatedAt;
  private long position;

  public MediaPlaybackPosition(long position) {
    this.position = position;
    this.updatedAt = new Date().getTime() * 1000;
  }
}
