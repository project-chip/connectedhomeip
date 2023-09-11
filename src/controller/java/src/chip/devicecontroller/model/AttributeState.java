/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller.model;

import android.util.Log;
import org.json.JSONException;
import org.json.JSONObject;

/** Represents the reported value of an attribute in object form, TLV and JSON. */
public final class AttributeState {
  private static final String TAG = "AttributeState";

  private final Object valueObject;
  private final byte[] tlv;
  private JSONObject json;

  public AttributeState(Object valueObject, byte[] tlv, String jsonString) {
    this.valueObject = valueObject;
    this.tlv = tlv;
    try {
      this.json = new JSONObject(jsonString);
    } catch (JSONException ex) {
      Log.e(TAG, "Error parsing JSON string", ex);
    }
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
}
