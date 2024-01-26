package com.matter.tv.server.tvapp;


public class MediaTrack {
  public String id;
  public String languageCode;
  public String displayName;

  public MediaTrack(String id, String languageCode, String displayName) {
    this.id = id;
    this.languageCode = languageCode;
    this.displayName = displayName;
  }
}
