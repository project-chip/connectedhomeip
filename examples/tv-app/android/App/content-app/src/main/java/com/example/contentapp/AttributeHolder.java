package com.example.contentapp;

import com.matter.tv.app.api.Clusters;
import java.util.HashMap;
import java.util.Map;

/** Class to hold attribute values to help test attribute read and subscribe use cases. */
public class AttributeHolder {
  private static AttributeHolder instance = new AttributeHolder();
  private Map<Long, Map<Long, Object>> attributeValues = new HashMap<>();

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
        Clusters.TargetNavigator.Id,
        Clusters.TargetNavigator.Attributes.TargetList,
        "[{\"0\":1, \"1\":\"Home\"},{\"0\":2, \"1\":\"Settings\"},{\"0\":3, \"1\":\"Casting Home\"}]");
    setAttributeValue(
        Clusters.TargetNavigator.Id, Clusters.TargetNavigator.Attributes.CurrentTarget, 1);
  };

  public static AttributeHolder getInstance() {
    return instance;
  }

  public void setAttributeValue(long clusterId, long attributeId, Object value) {
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
