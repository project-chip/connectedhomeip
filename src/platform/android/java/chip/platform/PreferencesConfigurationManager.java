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
package chip.platform;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import java.util.Base64;
import java.util.Map;

/** Java interface for ConfigurationManager */
public class PreferencesConfigurationManager implements ConfigurationManager {

  private final String TAG = KeyValueStoreManager.class.getSimpleName();
  private final String PREFERENCE_FILE_KEY = "chip.platform.ConfigurationManager";
  private SharedPreferences preferences;

  public PreferencesConfigurationManager(Context context) {
    preferences = context.getSharedPreferences(PREFERENCE_FILE_KEY, Context.MODE_PRIVATE);
  }

  @Override
  public long readConfigValueLong(String namespace, String name)
      throws AndroidChipPlatformException {
    String key = getKey(namespace, name);

    switch (key) {
      /**
       * The unique id assigned by the device vendor to identify the product or device type.  This
       * number is scoped to the device vendor id. return a different value than src/include/platform/CHIPDeviceConfig.h for debug
       */
      case kConfigNamespace_ChipFactory+":"+kConfigKey_ProductId:
        return 65278;

      /**
       * The default hardware version number assigned to the device or product by the device vendor.
       *
       * Hardware versions are specific to a particular device vendor and product id, and typically
       * correspond to a revision of the physical device, a change to its packaging, and/or a change
       * to its marketing presentation. This value is generally *not* incremented for device software
       * revisions.
       *
       * This is a default value which is used when a hardware version has not been stored in device
       * persistent storage (e.g. by a factory provisioning process).
       *
       * return a different value than src/include/platform/CHIPDeviceConfig.h for debug
       */
      case kConfigNamespace_ChipFactory+":"+kConfigKey_HardwareVersion:
        return 1;

      /**
       * A monothonic number identifying the software version running on the device.
       *
       * return a different value than src/include/platform/CHIPDeviceConfig.h for debug
       */
      case kConfigNamespace_ChipFactory+":"+kConfigKey_SoftwareVersion:
        return 1;
    }

    if (preferences.contains(key)) {
      long value = preferences.getLong(key, Long.MAX_VALUE);
      return value;
    } else {
      Log.d(TAG, "Key '" + key + "' not found in shared preferences");
      throw new AndroidChipPlatformException();
    }
  }

  @Override
  public String readConfigValueStr(String namespace, String name)
      throws AndroidChipPlatformException {
    String key = getKey(namespace, name);

    switch (key) {
      /**
       * Human readable name of the device model.
       * return a different value than src/include/platform/CHIPDeviceConfig.h for debug
       */
      case kConfigNamespace_ChipFactory+":"+kConfigKey_ProductName:
        return "TEST_ANDROID_PRODUCT";

      /**
       * Human readable string identifying version of the product assigned by the device vendor.
       *
       * return a different value than src/include/platform/CHIPDeviceConfig.h for debug
       */
      case kConfigNamespace_ChipFactory+":"+kConfigKey_HardwareVersionString:
        return "TEST_ANDROID_VERSION";

      /**
       * A string identifying the software version running on the device.
       *
       * return a different value than src/include/platform/CHIPDeviceConfig.h for debug
       */
      case kConfigNamespace_ChipFactory+":"+kConfigKey_SoftwareVersionString:
        return "prerelease(android)";
    }

    if (preferences.contains(key)) {
      String value = preferences.getString(key, null);
      return value;
    } else {
      Log.d(TAG, "Key '" + key + "' not found in shared preferences");
      throw new AndroidChipPlatformException();
    }
  }

  @Override
  public byte[] readConfigValueBin(String namespace, String name)
      throws AndroidChipPlatformException {
    String key = getKey(namespace, name);
    if (preferences.contains(key)) {
      String value = preferences.getString(key, null);
      byte[] byteValue = Base64.getDecoder().decode(value);
      return byteValue;
    } else {
      Log.d(TAG, "Key '" + key + "' not found in shared preferences");
      throw new AndroidChipPlatformException();
    }
  }

  @Override
  public void writeConfigValueLong(String namespace, String name, long val)
      throws AndroidChipPlatformException {
    String key = getKey(namespace, name);
    preferences.edit().putLong(key, val).apply();
  }

  @Override
  public void writeConfigValueStr(String namespace, String name, String val)
      throws AndroidChipPlatformException {
    String key = getKey(namespace, name);
    preferences.edit().putString(key, val).apply();
  }

  @Override
  public void writeConfigValueBin(String namespace, String name, byte[] val)
      throws AndroidChipPlatformException {
    String key = getKey(namespace, name);
    if (val != null) {
      String valStr = Base64.getEncoder().encodeToString(val);
      preferences.edit().putString(key, valStr).apply();
    } else {
      preferences.edit().remove(key).apply();
    }
  }

  @Override
  public void clearConfigValue(String namespace, String name) throws AndroidChipPlatformException {
    if (namespace != null && name != null) {
      preferences.edit().remove(getKey(namespace, name)).apply();
    } else if (namespace != null && name == null) {
      String pre = getKey(namespace, null);
      SharedPreferences.Editor editor = preferences.edit();
      Map<String, ?> allEntries = preferences.getAll();
      for (Map.Entry<String, ?> entry : allEntries.entrySet()) {
        String key = entry.getKey();
        if (key.startsWith(pre)) {
          editor.remove(key);
        }
      }
      editor.apply();
    } else if (namespace == null && name == null) {
      preferences.edit().clear().apply();
    }
  }

  @Override
  public boolean configValueExists(String namespace, String name)
      throws AndroidChipPlatformException {
    return preferences.contains(getKey(namespace, name));
  }

  private String getKey(String namespace, String name) throws AndroidChipPlatformException {
    if (namespace != null && name != null) {
      return namespace + ":" + name;
    } else if (namespace != null && name == null) {
      return namespace + ":";
    }

    throw new AndroidChipPlatformException();
  }
}
