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

/**
 * Java interface for ConfigurationManager
 */
public class DefaultConfigurationManager implements ConfigurationManager {

    private final String TAG = KeyValueStoreManager.class.getSimpleName();
    private final String PREFERENCE_FILE_KEY = "chip.platform.ConfigurationManager";
    private SharedPreferences preferences;

    public DefaultConfigurationManager(Context context) {
        preferences = context.getSharedPreferences(PREFERENCE_FILE_KEY, Context.MODE_PRIVATE);
    }

    @Override
    public long readConfigValueLong(String namespace, String name) throws AndroidChipPlatformException {
        String key = getKey(namespace,name);
        long value = preferences.getLong(key, Long.MAX_VALUE);
        if (value == Long.MAX_VALUE) {
            Log.d(TAG, "Key '" + key + "' not found in shared preferences");
            throw new AndroidChipPlatformException();
        }
        return value;
    }

    @Override
    public String readConfigValueStr(String namespace, String name) throws AndroidChipPlatformException {
        String key = getKey(namespace,name);
        String value = preferences.getString(key, null);
        if (value == null) {
            Log.d(TAG, "Key '" + key + "' not found in shared preferences");
            throw new AndroidChipPlatformException();
        }
        return value;
    }

    @Override
    public byte[] readConfigValueBin(String namespace, String name) throws AndroidChipPlatformException {
        String key = getKey(namespace,name);
        String value = preferences.getString(key, null);
        if (value == null) {
            Log.d(TAG, "Key '" + key + "' not found in shared preferences");
            throw new AndroidChipPlatformException();
        }
        byte[] byteValue = Base64.getDecoder().decode(value);
        return byteValue;
    }

    @Override
    public void writeConfigValueLong(String namespace, String name, long val) throws AndroidChipPlatformException {
        String key = getKey(namespace,name);
        preferences.edit().putLong(key, val).apply();
    }

    @Override
    public void writeConfigValueStr(String namespace, String name, String val) throws AndroidChipPlatformException  {
        String key = getKey(namespace,name);
        preferences.edit().putString(key, val).apply();
    }

    @Override
    public void writeConfigValueBin(String namespace, String name, byte[] val) throws AndroidChipPlatformException  {
        String key = getKey(namespace,name);
        if(val != null) {
            String valStr = Base64.getEncoder().encodeToString(val);
            preferences.edit().putString(key, valStr).apply();
        } else {
            preferences.edit().remove(key).apply();
        }
    }

    @Override
    public void clearConfigValue(String namespace, String name) throws AndroidChipPlatformException  {
        if(namespace != null && name != null) {
            preferences.edit().remove(getKey(namespace,name)).apply();;
        } else if(namespace != null && name == null) {
            String pre = getKey(namespace, null);
            SharedPreferences.Editor editor = preferences.edit();
            Map<String, ?> allEntries = preferences.getAll();
            for (Map.Entry<String, ?> entry : allEntries.entrySet()) {
                String key = entry.getKey();
                if(key.startsWith(pre)) {
                    editor.remove(key);
                }
            }
            editor.apply();
        } else if(namespace == null && name == null) {
            preferences.edit().clear().apply();
        }
    }

    @Override
    public boolean configValueExists(String namespace, String name) throws AndroidChipPlatformException  {
        return preferences.contains(getKey(namespace,name));
    }

    private String getKey(String namespace, String name) throws AndroidChipPlatformException {
        if(namespace != null && name != null) {
            return namespace + ":" + name;
        } else if(namespace != null && name == null) {
            return namespace + ":";
        }

        throw new AndroidChipPlatformException();
    }
}
