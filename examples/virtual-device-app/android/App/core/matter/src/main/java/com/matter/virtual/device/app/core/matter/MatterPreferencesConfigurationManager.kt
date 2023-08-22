package com.matter.virtual.device.app.core.matter

import android.content.Context
import android.content.SharedPreferences
import chip.platform.AndroidChipPlatformException
import chip.platform.ConfigurationManager
import com.matter.virtual.device.app.core.common.MatterConstants
import java.util.Base64
import java.util.UUID
import timber.log.Timber

class MatterPreferencesConfigurationManager(
  context: Context,
  private val deviceTypeId: Long,
  private val deviceName: String,
  private val discriminator: Int
) : ConfigurationManager {

  private val sharedPreferences: SharedPreferences =
    context.getSharedPreferences(PREFERENCE_FILE_KEY, Context.MODE_PRIVATE).apply {
      try {
        val uniqueId =
          getKey(
            ConfigurationManager.kConfigNamespace_ChipFactory,
            ConfigurationManager.kConfigKey_UniqueId
          )
        if (!this.contains(uniqueId)) {
          this.edit().putString(uniqueId, UUID.randomUUID().toString().replace("-", "")).apply()
        }
      } catch (e: Exception) {
        Timber.e(e, "Error : handling a uniqueId")
      }
    }

  override fun readConfigValueLong(namespace: String?, name: String?): Long {
    val key = getKey(namespace, name)

    when (key) {
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_ProductId ->
        return MatterConstants.DEFAULT_PRODUCT_ID.toLong()
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_HardwareVersion -> return 1
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_SoftwareVersion -> return 1
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_DeviceTypeId -> return deviceTypeId
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_SetupDiscriminator -> return discriminator.toLong()
    }

    if (sharedPreferences.contains(key)) {
      return sharedPreferences.getLong(key, Long.MAX_VALUE)
    } else {
      Timber.e("Key $key not found in shared preferences")
      throw AndroidChipPlatformException()
    }
  }

  override fun readConfigValueStr(namespace: String?, name: String?): String? {
    val key = getKey(namespace, name)

    when (key) {
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_ProductName -> return "VIRTUAL_DEVICE_APP"
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_HardwareVersionString ->
        return "VIRTUAL_DEVICE_APP_SOFTWARE_VERSION"
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_SoftwareVersionString ->
        return "VIRTUAL_DEVICE_APP_SOFTWARE_VERSION"
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_ManufacturingDate -> return "2023-08-22"
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_SerialNum -> return "VIRTUAL_DEVICE_APP_SN"
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_PartNumber -> return "VIRTUAL_DEVICE_APP_PN"
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_ProductURL -> return "https://buildwithmatter.com/"
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_ProductLabel -> return "VIRTUAL_DEVICE_APP"
      ConfigurationManager.kConfigNamespace_ChipFactory +
        ":" +
        ConfigurationManager.kConfigKey_DeviceName -> return deviceName
    }

    if (sharedPreferences.contains(key)) {
      return sharedPreferences.getString(key, null)
    } else {
      Timber.e("Key $key not found in shared preferences")
      throw AndroidChipPlatformException()
    }
  }

  override fun readConfigValueBin(namespace: String?, name: String?): ByteArray {
    val key = getKey(namespace, name)

    if (sharedPreferences.contains(key)) {
      val value = sharedPreferences.getString(key, null)
      return Base64.getDecoder().decode(value)
    } else {
      Timber.e("Key $key not found in shared preferences")
      throw AndroidChipPlatformException()
    }
  }

  override fun writeConfigValueLong(namespace: String?, name: String?, `val`: Long) {
    val key = getKey(namespace, name)
    sharedPreferences.edit().putLong(key, `val`).apply()
  }

  override fun writeConfigValueStr(namespace: String?, name: String?, `val`: String?) {
    val key = getKey(namespace, name)
    sharedPreferences.edit().putString(key, `val`).apply()
  }

  override fun writeConfigValueBin(namespace: String?, name: String?, `val`: ByteArray?) {
    val key = getKey(namespace, name)
    if (`val` != null) {
      val valStr = Base64.getEncoder().encodeToString(`val`)
      sharedPreferences.edit().putString(key, valStr).apply()
    } else {
      sharedPreferences.edit().remove(key).apply()
    }
  }

  override fun clearConfigValue(namespace: String?, name: String?) {
    if (namespace != null && name != null) {
      sharedPreferences.edit().remove(getKey(namespace, name)).apply()
    } else if (namespace != null && name == null) {
      val pre = getKey(namespace, name = null)
      val editor = sharedPreferences.edit()
      val allEntries = sharedPreferences.all
      allEntries.entries.forEach {
        val key = it.key
        if (key.startsWith(pre)) {
          editor.remove(key)
        }
      }
      editor.apply()
    } else if (namespace == null && name == null) {
      sharedPreferences.edit().clear().apply()
    }
  }

  override fun configValueExists(namespace: String?, name: String?): Boolean {
    return sharedPreferences.contains(getKey(namespace, name))
  }

  private fun getKey(namespace: String?, name: String?): String {
    if (namespace != null && name != null) {
      return "$namespace:$name"
    } else if (namespace != null && name == null) {
      return "$namespace:"
    }

    throw AndroidChipPlatformException()
  }

  companion object {
    private const val PREFERENCE_FILE_KEY = "chip.platform.ConfigurationManager"
  }
}
