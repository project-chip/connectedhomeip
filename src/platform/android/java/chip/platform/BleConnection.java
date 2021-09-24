package chip.platform;

import android.bluetooth.BluetoothGatt;

public interface BleConnection {
  BluetoothGatt getBluetoothGatt();

  void onCloseBleComplete(int connId);

  void onNotifyChipConnectionClosed(int connId);
}
