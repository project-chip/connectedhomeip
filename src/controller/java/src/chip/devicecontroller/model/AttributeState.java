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
public final class AttributeState {
  private static final String TAG = "AttributeState";

  private Object valueObject;
  private byte[] tlv;
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
