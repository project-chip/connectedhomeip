package com.google.chip.chiptool.bluetooth

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothProfile
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.ParcelUuid
import android.util.Log
import chip.platform.BleCallback
import com.google.chip.chiptool.ChipClient
import java.util.UUID
import kotlin.coroutines.resume
import kotlinx.coroutines.CancellableContinuation
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.channels.ProducerScope
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withTimeoutOrNull

@ExperimentalCoroutinesApi
class BluetoothManager : BleCallback {
  private val bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
  private var bleGatt: BluetoothGatt? = null
  var connectionId = 0
    private set

  private fun getServiceData(discriminator: Int): ByteArray {
    val opcode = 0
    val version = 0
    val versionDiscriminator = ((version and 0xf) shl 12) or (discriminator and 0xfff)
    return intArrayOf(opcode, versionDiscriminator, versionDiscriminator shr 8)
      .map { it.toByte() }
      .toByteArray()
  }

  private fun getServiceDataMask(isShortDiscriminator: Boolean): ByteArray {
    val shortDiscriminatorMask =
      when (isShortDiscriminator) {
        true -> 0x00
        false -> 0xff
      }
    return intArrayOf(0xff, shortDiscriminatorMask, 0xff).map { it.toByte() }.toByteArray()
  }

  suspend fun getBluetoothDevice(context: Context, discriminator: Int): BluetoothDevice? {
    return getBluetoothDevice(context, discriminator, false)
  }

  suspend fun getBluetoothDevice(
    context: Context,
    discriminator: Int,
    isShortDiscriminator: Boolean
  ): BluetoothDevice? {
    if (!bluetoothAdapter.isEnabled) {
      bluetoothAdapter.enable()
    }

    val scanner =
      bluetoothAdapter.bluetoothLeScanner
        ?: run {
          Log.e(TAG, "No bluetooth scanner found")
          return null
        }

    return withTimeoutOrNull(10000) {
      callbackFlow {
          val scanCallback =
            object : ScanCallback() {
              override fun onScanResult(callbackType: Int, result: ScanResult) {
                val device = result.device
                Log.i(TAG, "Bluetooth Device Scanned Addr: ${device.address}, Name ${device.name}")

                val producerScope: ProducerScope<BluetoothDevice> = this@callbackFlow
                if (producerScope.channel.isClosedForSend) {
                  Log.w(TAG, "Bluetooth device was scanned, but channel is already closed")
                } else {
                  offer(device)
                }
              }

              override fun onScanFailed(errorCode: Int) {
                Log.e(TAG, "Scan failed $errorCode")
              }
            }

          val serviceData = getServiceData(discriminator)
          val serviceDataMask = getServiceDataMask(isShortDiscriminator)

          val scanFilter =
            ScanFilter.Builder()
              .setServiceData(ParcelUuid(UUID.fromString(CHIP_UUID)), serviceData, serviceDataMask)
              .build()

          val scanSettings =
            ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build()

          Log.i(TAG, "Starting Bluetooth scan")
          scanner.startScan(listOf(scanFilter), scanSettings, scanCallback)
          awaitClose { scanner.stopScan(scanCallback) }
        }
        .first()
    }
  }

  /**
   * Connects to a [BluetoothDevice] and suspends until [BluetoothGattCallback.onServicesDiscovered]
   */
  suspend fun connect(context: Context, device: BluetoothDevice): BluetoothGatt? {
    return suspendCancellableCoroutine { continuation ->
      val bluetoothGattCallback = getBluetoothGattCallback(context, continuation)

      Log.i(TAG, "Connecting")
      bleGatt = device.connectGatt(context, false, bluetoothGattCallback)

      connectionId = ChipClient.getAndroidChipPlatform(context).bleManager.addConnection(bleGatt)
      ChipClient.getAndroidChipPlatform(context).bleManager.setBleCallback(this)

      continuation.invokeOnCancellation { bleGatt?.disconnect() }
    }
  }

  private fun getBluetoothGattCallback(
    context: Context,
    continuation: CancellableContinuation<BluetoothGatt?>
  ): BluetoothGattCallback {
    return object : BluetoothGattCallback() {
      private val wrappedCallback = ChipClient.getAndroidChipPlatform(context).bleManager.callback

      private val coroutineContinuation = continuation

      private val STATE_INIT = 1
      private val STATE_DISCOVER_SERVICE = 2
      private val STATE_REQUEST_MTU = 3

      private var mState = STATE_INIT

      override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
        super.onConnectionStateChange(gatt, status, newState)
        Log.i(
          TAG,
          "${gatt?.device?.name}.onConnectionStateChange status = $status, newState=$newState"
        )
        wrappedCallback.onConnectionStateChange(gatt, status, newState)

        if (newState == BluetoothProfile.STATE_CONNECTED && status == BluetoothGatt.GATT_SUCCESS) {
          Log.i("$TAG|onConnectionStateChange", "Discovering Services...")
          mState = STATE_DISCOVER_SERVICE
          gatt?.discoverServices()
        }
      }

      override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
        Log.d(TAG, "${gatt?.device?.name}.onServicesDiscovered status = $status")
        if (mState != STATE_DISCOVER_SERVICE) {
          Log.d(TAG, "Invalid state : $mState")
          return
        }
        wrappedCallback.onServicesDiscovered(gatt, status)

        Log.i("$TAG|onServicesDiscovered", "Services Discovered")
        mState = STATE_REQUEST_MTU
        gatt?.requestMtu(247)
      }

      override fun onMtuChanged(gatt: BluetoothGatt?, mtu: Int, status: Int) {
        Log.d(TAG, "${gatt?.device?.name}.onMtuChanged: connecting to CHIP device")
        super.onMtuChanged(gatt, mtu, status)
        if (mState != STATE_REQUEST_MTU) {
          Log.d(TAG, "Invalid state : $mState")
          return
        }
        wrappedCallback.onMtuChanged(gatt, mtu, status)
        if (coroutineContinuation.isActive) {
          coroutineContinuation.resume(gatt)
        }
      }

      override fun onCharacteristicChanged(
        gatt: BluetoothGatt,
        characteristic: BluetoothGattCharacteristic
      ) {
        Log.d(TAG, "${gatt.device.name}.onCharacteristicChanged: ${characteristic.uuid}")
        wrappedCallback.onCharacteristicChanged(gatt, characteristic)
      }

      override fun onCharacteristicRead(
        gatt: BluetoothGatt,
        characteristic: BluetoothGattCharacteristic,
        status: Int
      ) {
        Log.d(TAG, "${gatt.device.name}.onCharacteristicRead: ${characteristic.uuid} -> $status")
        wrappedCallback.onCharacteristicRead(gatt, characteristic, status)
      }

      override fun onCharacteristicWrite(
        gatt: BluetoothGatt,
        characteristic: BluetoothGattCharacteristic,
        status: Int
      ) {
        Log.d(TAG, "${gatt.device.name}.onCharacteristicWrite: ${characteristic.uuid} -> $status")
        wrappedCallback.onCharacteristicWrite(gatt, characteristic, status)
      }

      override fun onDescriptorRead(
        gatt: BluetoothGatt,
        descriptor: BluetoothGattDescriptor,
        status: Int
      ) {
        Log.d(TAG, "${gatt.device.name}.onDescriptorRead: ${descriptor.uuid} -> $status")
        wrappedCallback.onDescriptorRead(gatt, descriptor, status)
      }

      override fun onDescriptorWrite(
        gatt: BluetoothGatt,
        descriptor: BluetoothGattDescriptor,
        status: Int
      ) {
        Log.d(TAG, "${gatt.device.name}.onDescriptorWrite: ${descriptor.uuid} -> $status")
        wrappedCallback.onDescriptorWrite(gatt, descriptor, status)
      }

      override fun onReadRemoteRssi(gatt: BluetoothGatt, rssi: Int, status: Int) {
        Log.d(TAG, "${gatt.device.name}.onReadRemoteRssi: $rssi -> $status")
        wrappedCallback.onReadRemoteRssi(gatt, rssi, status)
      }

      override fun onReliableWriteCompleted(gatt: BluetoothGatt, status: Int) {
        Log.d(TAG, "${gatt.device.name}.onReliableWriteCompleted: $status")
        wrappedCallback.onReliableWriteCompleted(gatt, status)
      }
    }
  }

  companion object {
    private const val TAG = "chip.BluetoothManager"
    private const val CHIP_UUID = "0000FFF6-0000-1000-8000-00805F9B34FB"
  }

  override fun onCloseBleComplete(connId: Int) {
    connectionId = 0
    Log.d(TAG, "onCloseBleComplete")
  }

  override fun onNotifyChipConnectionClosed(connId: Int) {
    bleGatt?.close()
    connectionId = 0
    Log.d(TAG, "onNotifyChipConnectionClosed")
  }
}
