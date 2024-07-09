/*
 *   Copyright (c) 2021-2024 Project CHIP Authors
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
package com.matter.casting;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import chip.platform.AndroidChipPlatformException;
import chip.platform.ConfigurationManager;
import chip.platform.KeyValueStoreManager;
import java.util.Base64;
import java.util.Map;
import java.util.UUID;

/** Java interface for ConfigurationManager */
public class PreferencesConfigurationManager implements ConfigurationManager {

  private final String TAG = KeyValueStoreManager.class.getSimpleName();
  private SharedPreferences preferences;

  public PreferencesConfigurationManager(Context context, String preferenceFileKey) {
    preferences = context.getSharedPreferences(preferenceFileKey, Context.MODE_PRIVATE);

    try {
      String keyUniqueId = getKey(kConfigNamespace_ChipFactory, kConfigKey_UniqueId);
      if (!preferences.contains(keyUniqueId)) {
        preferences
            .edit()
            .putString(keyUniqueId, UUID.randomUUID().toString().replaceAll("-", ""))
            .apply();
      }
    } catch (AndroidChipPlatformException e) {
      e.printStackTrace();
    }
  }

  @Override
  public long readConfigValueLong(String namespace, String name)
      throws AndroidChipPlatformException {
    String key = getKey(namespace, name);
    switch (key) {
        /**
         * The unique id assigned by the device vendor to identify the product or device type. This
         * number is scoped to the device vendor id. return a different value than
         * src/include/platform/CHIPDeviceConfig.h for debug
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_ProductId:
        return 0x8003;

        /**
         * The default hardware version number assigned to the device or product by the device
         * vendor.
         *
         * <p>Hardware versions are specific to a particular device vendor and product id, and
         * typically correspond to a revision of the physical device, a change to its packaging,
         * and/or a change to its marketing presentation. This value is generally *not* incremented
         * for device software revisions.
         *
         * <p>This is a default value which is used when a hardware version has not been stored in
         * device persistent storage (e.g. by a factory provisioning process).
         *
         * <p>return a different value than src/include/platform/CHIPDeviceConfig.h for debug
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_HardwareVersion:
        return 1;

        /**
         * A monothonic number identifying the software version running on the device.
         *
         * <p>return a different value than src/include/platform/CHIPDeviceConfig.h for debug
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_SoftwareVersion:
        return 1;

        /** Matter Casting Video Client has device type ID 41 (i.e. 0x0029) */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_DeviceTypeId:
        return 41;
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
         * Human readable name of the device model. return a different value than
         * src/include/platform/CHIPDeviceConfig.h for debug
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_ProductName:
        return "TEST_ANDROID_PRODUCT";

        /**
         * Human readable string identifying version of the product assigned by the device vendor.
         *
         * <p>return a different value than src/include/platform/CHIPDeviceConfig.h for debug
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_HardwareVersionString:
        return "TEST_ANDROID_VERSION";

        /**
         * A string identifying the software version running on the device.
         *
         * <p>return a different value than src/include/platform/CHIPDeviceConfig.h for debug
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_SoftwareVersionString:
        return "prerelease(android)";

        /**
         * The ManufacturingDate attribute SHALL specify the date that the Node was manufactured.
         * The first 8 characters SHALL specify the date of manufacture of the Node in international
         * date notation according to ISO 8601, i.e., YYYYMMDD, e.g., 20060814. The final 8
         * characters MAY include country, factory, line, shift or other related information at the
         * option of the vendor. The format of this information is vendor defined.
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_ManufacturingDate:
        return "2021-12-06";

        /**
         * Enables the use of a hard-coded default serial number if none * is found in Chip NV
         * storage.
         *
         * <p>return a different value than src/include/platform/CHIPDeviceConfig.h for debug
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_SerialNum:
        return "TEST_ANDROID_SN";

        /**
         * The PartNumber attribute SHALL specify a human-readable (displayable) vendor assigned
         * part number for the Node whose meaning and numbering scheme is vendor defined. Multiple
         * products (and hence PartNumbers) can share a ProductID. For instance, there may be
         * different packaging (with different PartNumbers) for different regions; also different
         * colors of a product might share the ProductID but may have a different PartNumber.
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_PartNumber:
        return "TEST_ANDROID_PRODUCT_BLUE";

        /**
         * The ProductURL attribute SHALL specify a link to a product specific web page. The syntax
         * of the ProductURL attribute SHALL follow the syntax as specified in RFC 3986. The
         * specified URL SHOULD resolve to a maintained web page available for the lifetime of the
         * product. The maximum length of the ProductUrl attribute is 256 ASCII characters.
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_ProductURL:
        return "https://buildwithmatter.com/";

        /**
         * The ProductLabel attribute SHALL specify a vendor specific human readable (displayable)
         * product label. The ProductLabel attribute MAY be used to provide a more user-friendly
         * value than that represented by the ProductName attribute. The ProductLabel attribute
         * SHOULD NOT include the name of the vendor as defined within the VendorName attribute.
         */
      case kConfigNamespace_ChipFactory + ":" + kConfigKey_ProductLabel:
        return "X10";
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
