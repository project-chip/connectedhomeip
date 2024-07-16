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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.util.Log;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

public class AndroidBleManager implements BleManager {

  private static final String TAG = AndroidBleManager.class.getSimpleName();
  public static final int INITIAL_CONNECTIONS = 4;

  private static class BleMtuDenylist {
    /**
     * Will be set at initialization to indicate whether the device on which this code is being run
     * is known to indicate unreliable MTU values for Bluetooth LE connections.
     */
    static final boolean BLE_MTU_DENYLISTED;

    /**
     * If {@link #BLE_MTU_DENYLISTED} is true, then this is the fallback MTU to use for this device
     */
    static final int BLE_MTU_FALLBACK = 23;

    static {
      if ("OnePlus".equals(Build.MANUFACTURER)) {
        BLE_MTU_DENYLISTED = "ONE A2005".equals(Build.MODEL) ? true : false;
      } else if ("motorola".equals(Build.MANUFACTURER)) {
        BLE_MTU_DENYLISTED =
            "XT1575".equals(Build.MODEL) || "XT1585".equals(Build.MODEL) ? true : false;
      } else {
        BLE_MTU_DENYLISTED = false;
      }
    }
  }

  private final List<BluetoothGatt> mConnections;
  private BleCallback mBleCallback;
  private BluetoothGattCallback mGattCallback;
  private AndroidChipPlatform mPlatform;

  private Context mContext;
  private BluetoothAdapter mBluetoothAdapter;

  private BleConnectCallback mBleConnectCallback;

  private BleConnectionHandler mConnectionHandler;
  private ScanCallback mScanCallback;

  private static final int MSG_BLE_SCAN = 0;
  private static final int MSG_BLE_CONNECT = 1;
  private static final int MSG_BLE_CONNECT_SUCCESS = 2;
  private static final int MSG_BLE_FAIL = 99;

  private static final int BLE_TIMEOUT_MS = 10000;
  private static final int BLUETOOTH_ENABLE_TIMEOUT_MS = 1000;

  private static final String MSG_BUNDLE_SERVICE_DATA = "serviceData";
  private static final String MSG_BUNDLE_SERVICE_DATA_MASK = "serviceDataMask";

  public AndroidBleManager(Context context) {
    this();
    mContext = context;
    @SuppressWarnings("unchecked")
    BluetoothManager manager =
        (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
    mBluetoothAdapter = manager.getAdapter();
    mConnectionHandler = new BleConnectionHandler(Looper.getMainLooper());
  }

  public AndroidBleManager() {
    mConnections = new ArrayList<>(INITIAL_CONNECTIONS);

    mGattCallback = new AndroidBluetoothGattCallback();
  }

  class AndroidBluetoothGattCallback extends BluetoothGattCallback {
    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
      int connId = 0;

      if (newState == BluetoothProfile.STATE_DISCONNECTED) {
        connId = getConnId(gatt);
        if (connId > 0) {
          Log.d(TAG, "onConnectionStateChange Disconnected");
          mPlatform.handleConnectionError(connId);
        } else {
          Log.e(TAG, "onConnectionStateChange disconnected: no active connection");
        }
      }
    }

    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status) {}

    @Override
    public void onCharacteristicRead(
        BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {}

    @Override
    public void onCharacteristicWrite(
        BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
      byte[] svcIdBytes = convertUUIDToBytes(characteristic.getService().getUuid());
      byte[] charIdBytes = convertUUIDToBytes(characteristic.getUuid());

      if (status != BluetoothGatt.GATT_SUCCESS) {
        Log.e(
            TAG,
            "onCharacteristicWrite for "
                + characteristic.getUuid().toString()
                + " failed with status: "
                + status);
        return;
      }

      int connId = getConnId(gatt);
      if (connId > 0) {
        mPlatform.handleWriteConfirmation(
            connId, svcIdBytes, charIdBytes, status == BluetoothGatt.GATT_SUCCESS);
      } else {
        Log.e(TAG, "onCharacteristicWrite no active connection");
        return;
      }
    }

    @Override
    public void onCharacteristicChanged(
        BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
      byte[] svcIdBytes = convertUUIDToBytes(characteristic.getService().getUuid());
      byte[] charIdBytes = convertUUIDToBytes(characteristic.getUuid());
      int connId = getConnId(gatt);
      if (connId > 0) {
        mPlatform.handleIndicationReceived(
            connId, svcIdBytes, charIdBytes, characteristic.getValue());
      } else {
        Log.e(TAG, "onCharacteristicChanged no active connection");
        return;
      }
    }

    @Override
    public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor desc, int status) {
      BluetoothGattCharacteristic characteristic = desc.getCharacteristic();

      byte[] svcIdBytes = convertUUIDToBytes(characteristic.getService().getUuid());
      byte[] charIdBytes = convertUUIDToBytes(characteristic.getUuid());

      if (status != BluetoothGatt.GATT_SUCCESS) {
        Log.e(
            TAG,
            "onDescriptorWrite for "
                + desc.getUuid().toString()
                + " failed with status: "
                + status);
      }

      int connId = getConnId(gatt);
      if (connId == 0) {
        Log.e(TAG, "onDescriptorWrite no active connection");
        return;
      }

      if (desc.getValue() == BluetoothGattDescriptor.ENABLE_INDICATION_VALUE) {
        mPlatform.handleSubscribeComplete(
            connId, svcIdBytes, charIdBytes, status == BluetoothGatt.GATT_SUCCESS);
      } else if (desc.getValue() == BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE) {
        mPlatform.handleSubscribeComplete(
            connId, svcIdBytes, charIdBytes, status == BluetoothGatt.GATT_SUCCESS);
      } else if (desc.getValue() == BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE) {
        mPlatform.handleUnsubscribeComplete(
            connId, svcIdBytes, charIdBytes, status == BluetoothGatt.GATT_SUCCESS);
      } else {
        Log.d(TAG, "Unexpected onDescriptorWrite().");
      }
    }

    @Override
    public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor desc, int status) {}
  }

  @Override
  public synchronized int addConnection(BluetoothGatt bleGatt) {
    int connIndex = 0;
    while (connIndex < mConnections.size()) {
      if (mConnections.get(connIndex) == null) {
        mConnections.set(connIndex, bleGatt);
        return connIndex + 1;
      }
      connIndex++;
    }
    mConnections.add(connIndex, bleGatt);
    return connIndex + 1;
  }

  @Override
  public synchronized BluetoothGatt removeConnection(int connId) {
    int connIndex = connId - 1;
    if (connIndex >= 0 && connIndex < mConnections.size()) {
      // Set to null, rather than remove, so that other indexes are unchanged.
      return mConnections.set(connIndex, null);
    } else {
      Log.e(TAG, "Trying to remove unknown connId " + connId);
      return null;
    }
  }

  @Override
  public synchronized BluetoothGatt getConnection(int connId) {
    int connIndex = connId - 1;
    if (connIndex >= 0 && connIndex < mConnections.size()) {
      return mConnections.get(connIndex);
    } else {
      Log.e(TAG, "Unknown connId " + connId);
      return null;
    }
  }

  @Override
  public void setBleCallback(BleCallback bleCallback) {
    mBleCallback = bleCallback;
  }

  @Override
  public BluetoothGattCallback getCallback() {
    return mGattCallback;
  }

  @Override
  public void setAndroidChipPlatform(AndroidChipPlatform platform) {
    mPlatform = platform;
  }

  private synchronized int getConnId(BluetoothGatt gatt) {
    // Find callback given gatt
    int connIndex = 0;
    while (connIndex < mConnections.size()) {
      BluetoothGatt inGatt = mConnections.get(connIndex);
      if (inGatt == gatt && gatt != null) {
        return connIndex + 1;
      }
      connIndex++;
    }
    return 0;
  }

  @Override
  public int init() {
    // TODO: add it when implementing _SetAdvertisingMode etc
    return 0;
  }

  @Override
  public long setFlag(long flag, boolean isSet) {
    // TODO: add it when implementing _SetAdvertisingMode etc
    return 0;
  }

  @Override
  public boolean hasFlag(long flag) {
    // TODO: add it when implementing _SetAdvertisingMode etc
    return false;
  }

  @Override
  public boolean onSubscribeCharacteristic(int connId, byte[] svcId, byte[] charId) {
    BluetoothGatt bluetoothGatt = getConnection(connId);
    if (bluetoothGatt == null) {
      Log.i(TAG, "Tried to send characteristic, but BLE connection was not found.");
      return false;
    }

    UUID svcUUID = convertBytesToUUID(svcId);
    BluetoothGattService subscribeSvc = bluetoothGatt.getService(svcUUID);
    if (subscribeSvc == null) {
      Log.e(TAG, "Bad service");
      return false;
    }

    UUID charUUID = convertBytesToUUID(charId);
    BluetoothGattCharacteristic subscribeChar = subscribeSvc.getCharacteristic(charUUID);
    if (subscribeChar == null) {
      Log.e(TAG, "Bad characteristic");
      return false;
    }

    if (!bluetoothGatt.setCharacteristicNotification(subscribeChar, true)) {
      Log.e(TAG, "Failed to subscribe to characteristic.");
      return false;
    }

    BluetoothGattDescriptor descriptor =
        subscribeChar.getDescriptor(UUID.fromString(CLIENT_CHARACTERISTIC_CONFIG));
    if ((subscribeChar.getProperties() & BluetoothGattCharacteristic.PROPERTY_INDICATE) != 0) {
      descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);
      if (!bluetoothGatt.writeDescriptor(descriptor)) {
        Log.e(TAG, "writeDescriptor failed");
        return false;
      }
    } else if ((subscribeChar.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0) {
      descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
      if (!bluetoothGatt.writeDescriptor(descriptor)) {
        Log.e(TAG, "writeDescriptor failed");
        return false;
      }
    }
    return true;
  }

  @Override
  public boolean onUnsubscribeCharacteristic(int connId, byte[] svcId, byte[] charId) {
    BluetoothGatt bluetoothGatt = getConnection(connId);
    if (bluetoothGatt == null) {
      Log.i(TAG, "Tried to unsubscribe characteristic, but BLE connection was not found.");
      return false;
    }

    UUID svcUUID = convertBytesToUUID(svcId);
    BluetoothGattService subscribeSvc = bluetoothGatt.getService(svcUUID);
    if (subscribeSvc == null) {
      Log.e(TAG, "Bad service");
      return false;
    }

    UUID charUUID = convertBytesToUUID(charId);
    BluetoothGattCharacteristic subscribeChar = subscribeSvc.getCharacteristic(charUUID);
    if (subscribeChar == null) {
      Log.e(TAG, "Bad characteristic");
      return false;
    }

    if (!bluetoothGatt.setCharacteristicNotification(subscribeChar, false)) {
      Log.e(TAG, "Failed to unsubscribe to characteristic.");
      return false;
    }

    BluetoothGattDescriptor descriptor =
        subscribeChar.getDescriptor(UUID.fromString(CLIENT_CHARACTERISTIC_CONFIG));
    descriptor.setValue(BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
    if (!bluetoothGatt.writeDescriptor(descriptor)) {
      Log.e(TAG, "writeDescriptor failed");
      return false;
    }
    return true;
  }

  @Override
  public boolean onCloseConnection(int connId) {
    BluetoothGatt bluetoothGatt = getConnection(connId);
    if (bluetoothGatt != null) {
      bluetoothGatt.close();
      removeConnection(connId);
      if (mBleCallback != null) {
        mBleCallback.onCloseBleComplete(connId);
      }
    } else {
      Log.i(TAG, "Tried to close BLE connection, but connection was not found.");
    }
    return true;
  }

  // onGetMTU returns the desired MTU for the BLE connection.
  // In most cases, a value of 0 is used to indicate no preference.
  // On some devices, we override to use the minimum MTU to work around device bugs.
  @Override
  public int onGetMTU(int connId) {
    int mtu = 0;
    Log.d(TAG, "Android Manufacturer: (" + Build.MANUFACTURER + ")");
    Log.d(TAG, "Android Model: (" + Build.MODEL + ")");

    if (BleMtuDenylist.BLE_MTU_DENYLISTED) {
      mtu = BleMtuDenylist.BLE_MTU_FALLBACK;
      Log.e(TAG, "Detected Manufacturer/Model with MTU incompatibiility. Reporting MTU: " + mtu);
    }
    return mtu;
  }

  @Override
  public boolean onSendWriteRequest(
      int connId, byte[] svcId, byte[] charId, byte[] characteristicData) {
    BluetoothGatt bluetoothGatt = getConnection(connId);
    if (bluetoothGatt == null) {
      Log.i(TAG, "Tried to send characteristic, but BLE connection was not found.");
      return false;
    }

    UUID svcUUID = convertBytesToUUID(svcId);
    BluetoothGattService sendSvc = bluetoothGatt.getService(svcUUID);
    if (sendSvc == null) {
      Log.e(TAG, "Bad service");
      return false;
    }

    UUID charUUID = convertBytesToUUID(charId);
    BluetoothGattCharacteristic sendChar = sendSvc.getCharacteristic(charUUID);
    if (!sendChar.setValue(characteristicData)) {
      Log.e(TAG, "Failed to set characteristic");
      return false;
    }

    // Request acknowledgement (use ATT Write Request).
    sendChar.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);

    if (!bluetoothGatt.writeCharacteristic(sendChar)) {
      Log.e(TAG, "Failed writing char");
      return false;
    }
    return true;
  }

  @Override
  public void onNotifyChipConnectionClosed(int connId) {
    BluetoothGatt gatt = getConnection(connId);
    if (gatt != null) {
      removeConnection(connId);
      if (mBleCallback != null) {
        mBleCallback.onNotifyChipConnectionClosed(connId);
      }
    } else {
      Log.i(TAG, "Tried to notify connection closed, but BLE connection was not found.");
    }
  }

  @Override
  public void onNewConnection(
      int discriminator, boolean isShortDiscriminator, long implPtr, long appStatePtr) {
    Log.d(TAG, "onNewConnection : " + discriminator + ", " + isShortDiscriminator);
    if (mContext == null) {
      return;
    }
    mBleConnectCallback = new BleConnectCallback(implPtr, appStatePtr);
    mConnectionHandler.sendEmptyMessageDelayed(MSG_BLE_FAIL, BLE_TIMEOUT_MS);
    Message msg = mConnectionHandler.obtainMessage();
    msg.what = MSG_BLE_SCAN;
    Bundle bundle = new Bundle();
    byte[] serviceData = getServiceData(discriminator);
    byte[] serviceDataMask = getServiceDataMask(isShortDiscriminator);
    bundle.putByteArray(MSG_BUNDLE_SERVICE_DATA, serviceData);
    bundle.putByteArray(MSG_BUNDLE_SERVICE_DATA_MASK, serviceDataMask);
    msg.setData(bundle);
    if (!mBluetoothAdapter.isEnabled()) {
      mBluetoothAdapter.enable();
      // TODO: Check Bluetooth enable intent
      mConnectionHandler.sendMessageDelayed(msg, BLUETOOTH_ENABLE_TIMEOUT_MS);
      return;
    }
    mConnectionHandler.sendMessage(msg);
    return;
  }

  class BleConnectionHandler extends Handler {
    public BleConnectionHandler(Looper looper) {
      super(looper);
    }

    @Override
    public void handleMessage(Message msg) {
      super.handleMessage(msg);

      switch (msg.what) {
        case MSG_BLE_SCAN:
          startBleScan(msg.getData());
          break;
        case MSG_BLE_CONNECT:
          stopBleScan();
          connectBLE(msg.obj);
          break;
        case MSG_BLE_CONNECT_SUCCESS:
          bleConnectSuccess(msg.obj);
          break;
        case MSG_BLE_FAIL:
        default:
          stopBleScan();
          bleConnectFail();
          break;
      }
    }
  }

  private void startBleScan(Bundle bundle) {
    BluetoothLeScanner scanner = mBluetoothAdapter.getBluetoothLeScanner();
    if (scanner == null) {
      Log.d(TAG, "No bluetooth scanner found");
      return;
    }

    mScanCallback =
        new ScanCallback() {
          @Override
          public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            Log.i(
                TAG,
                "Bluetooth Device Scanned Addr: " + device.getAddress() + ", " + device.getName());
            Message msg = mConnectionHandler.obtainMessage();
            msg.what = MSG_BLE_CONNECT;
            msg.obj = (Object) device;
            mConnectionHandler.sendMessage(msg);
          }

          @Override
          public void onScanFailed(int errorCode) {
            Log.e(TAG, "Scan failed " + errorCode);
          }
        };

    byte[] serviceData = bundle.getByteArray(MSG_BUNDLE_SERVICE_DATA);
    byte[] serviceDataMask = bundle.getByteArray(MSG_BUNDLE_SERVICE_DATA_MASK);
    ScanFilter scanFilter =
        new ScanFilter.Builder()
            .setServiceData(
                new ParcelUuid(UUID.fromString(CHIP_UUID)), serviceData, serviceDataMask)
            .build();
    ScanSettings scanSettings =
        new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();
    Log.i(TAG, "Starting Bluetooth scan");
    scanner.startScan(Arrays.asList(scanFilter), scanSettings, mScanCallback);
  }

  private void stopBleScan() {
    if (mScanCallback != null) {
      BluetoothLeScanner scanner = mBluetoothAdapter.getBluetoothLeScanner();
      if (scanner == null) {
        Log.d(TAG, "No bluetooth scanner found");
        return;
      }
      scanner.stopScan(mScanCallback);
      mScanCallback = null;
    }
  }

  private void connectBLE(Object bluetoothDeviceObj) {
    if (bluetoothDeviceObj == null) {
      return;
    }

    // Fail Timer reset.
    mConnectionHandler.removeMessages(MSG_BLE_FAIL);
    mConnectionHandler.sendEmptyMessageDelayed(MSG_BLE_FAIL, BLE_TIMEOUT_MS);

    @SuppressWarnings("unchecked")
    BluetoothDevice device = (BluetoothDevice) bluetoothDeviceObj;

    Log.i(TAG, "Connecting");
    BluetoothGatt gatt = device.connectGatt(mContext, false, new ConnectionGattCallback());
  }

  class ConnectionGattCallback extends AndroidBluetoothGattCallback {
    private static final int STATE_INIT = 1;
    private static final int STATE_DISCOVER_SERVICE = 2;
    private static final int STATE_REQUEST_MTU = 3;

    private int mState = STATE_INIT;

    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
      Log.i(TAG, "onConnectionStateChange status = " + status + ", newState= + " + newState);
      super.onConnectionStateChange(gatt, status, newState);
      if (newState == BluetoothProfile.STATE_CONNECTED && status == BluetoothGatt.GATT_SUCCESS) {
        Log.i(TAG, "Discovering Services...");
        mState = STATE_DISCOVER_SERVICE;
        gatt.discoverServices();
        return;
      } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
        Log.i(TAG, "Services Disconnected");
      }
      mConnectionHandler.sendEmptyMessage(MSG_BLE_FAIL);
    }

    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status) {
      Log.d(TAG, "onServicesDiscovered status = " + status);
      super.onServicesDiscovered(gatt, status);
      if (mState != STATE_DISCOVER_SERVICE) {
        Log.d(TAG, "Invalid state : " + mState);
        return;
      }

      Log.i(TAG, "Services Discovered");
      mState = STATE_REQUEST_MTU;
      gatt.requestMtu(247);
    }

    @Override
    public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
      super.onMtuChanged(gatt, mtu, status);
      if (mState != STATE_REQUEST_MTU) {
        Log.d(TAG, "Invalid state : " + mState);
        return;
      }
      String deviceName = "";
      if (gatt != null && gatt.getDevice() != null) {
        deviceName = gatt.getDevice().getName();
      }
      Log.d(TAG, deviceName + ".onMtuChanged: connecting to CHIP device : " + status);

      Message msg = mConnectionHandler.obtainMessage();
      msg.what = MSG_BLE_CONNECT_SUCCESS;
      msg.obj = (Object) gatt;

      mConnectionHandler.sendMessage(msg);
    }
  }

  private void bleConnectSuccess(Object gattObj) {
    Log.d(TAG, "bleConnectSuccess");
    mConnectionHandler.removeMessages(MSG_BLE_FAIL);
    @SuppressWarnings("unchecked")
    BluetoothGatt gatt = (BluetoothGatt) gattObj;
    int connId = addConnection(gatt);

    setBleCallback(
        new BleCallback() {
          @Override
          public void onCloseBleComplete(int connId) {
            Log.d(TAG, "onCloseBleComplete : " + connId);
          }

          @Override
          public void onNotifyChipConnectionClosed(int connId) {
            Log.d(TAG, "onNotifyChipConnectionClosed : " + connId);
          }
        });

    if (mBleConnectCallback != null) {
      mBleConnectCallback.onConnectSuccess(connId);
    } else {
      // Already fail returned
      Log.d(TAG, "Already timeout");
      gatt.disconnect();
      removeConnection(connId);
    }
    mBleConnectCallback = null;
  }

  private void bleConnectFail() {
    Log.d(TAG, "bleConnectFail");
    if (mBleConnectCallback != null) {
      mBleConnectCallback.onConnectFailed();
    }
    mBleConnectCallback = null;
  }

  private byte[] getServiceData(int discriminator) {
    int opcode = 0;
    int version = 0;
    int versionDiscriminator = ((version & 0xf) << 12) | (discriminator & 0xfff);
    return new byte[] {
      (byte) (opcode & 0xFF),
      (byte) (versionDiscriminator & 0xFF),
      (byte) ((versionDiscriminator >> 8) & 0xFF)
    };
  }

  private byte[] getServiceDataMask(boolean isShortDiscriminator) {
    return new byte[] {(byte) 0xFF, (byte) (isShortDiscriminator ? 0x00 : 0xFF), (byte) 0xFF};
  }

  // CLIENT_CHARACTERISTIC_CONFIG is the well-known UUID of the client characteristic descriptor
  // that has the flags for enabling and disabling notifications and indications.
  // c.f. https://www.bluetooth.org/en-us/specification/assigned-numbers/generic-attribute-profile
  private static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";
  private static String CHIP_UUID = "0000FFF6-0000-1000-8000-00805F9B34FB";

  private static byte[] convertUUIDToBytes(UUID uuid) {
    byte[] idBytes = new byte[16];
    long idBits;
    idBits = uuid.getLeastSignificantBits();

    for (int i = 0; i < 8; i++) {
      idBytes[15 - i] = (byte) (idBits & 0xff);
      idBits = idBits >> 8;
    }

    idBits = uuid.getMostSignificantBits();
    for (int i = 0; i < 8; i++) {
      idBytes[7 - i] = (byte) (idBits & 0xff);
      idBits = idBits >> 8;
    }

    return idBytes;
  }

  private static UUID convertBytesToUUID(byte[] id) {
    long mostSigBits = 0;
    long leastSigBits = 0;

    if (id.length == 16) {
      for (int i = 0; i < 8; i++) {
        mostSigBits = (mostSigBits << 8) | (0xff & id[i]);
      }

      for (int i = 0; i < 8; i++) {
        leastSigBits = (leastSigBits << 8) | (0xff & id[i + 8]);
      }
    }

    return new UUID(mostSigBits, leastSigBits);
  }
}
