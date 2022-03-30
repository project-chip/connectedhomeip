/*
 *   Copyright (c) 2022 Project CHIP Authors
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

import android.content.Context;
import android.util.Log;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;

public class DiagnosticDataProviderImpl implements DiagnosticDataProvider {
  private static final String TAG = DiagnosticDataProviderImpl.class.getSimpleName();
  private Context mContext;

  public DiagnosticDataProviderImpl(Context context) {
    mContext = context;
  }

  @Override
  public int getRebootCount() {
    // TODO: boot count is not supported until api 24, please do it at factory app
    //        try {
    //            return Settings.Global.getInt(mContext.getContentResolver(),
    // Settings.Global.BOOT_COUNT);
    //        } catch (Settings.SettingNotFoundException e) {
    //            Log.e(TAG, "getRebootCount: " + e.getMessage());
    //        }
    return 100;
  }

  @Override
  public NetworkInterface[] getNetworkInterfaces() {
    try {
      ArrayList<java.net.NetworkInterface> networkInterfaces =
          Collections.list(java.net.NetworkInterface.getNetworkInterfaces());
      int size = networkInterfaces.size();
      List<NetworkInterface> destInterfaces = new ArrayList<>();
      for (int i = 0; i < size; i++) {
        java.net.NetworkInterface nif = networkInterfaces.get(i);
        String name = nif.getName();

        if (name != null && (name.startsWith("eth") || name.startsWith("wlan"))) {
          NetworkInterface anInterface = new NetworkInterface();
          anInterface.isOperational = nif.isUp();
          anInterface.hardwareAddress = nif.getHardwareAddress();
          anInterface.name = nif.getName();
          anInterface.type =
              name.startsWith("wlan")
                  ? NetworkInterface.INTERFACE_TYPE_WI_FI
                  : NetworkInterface.INTERFACE_TYPE_ETHERNET;

          Enumeration<InetAddress> inetAddress = nif.getInetAddresses();
          while (inetAddress.hasMoreElements()) {
            InetAddress ip = inetAddress.nextElement();

            if (ip instanceof Inet4Address) {
              if (anInterface.ipv4Address == null) {
                anInterface.ipv4Address = ip.getAddress();
              }
            } else if (ip instanceof InetAddress) {
              if (anInterface.ipv6Address == null) {
                anInterface.ipv6Address = ip.getAddress();
              }
            }
          }

          destInterfaces.add(anInterface);
        }
      }

      NetworkInterface[] inetArray = new NetworkInterface[destInterfaces.size()];
      destInterfaces.toArray(inetArray);
      return inetArray;
    } catch (SocketException e) {
      Log.e(TAG, "getNetworkInterfaces: " + e.getMessage());
    }

    return new NetworkInterface[0];
  }
}
