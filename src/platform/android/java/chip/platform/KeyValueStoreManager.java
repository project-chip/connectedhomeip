/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

/**
 * Java interface for a key/value store.
 *
 * <p>Exposes get/set/delete methods to be used by the native C++ JNI Layer.
 */
public interface KeyValueStoreManager {
  public String get(String key);

  public void set(String key, String value);

  public void delete(String key);
}
