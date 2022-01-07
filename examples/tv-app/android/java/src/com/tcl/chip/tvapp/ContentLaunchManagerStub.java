package com.tcl.chip.tvapp;

import android.util.Log;

public class ContentLaunchManagerStub implements ContentLaunchManager {

  private final String TAG = ContentLaunchManagerStub.class.getSimpleName();

  private int endpoint;

  public ContentLaunchManagerStub(int endpoint) {
    this.endpoint = endpoint;
  }

  @Override
  public String[] getAcceptHeader() {
    String[] headers =
        new String[] {
          "application/dash+xml", "application/vnd.apple.mpegurl", "text/html",
        };

    return headers;
  }

  @Override
  public int[] getSupportedStreamingProtocols() {
    int[] types = new int[] {STREAMING_TYPE_DASH, STREAMING_TYPE_HLS};
    return types;
  }

  @Override
  public ContentLaunchResponse launchContent(
      ContentLaunchSearchParameter[] search, boolean autoplay, String data) {
    Log.d(TAG, "launchContent:" + data + " autoplay=" + autoplay + " at " + endpoint);
    ContentLaunchResponse resp =
        new ContentLaunchResponse(ContentLaunchResponse.STATUS_SUCCESS, "Example data in Java");
    return resp;
  }

  @Override
  public ContentLaunchResponse launchUrl(
      String url, String display, ContentLaunchBrandingInformation branding) {
    Log.d(TAG, "launchUrl:" + url + " display=" + display + " at " + endpoint);
    ContentLaunchResponse resp =
        new ContentLaunchResponse(ContentLaunchResponse.STATUS_SUCCESS, "Example data in Java");
    return resp;
  }
}
