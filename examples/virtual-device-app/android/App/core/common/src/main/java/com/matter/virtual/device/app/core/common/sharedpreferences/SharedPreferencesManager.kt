package com.matter.virtual.device.app.core.common.sharedpreferences

import android.content.Context
import android.content.SharedPreferences
import dagger.hilt.android.qualifiers.ApplicationContext
import javax.inject.Inject
import javax.inject.Singleton
import timber.log.Timber

@Singleton
class SharedPreferencesManager
@Inject
constructor(@ApplicationContext private val context: Context) {

  private val sharedPreferences: SharedPreferences by lazy {
    context.getSharedPreferences(PREFERENCES_NAME, Context.MODE_PRIVATE)
  }

  fun getString(key: String): String {
    val value = sharedPreferences.getString(key, DEFAULT_VALUE_STRING)
    return value ?: ""
  }

  fun setString(key: String, value: String) {
    val editor = sharedPreferences.edit()
    editor.putString(key, value)
    editor.apply()
  }

  fun getBoolean(key: String): Boolean {
    return sharedPreferences.getBoolean(key, DEFAULT_VALUE_BOOLEAN)
  }

  fun setBoolean(key: String, value: Boolean) {
    val editor = sharedPreferences.edit()
    editor.putBoolean(key, value)
    editor.apply()
  }

  fun deleteMatterSharedPreferences() {
    Timber.d("deleteMatterSharedPreferences()")
    if (!context.deleteSharedPreferences(PREFERENCES_NAME_MATTER_KEY_VALUE_STORE)) {
      Timber.e("delete failure($PREFERENCES_NAME_MATTER_KEY_VALUE_STORE)")
    }

    if (!context.deleteSharedPreferences(PREFERENCES_NAME_MATTER_CONFIGURATION_MANAGER)) {
      Timber.e("delete failure($PREFERENCES_NAME_MATTER_CONFIGURATION_MANAGER)")
    }
  }

  companion object {
    private const val PREFERENCES_NAME = "virtualdeviceapp"
    private const val PREFERENCES_NAME_MATTER_KEY_VALUE_STORE = "chip.platform.KeyValueStore"
    private const val PREFERENCES_NAME_MATTER_CONFIGURATION_MANAGER =
      "chip.platform.ConfigurationManager"
    private const val DEFAULT_VALUE_STRING = ""
    private const val DEFAULT_VALUE_BOOLEAN = false
  }
}
