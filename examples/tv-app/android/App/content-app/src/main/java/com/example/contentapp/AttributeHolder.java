package com.example.contentapp;

import java.util.HashMap;
import java.util.Map;

/** Class to hold attribute values to help test attribute read and subscribe use cases. */
public class AttributeHolder {
  private static AttributeHolder instance = new AttributeHolder();
  private Map<Long, Map<Long, Object>> attributeValues = new HashMap<>();

  private AttributeHolder() {};

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
