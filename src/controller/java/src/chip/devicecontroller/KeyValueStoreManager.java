/*
 *   Copyright (c) 2021 Project CHIP Authors
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
package chip.devicecontroller;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

/**
 * Java implementation of a key/value store.
 *
 * <p>Exposes get/set/delete methods to be used by the native C++ JNI Layer.
 */
public class KeyValueStoreManager {
  private static final String TAG = KeyValueStoreManager.class.getSimpleName();
  private static SharedPreferences preferences;
  private static final String PREFERENCE_FILE_KEY = "com.google.chip.KeyValueStore";

  public static String get(String key) {
    String value = preferences.getString(key, null);
    if (value == null) {
      Log.d(TAG, "Key '" + key + "' not found in shared preferences");
    }
    return value;
  }

  public static void set(String key, String value) {
    preferences.edit().putString(key, value).apply();
  }

  public static void delete(String key) {
    preferences.edit().remove(key).apply();
  }

  /** Initialization MUST be done before any of get/set/delete work. */
  public static void initialize(Context context) {
    preferences = context.getSharedPreferences(PREFERENCE_FILE_KEY, Context.MODE_PRIVATE);
  }
}
