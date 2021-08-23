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
import com.google.chip.chiptool.ChipClient
import kotlinx.coroutines.CancellableContinuation
import java.util.UUID
import kotlin.coroutines.resume
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withTimeoutOrNull

@ExperimentalCoroutinesApi
class BluetoothManager {
  private val bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()

  private fun getServiceData(discriminator: Int): ByteArray {
    val opcode = 0
    val version = 0
    val versionDiscriminator = ((version and 0xf) shl 12) or (discriminator and 0xfff)
    return intArrayOf(opcode, versionDiscriminator, versionDiscriminator shr 8)
        .map { it.toByte() }
        .toByteArray()
  }

  suspend fun getBluetoothDevice(discriminator: Int): BluetoothDevice? {

    if (! bluetoothAdapter.isEnabled) {
      bluetoothAdapter.enable();
    }

    val scanner = bluetoothAdapter.bluetoothLeScanner ?: run {
      Log.e(TAG, "No bluetooth scanner found")
      return null
    }

    return withTimeoutOrNull(10000) {
      callbackFlow {
        val scanCallback = object : ScanCallback() {
          override fun onScanResult(callbackType: Int, result: ScanResult) {
            val device = result.device
            Log.i(TAG, "Bluetooth Device Scanned Addr: ${device.address}, Name ${device.name}")

            offer(device)
          }

          override fun onScanFailed(errorCode: Int) {
            Log.e(TAG, "Scan failed $errorCode")
          }
        }

        val serviceData = getServiceData(discriminator)
        val scanFilter =
            ScanFilter.Builder()
                .setServiceData(ParcelUuid(UUID.fromString(CHIP_UUID)), serviceData)
                .build()

        val scanSettings = ScanSettings.Builder()
            .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
            .build()

        Log.i(TAG, "Starting Bluetooth scan")
        scanner.startScan(listOf(scanFilter), scanSettings, scanCallback)
        awaitClose { scanner.stopScan(scanCallback) }
      }.first()
    }
  }

  /**
   * Connects to a [BluetoothDevice] and suspends until [BluetoothGattCallback.onServicesDiscovered]
   */
  suspend fun connect(context: Context, device: BluetoothDevice): BluetoothGatt? {
    return suspendCancellableCoroutine { continuation ->
      val bluetoothGattCallback = getBluetoothGattCallback(context, continuation)

      Log.i(TAG, "Connecting")
      val gatt = device.connectGatt(context, false, bluetoothGattCallback)
      continuation.invokeOnCancellation { gatt.disconnect() }
    }
  }

  private fun getBluetoothGattCallback(
    context: Context,
    continuation: CancellableContinuation<BluetoothGatt?>
  ): BluetoothGattCallback {
    return object : BluetoothGattCallback() {
      private val wrappedCallback = ChipClient.getDeviceController(context).callback
      private val coroutineContinuation = continuation

      override fun onConnectionStateChange(
          gatt: BluetoothGatt?,
          status: Int,
          newState: Int
      ) {
        super.onConnectionStateChange(gatt, status, newState)
        wrappedCallback.onConnectionStateChange(gatt, status, newState)

        if (newState == BluetoothProfile.STATE_CONNECTED && status == BluetoothGatt.GATT_SUCCESS) {
          Log.i("$TAG|onConnectionStateChange", "Discovering Services...")
          gatt?.discoverServices()
        }
      }

      override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
        wrappedCallback.onServicesDiscovered(gatt, status)

        Log.i("$TAG|onServicesDiscovered", "Services Discovered")
        gatt?.requestMtu(131);
      }

      override fun onMtuChanged(gatt: BluetoothGatt?, mtu: Int, status: Int) {
        super.onMtuChanged(gatt, mtu, status)
        wrappedCallback.onMtuChanged(gatt, mtu, status)

        Log.d(TAG, "MTU changed: connecting to CHIP device")
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
}
