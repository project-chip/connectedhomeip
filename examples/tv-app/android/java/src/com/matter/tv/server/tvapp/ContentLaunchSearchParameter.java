package com.matter.tv.server.tvapp;

import java.util.Map;

public class ContentLaunchSearchParameter {

  public ContentLaunchSearchParameter(ContentLaunchSearchParameterType type, String data) {
    this.type = type;
    this.data = data;
  }
  /** content data type in TYPE_XXX */
  public ContentLaunchSearchParameterType type;

  /** The entity value, which is a search string, ex. "Manchester by the Sea". */
  public String data;

  /** This object defines additional name=value pairs that can be used for identifying content. */
  public Map.Entry<String, String>[] externalIDList;
}
