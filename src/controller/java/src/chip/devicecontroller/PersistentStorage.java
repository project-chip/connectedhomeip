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
 * Persistent storage class, meant to be called by the CHIP SDK. Defaults to use SharedPreferences
 * for storage, or uses a delegate if set.
 */
public class PersistentStorage {
  private static final String TAG = PersistentStorage.class.getSimpleName();
  private static SharedPreferences preferences;
  private static AndroidPersistentStorageDelegate storageDelegate;

  private static final String PREFERENCE_FILE_KEY =
      "com.google.chip.devicecontroller.PREFERENCE_FILE_KEY";

  public static String getKeyValue(String key) {
    Log.d(TAG, "getKeyValue called with key: " + key);

    if (storageDelegate != null) {
      return storageDelegate.getKeyValue(key);
    } else {
      String value = preferences.getString(key, null);
      if (value == null) {
        Log.d(TAG, "Key " + key + " not found in shared preferences");
      }
      return value;
    }
  }

  public static void setKeyValue(String key, String value) {
    Log.d(TAG, "setKeyValue called with key: " + key + " value: " + value);

    if (storageDelegate != null) {
      storageDelegate.setKeyValue(key, value);
    } else {
      preferences.edit().putString(key, value).apply();
    }
  }

  public static void deleteKeyValue(String key) {
    Log.d(TAG, "deleteKeyValue called with key: " + key);

    if (storageDelegate != null) {
      storageDelegate.deleteKeyValue(key);
    } else {
      preferences.edit().remove(key).apply();
    }
  }

  public static void setStorageDelegate(AndroidPersistentStorageDelegate newStorageDelegate) {
    storageDelegate = newStorageDelegate;
  }

  /** Initialize storage with the given context. Not required if a delegate is set. */
  public static void initialize(Context context) {
    preferences = context.getSharedPreferences(PREFERENCE_FILE_KEY, Context.MODE_PRIVATE);
  }
}
