/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

/** Android implementation of a key/value store using SharedPreferences. */
public class PreferencesKeyValueStoreManager implements KeyValueStoreManager {
  private final String TAG = KeyValueStoreManager.class.getSimpleName();
  private final String PREFERENCE_FILE_KEY = "chip.platform.KeyValueStore";
  private SharedPreferences preferences;

  public PreferencesKeyValueStoreManager(Context context) {
    preferences = context.getSharedPreferences(PREFERENCE_FILE_KEY, Context.MODE_PRIVATE);
  }

  @Override
  public String get(String key) {
    String value = preferences.getString(key, null);
    if (value == null) {
      Log.d(TAG, "Key '" + key + "' not found in shared preferences");
    }
    return value;
  }

  @Override
  public void set(String key, String value) {
    preferences.edit().putString(key, value).apply();
  }

  @Override
  public void delete(String key) {
    preferences.edit().remove(key).apply();
  }
}
