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
  public long getSupportedStreamingProtocols() {
    long protocols = STREAMING_TYPE_DASH | STREAMING_TYPE_HLS;
    return protocols;
  }

  @Override
  public ContentLaunchResponse launchContent(
      ContentLaunchSearchParameter[] search, boolean autoplay, String data) {
    Log.d(TAG, "launchContent:" + data + " autoplay=" + autoplay + " at " + endpoint);

    if ("err".equals(data)) {
      return new ContentLaunchResponse(
          ContentLaunchResponse.STATUS_URL_NOT_AVAILABLE, "Error data in Java");
    }
    return new ContentLaunchResponse(ContentLaunchResponse.STATUS_SUCCESS, "Example data in Java");
  }

  @Override
  public ContentLaunchResponse launchUrl(
      String url, String display, ContentLaunchBrandingInformation branding) {
    Log.d(TAG, "launchUrl:" + url + " display=" + display + " at " + endpoint);

    if ("err".equals(display)) {
      return new ContentLaunchResponse(
          ContentLaunchResponse.STATUS_URL_NOT_AVAILABLE, "Error data in Java");
    }
    return new ContentLaunchResponse(ContentLaunchResponse.STATUS_SUCCESS, "Example data in Java");
  }
}
