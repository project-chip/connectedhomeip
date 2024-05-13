package com.matter.tv.server.tvapp;

public class ContentLaunchResponse {

  public static final int STATUS_SUCCESS = 0;
  public static final int STATUS_URL_NOT_AVAILABLE = 1;
  public static final int STATUS_AUTH_FAILED = 2;

  public ContentLaunchResponse(int status, String data) {
    this.status = status;
    this.data = data;
  }

  /** The status in STATUS_XXX */
  public int status;

  /** Optional app-specific data. */
  public String data;
}
