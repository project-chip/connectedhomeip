/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.model;

import android.util.Log;
import org.json.JSONException;
import org.json.JSONObject;

/** Represents the reported value of an attribute in object form, TLV and JSON. */
public final class EventState {
  private static final String TAG = "EventState";

  private long eventNumber;
  private int priorityLevel;
  private long systemTimeStamp;

  private Object valueObject;
  private byte[] tlv;
  private JSONObject json;

  public EventState(
      long eventNumber,
      int priorityLevel,
      long systemTimeStamp,
      Object valueObject,
      byte[] tlv,
      String jsonString) {
    this.eventNumber = eventNumber;
    this.priorityLevel = priorityLevel;
    this.systemTimeStamp = systemTimeStamp;

    this.valueObject = valueObject;
    this.tlv = tlv;
    try {
      this.json = new JSONObject(jsonString);
    } catch (JSONException ex) {
      Log.e(TAG, "Error parsing JSON string", ex);
    }
  }

  public long getEventNumber() {
    return eventNumber;
  }

  public int getPriorityLevel() {
    return priorityLevel;
  }

  public long getSystemTimeStamp() {
    return systemTimeStamp;
  }

  public Object getValue() {
    return valueObject;
  }

  /**
   * Return a byte array containing the TLV for an attribute, wrapped within an anonymous TLV tag.
   */
  public byte[] getTlv() {
    return tlv;
  }

  public JSONObject getJson() {
    return json;
  }

  @Override
  public String toString() {
    return valueObject.toString();
  }
}
