package com.matter.tv.server.tvapp;

public class LauncherResponse {

  public static final int STATUS_SUCCESS = 0;
  public static final int STATUS_APP_NOT_AVAILABLE = 1;
  public static final int STATUS_SYSTEM_BUSY = 2;
  public static final int STATUS_PENDING_USER_APPROVAL = 3;
  public static final int STATUS_DOWNLOADING = 4;
  public static final int STATUS_INSTALLING = 5;

  public LauncherResponse(int status, String data) {
    this.status = status;
    this.data = data;
  }

  /** The status in STATUS_XXX */
  public int status;

  /** Optional app-specific data. */
  public String data;
}
