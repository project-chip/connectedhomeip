package com.example.contentapp;

import android.util.Log;
import com.matter.tv.app.api.Clusters;
import java.util.HashMap;
import java.util.Map;

/** Class to hold attribute values to help test attribute read and subscribe use cases. */
public class AttributeHolder {
  private Map<Long, Map<Long, Object>> attributeValues = new HashMap<>();
  private static final String TAG = "AttributeHolder";

  // The following are the various values of the target list that can be set using the UI of the
  // content app for testing.
  // We have a short version (TL_SHORT) which is a simple target list. A long version (TL_LONG)
  // which has several large values in the target list but always less than the single message
  // limit(can be chunked) and a bad version (TL_LONG_BAD) which puts a value that is larger
  // than can be handled by a single message and hence cannot be chunked.
  // This is only for testing. These are public since they are used in the Main Activity(which
  // renders the UI).
  public static final String TL_LONG =
      "[{\"0\":1, \"1\":\"Home\"},{\"0\":2, \"1\":\"Settings\"},{\"0\":3, \"1\":\"Casting Home\"}"
          + ",{\"0\":249, \"1\":\""
          + "1skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt\"}"
          + ",{\"0\":250, \"1\":\""
          + "2skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt\"}"
          + ",{\"0\":251, \"1\":\""
          + "3skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt\"}"
          + ",{\"0\":252, \"1\":\""
          + "4skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt\"}"
          + ",{\"0\":253, \"1\":\""
          + "5skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt\"}"
          + ",{\"0\":254, \"1\":\""
          + "6skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt\"}"
          + "]";
  public static final String TL_SHORT =
      "[{\"0\":1, \"1\":\"Home\"},{\"0\":2, \"1\":\"Settings\"},{\"0\":3, \"1\":\"Casting Home\"}]";
  public static final String TL_LONG_BAD =
      "[{\"0\":1, \"1\":\"Home\"},{\"0\":2, \"1\":\"Settings\"},{\"0\":3, \"1\":\"Casting Home\"}"
          + ",{\"0\":254, \"1\":\""
          + "badskfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt"
          + "skfguioufgirufgieufgifugaeifugaeifugadifugbdifugadfiugawdfiuawgdfiuawdbvawiufvafuvwiufgwieufgdhtskhtdhhtbbsagthdkgusdfjgfghdgrbt\"}"
          + "]";
  private static AttributeHolder instance = new AttributeHolder();

  private AttributeHolder() {
    // Setting up attribute defaults
    setAttributeValue(
        Clusters.ContentLauncher.Id,
        Clusters.ContentLauncher.Attributes.AcceptHeader,
        "[\"video/mp4\", \"application/x-mpegURL\", \"application/dash+xml\"]");
    setAttributeValue(
        Clusters.ContentLauncher.Id,
        Clusters.ContentLauncher.Attributes.SupportedStreamingProtocols,
        3);
    setAttributeValue(Clusters.MediaPlayback.Id, Clusters.MediaPlayback.Attributes.CurrentState, 2);
    setAttributeValue(
        Clusters.TargetNavigator.Id, Clusters.TargetNavigator.Attributes.TargetList, TL_SHORT);
    setAttributeValue(
        Clusters.TargetNavigator.Id, Clusters.TargetNavigator.Attributes.CurrentTarget, 1);
  };

  public static AttributeHolder getInstance() {
    return instance;
  }

  public void setAttributeValue(long clusterId, long attributeId, Object value) {
    if (value == null) {
      Log.d(TAG, "Setting null for cluster " + clusterId + " attribute " + attributeId);
    }
    Map<Long, Object> attributes = attributeValues.get(clusterId);
    if (attributes == null) {
      attributes = new HashMap<>();
      attributeValues.put(clusterId, attributes);
    }
    attributes.put(attributeId, value);
  }

  public Object getAttributeValue(long clusterId, long attributeId) {
    Map<Long, Object> attributes = attributeValues.get(clusterId);
    return attributes.get(attributeId);
  }
}
