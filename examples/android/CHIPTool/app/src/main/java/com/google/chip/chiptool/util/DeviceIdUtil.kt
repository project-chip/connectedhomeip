package com.google.chip.chiptool.util

import android.content.Context
import android.content.SharedPreferences

/** Utils for storing and accessing available device IDs using shared preferences. */
object DeviceIdUtil {
  private const val PREFERENCE_FILE_KEY = "com.google.chip.chiptool.PREFERENCE_FILE_KEY"
  private const val DEVICE_ID_PREFS_KEY = "device_id"
  private const val DEVICE_ID_LIST_PREFS_KEY = "device_id_list"
  private const val DEFAULT_DEVICE_ID = 1L
  private const val HEX_RADIX = 16

  fun getNextAvailableId(context: Context): Long {
    val prefs = getPrefs(context)
    return if (prefs.contains(DEVICE_ID_PREFS_KEY)) {
      prefs.getLong(DEVICE_ID_PREFS_KEY, DEFAULT_DEVICE_ID)
    } else {
      prefs.edit().putLong(DEVICE_ID_PREFS_KEY, DEFAULT_DEVICE_ID).apply()
      DEFAULT_DEVICE_ID
    }
  }

  fun setNextAvailableId(context: Context, newId: Long) {
    getPrefs(context).edit().putLong(DEVICE_ID_PREFS_KEY, newId).apply()
  }

  fun setCommissionedNodeId(context: Context, nodeId: Long) {
    val nodeList = getCommissionedNodeId(context)

    nodeList.add(nodeId.toULong().toString(HEX_RADIX))

    getPrefs(context).edit().putStringSet(DEVICE_ID_LIST_PREFS_KEY, nodeList.toSet()).apply()
  }

  fun removeCommissionedNodeId(context: Context, nodeId: Long) {
    val nodeList = getCommissionedNodeId(context)
    if (!nodeList.contains(nodeId.toULong().toString())) {
      return
    }

    nodeList.remove(nodeId.toULong().toString())
    getPrefs(context).edit().putStringSet(DEVICE_ID_LIST_PREFS_KEY, nodeList.toSet()).apply()
  }

  fun getCommissionedNodeId(context: Context): ArrayList<String> {
    val nodeList = getPrefs(context).getStringSet(DEVICE_ID_LIST_PREFS_KEY, setOf()) ?: setOf()
    return ArrayList(nodeList.toList())
  }

  fun getLastDeviceId(context: Context): Long = getNextAvailableId(context) - 1

  private fun getPrefs(context: Context): SharedPreferences {
    return context.getSharedPreferences(PREFERENCE_FILE_KEY, Context.MODE_PRIVATE)
  }
}
