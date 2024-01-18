package com.matter.virtual.device.app.core.matter

import android.content.Context
import chip.appserver.ChipAppServer
import chip.appserver.ChipAppServerDelegate
import chip.platform.*
import com.matter.virtual.device.app.Clusters
import com.matter.virtual.device.app.DeviceApp
import com.matter.virtual.device.app.DeviceAppCallback
import com.matter.virtual.device.app.DeviceEventType
import com.matter.virtual.device.app.core.common.MatterConstants
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.matter.manager.DoorLockManagerStub
import com.matter.virtual.device.app.core.matter.manager.OnOffManagerStub
import com.matter.virtual.device.app.core.matter.manager.PowerSourceManagerStub
import dagger.hilt.android.qualifiers.ApplicationContext
import javax.inject.Inject
import javax.inject.Singleton
import timber.log.Timber

@Singleton
class MatterApp
@Inject
constructor(
  @ApplicationContext private val context: Context,
  private val deviceApp: DeviceApp,
  private val onOffManagerStub: OnOffManagerStub,
  private val doorLockManagerStub: DoorLockManagerStub,
  private val powerSourceManagerStub: PowerSourceManagerStub
) {

  private var androidChipPlatform: AndroidChipPlatform? = null
  private var chipAppServer: ChipAppServer? = null
  private val deviceEventCallbackList = ArrayList<MatterDeviceEventCallback>()

  fun start(matterSettings: MatterSettings) {
    Timber.d("start():$matterSettings")

    deviceApp.setCallback(
      object : DeviceAppCallback {
        override fun onClusterInit(app: DeviceApp, clusterId: Long, endpoint: Int) {
          Timber.d("onClusterInit():clusterId:$clusterId,endpoint:$endpoint")
          when (clusterId) {
            Clusters.ClusterId_OnOff -> {
              app.setOnOffManager(endpoint, onOffManagerStub)
              onOffManagerStub.initAttributeValue()
            }
            Clusters.ClusterId_DoorLock -> {
              app.setDoorLockManager(endpoint, doorLockManagerStub)
              doorLockManagerStub.initAttributeValue()
            }
            Clusters.ClusterId_PowerSource -> {
              app.setPowerSourceManager(endpoint, powerSourceManagerStub)
              powerSourceManagerStub.initAttributeValue()
            }
          }
        }

        override fun onEvent(event: Long) {
          Timber.d("onEvent():event:$event")

          when (event) {
            DeviceEventType.EventId_DnssdInitialized -> {
              Timber.d("DNS-SD Platform Initialized")
            }
            DeviceEventType.EventId_CHIPoBLEConnectionEstablished -> {
              Timber.d("BLE Connection Established")
            }
            DeviceEventType.EventId_CommissioningComplete -> {
              Timber.d("Commissioning Complete")
              deviceEventCallbackList.forEach { callback -> callback.onCommissioningCompleted() }
            }
            DeviceEventType.EventId_FabricRemoved -> {
              Timber.d("Fabric Removed")
              deviceEventCallbackList.forEach { callback -> callback.onFabricRemoved() }
            }
          }
        }
      }
    )

    androidChipPlatform =
      AndroidChipPlatform(
        AndroidBleManager(),
        PreferencesKeyValueStoreManager(context),
        MatterPreferencesConfigurationManager(
          context,
          matterSettings.device.deviceTypeId,
          context.resources.getString(matterSettings.device.deviceNameResId),
          matterSettings.discriminator
        ),
        NsdManagerServiceResolver(context),
        NsdManagerServiceBrowser(context),
        ChipMdnsCallbackImpl(),
        DiagnosticDataProviderImpl(context)
      )

    androidChipPlatform?.updateCommissionableDataProviderData(
      MatterConstants.TEST_SPAKE2P_VERIFIER,
      MatterConstants.TEST_SPAKE2P_SALT,
      MatterConstants.TEST_SPAKE2P_ITERATION_COUNT,
      MatterConstants.TEST_SETUP_PASSCODE,
      matterSettings.discriminator
    )

    deviceApp.preServerInit()

    chipAppServer = ChipAppServer()
    chipAppServer?.startAppWithDelegate(
      object : ChipAppServerDelegate {
        override fun onCommissioningSessionEstablishmentStarted() {
          Timber.d("onCommissioningSessionEstablishmentStarted()")
          deviceEventCallbackList.forEach { callback ->
            callback.onCommissioningSessionEstablishmentStarted()
          }
        }

        override fun onCommissioningSessionStarted() {
          Timber.d("onCommissioningSessionStarted()")
        }

        override fun onCommissioningSessionEstablishmentError(errorCode: Int) {
          Timber.d("onCommissioningSessionEstablishmentError():$errorCode")
          deviceEventCallbackList.forEach { callback ->
            callback.onCommissioningSessionEstablishmentError(errorCode)
          }
        }

        override fun onCommissioningSessionStopped() {
          Timber.d("onCommissioningSessionStopped()")
        }

        override fun onCommissioningWindowOpened() {
          Timber.d("onCommissioningWindowOpened()")
        }

        override fun onCommissioningWindowClosed() {
          Timber.d("onCommissioningWindowClosed()")
        }
      }
    )

    deviceApp.postServerInit(matterSettings.device.deviceTypeId.toInt())
  }

  fun stop() {
    chipAppServer?.stopApp()
  }

  fun reset() {
    chipAppServer?.resetApp()
  }

  fun addDeviceEventCallback(deviceEventCallback: MatterDeviceEventCallback) {
    Timber.d("Hit")
    if (!this.deviceEventCallbackList.contains(deviceEventCallback)) {
      Timber.d("Add")
      this.deviceEventCallbackList.add(deviceEventCallback)
    }
  }

  fun removeDeviceEventCallback(deviceEventCallback: MatterDeviceEventCallback) {
    Timber.d("Hit")
    if (this.deviceEventCallbackList.contains(deviceEventCallback)) {
      Timber.d("Remove")
      this.deviceEventCallbackList.remove(deviceEventCallback)
    }
  }
}
