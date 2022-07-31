package com.matter.tv.server.tvapp;

import android.util.Log;

public class ContentLaunchManagerStub implements ContentLaunchManager {

  private final String TAG = ContentLaunchManagerStub.class.getSimpleName();

  private int endpoint;

  private ContentLaunchSearchParameter[] tvShowSearchList = {
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.ACTOR, "Gaby sHofmann"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.CHANNEL, "PBS"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.CHARACTER, "Snow White"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.DIRECTOR, "Spike Lee"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.FRANCHISE, "Star Wars"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.GENRE, "Horror"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.POPULARITY, "Popularity"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.PROVIDER, "Netfxlix"),
  };

  private ContentLaunchSearchParameter[] sportsShowSearchList = {
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.EVENT, "Football games"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.LEAGUE, "NCAA"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.SPORT, "football"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.SPORTS_TEAM, "Arsenal"),
    new ContentLaunchSearchParameter(ContentLaunchSearchParameterType.TYPE, "TVSeries"),
  };

  private ContentLaunchEntry[] entries = {
    new ContentLaunchEntry("Sports Example", sportsShowSearchList),
    new ContentLaunchEntry("TV Show Example", tvShowSearchList)
  };

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

    boolean found = false;
    for (ContentLaunchEntry entry : entries) {
      for (ContentLaunchSearchParameter parameter : entry.parameters) {
        for (ContentLaunchSearchParameter query : search) {
          if (query.type == parameter.type && query.data.equals(parameter.data)) {
            Log.d(TAG, " TEST CASE found match=" + entry.name);
            found = true;
          }
        }
      }
    }

    if (!found) {
      Log.d(TAG, " TEST CASE did not find a match");
    }

    if ("err".equals(data)) {
      return new ContentLaunchResponse(
          ContentLaunchResponse.STATUS_URL_NOT_AVAILABLE, "Error data in Java");
    }
    return new ContentLaunchResponse(ContentLaunchResponse.STATUS_SUCCESS, "Example data in Java");
  }

  @Override
  public ContentLaunchResponse launchUrl(
      String url, String display, ContentLaunchBrandingInformation branding) {
    Log.d(
        TAG,
        "launchUrl:"
            + url
            + " display="
            + display
            + " branding.providerName="
            + (branding == null ? "NULL" : branding.providerName)
            + " at "
            + endpoint);

    if ("err".equals(display)) {
      return new ContentLaunchResponse(
          ContentLaunchResponse.STATUS_URL_NOT_AVAILABLE, "Error data in Java");
    } else if (url.equals("https://badurl")) {
      return new ContentLaunchResponse(ContentLaunchResponse.STATUS_URL_NOT_AVAILABLE, "Bad URL");
    } else if (url.equals("https://csa-iot.org/badauth")) {
      return new ContentLaunchResponse(ContentLaunchResponse.STATUS_AUTH_FAILED, "Auth Failed");
    }
    return new ContentLaunchResponse(ContentLaunchResponse.STATUS_SUCCESS, "Example data in Java");
  }
}
