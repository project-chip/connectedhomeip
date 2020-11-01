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
import chip.devicecontroller.AndroidChipStack
import chip.devicecontroller.ChipDeviceController
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

    /** Converts an integer to a ByteArray with the first [size] bytes of the [Int]. */
    private fun Int.toByteArray(size: Int): ByteArray {
        return ByteArray(size) { i -> (this shr (8 * i)).toByte() }
    }

    private fun getServiceData(discriminator: Int, vendorId: Int, productId: Int): ByteArray {
      var opcode = 0
      var version = 0
      var versionDiscriminator = ((version and 0xf) shl 12) or (discriminator and 0xfff)
      return intArrayOf(
         opcode,
         versionDiscriminator,
         versionDiscriminator shr 8,
         vendorId,
         vendorId shr 8,
         productId,
         productId shr 8
      ).map { it.toByte() }.toByteArray()
    }

    suspend fun getBluetoothDevice(discriminator: Int, vendorId: Int, productId: Int): BluetoothDevice? {
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


                var serviceDataList = listOf(
                   getServiceData(discriminator, vendorId, productId),

                   // TODO - Remove this incorrect format.
                   discriminator.toByteArray(3)
                )

                serviceDataList.forEach {
                  Log.v(TAG, "Matching service " + CHIP_UUID.substring(4, 8) + " " + it.map { String.format("%02x", it) } )
                }

                val scanFilters = serviceDataList.map { ScanFilter.Builder()
                    .setServiceData(
                      ParcelUuid(UUID.fromString(CHIP_UUID)),
                      it
                    )
                    .build()
                }

                val scanSettings = ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                    .build()

                Log.i(TAG, "Starting Bluetooth scan")
                scanner.startScan(scanFilters, scanSettings, scanCallback)
                awaitClose { scanner.stopScan(scanCallback) }
            }.first()
        }
    }

    /**
     * Connects to a [BluetoothDevice] and suspends until [BluetoothGattCallback.onServicesDiscovered]
     */
    suspend fun connect(context: Context, device: BluetoothDevice): BluetoothGatt? {
        return suspendCancellableCoroutine { continuation ->
            val bluetoothGattCallback = object : AndroidChipStack.ChipGattCallback() {
                override fun onConnectionStateChange(
                  gatt: BluetoothGatt?,
                  status: Int,
                  newState: Int
                ) {
                    super.onConnectionStateChange(gatt, status, newState)

                    if (newState == BluetoothProfile.STATE_CONNECTED && status == BluetoothGatt.GATT_SUCCESS) {
                        Log.i("$TAG|onConnectionStateChange", "Discovering Services...")
                        gatt?.discoverServices()
                    }
                }

                override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
                    super.onServicesDiscovered(gatt, status)

                    Log.i("$TAG|onServicesDiscovered", "Services Discovered")
                    if (continuation.isActive) {
                        continuation.resume(gatt)
                    }
                }
            }

            Log.i(TAG, "Connecting")
            val gatt = device.connectGatt(context, false, bluetoothGattCallback, BluetoothDevice.TRANSPORT_LE)
            continuation.invokeOnCancellation { gatt.disconnect() }
        }
    }

    companion object {
        private const val TAG = "chip.BluetoothManager"
        private const val CHIP_UUID = "0000FEAF-0000-1000-8000-00805F9B34FB"
    }
}
