/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package chip.devicecontroller.model;

import android.util.Log;
import org.json.JSONException;
import org.json.JSONObject;

/** Represents the reported value of an attribute in object form, TLV and JSON. */
public final class EventState {
  public static final int MILLIS_SINCE_BOOT = 0;
  public static final int MILLIS_SINCE_EPOCH = 1;
  private static final String TAG = "EventState";

  private final long eventNumber;
  private final int priorityLevel;
  private final int timestampType;
  private final long timestampValue;

  private final Object valueObject;
  private final byte[] tlv;
  private JSONObject json;

  public EventState(
      long eventNumber,
      int priorityLevel,
      int timestampType,
      long timestampValue,
      Object valueObject,
      byte[] tlv,
      String jsonString) {
    this.eventNumber = eventNumber;
    this.priorityLevel = priorityLevel;
    this.timestampType = timestampType;
    this.timestampValue = timestampValue;

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

  public int getTimestampType() {
    return timestampType;
  }

  public long getTimestampValue() {
    return timestampValue;
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
