/*
 *   Copyright (c) 2026 Project CHIP Authors
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
import android.net.MacAddress;
import android.net.wifi.usd.SubscribeConfig;
import android.net.wifi.usd.UsdManager;
import android.net.wifi.usd.UsdSubscribeSession;
import android.net.wifi.usd.UsdSubscribeSessionCallback;
import android.util.Log;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

public class AndroidWifiPafManager implements WifiPafManager {
  private static final String TAG = AndroidWifiPafManager.class.getSimpleName();
  private static final String MATTER_SERVICE_NAME = "_matterc._udp";
  private static final int MATTER_PROTOCOL_TYPE = 3; // CSA Matter

  private final Context context;
  private UsdManager usdManager;
  private AndroidChipPlatform platform;
  private final Executor executor = Executors.newSingleThreadExecutor();

  private int nextSubscribeId = 1;
  private final Map<Integer, UsdSessionWrapper> activeSessions = new HashMap<>();
  private int nextMessageId = 1;

  public AndroidWifiPafManager(Context context) {
    this.context = context;
  }

  @Override
  public int init() {
    // UsdManager is a system service, we use the string constant directly
    usdManager = (UsdManager) context.getSystemService("wifi_usd");
    if (usdManager == null) {
      Log.e(TAG, "Failed to get WIFI_USD_SERVICE (wifi_usd)");
      return -1;
    }
    return 0;
  }

  @Override
  public int wifiPafSubscribe(int discriminator, int freq) {
    Log.d(TAG, "wifiPafSubscribe: discriminator=" + discriminator + ", freq=" + freq);

    byte[] ssi = getSubscribeSsi(discriminator);
    SubscribeConfig config =
        new SubscribeConfig.Builder()
            .setServiceName(MATTER_SERVICE_NAME)
            .setServiceProtocolType(MATTER_PROTOCOL_TYPE)
            .setServiceSpecificInfo(ssi)
            .build();

    final int subscribeId = nextSubscribeId++;
    UsdSessionWrapper wrapper = new UsdSessionWrapper(subscribeId);
    synchronized (activeSessions) {
      activeSessions.put(subscribeId, wrapper);
    }

    try {
      usdManager.subscribe(config, executor, wrapper.getCallback());
    } catch (Exception e) {
      Log.e(TAG, "Failed to subscribe via UsdManager", e);
      synchronized (activeSessions) {
        activeSessions.remove(subscribeId);
      }
      return -1;
    }

    return subscribeId;
  }

  @Override
  public int wifiPafCancelSubscribe(int subscribeId) {
    Log.d(TAG, "wifiPafCancelSubscribe: " + subscribeId);
    UsdSessionWrapper wrapper;
    synchronized (activeSessions) {
      wrapper = activeSessions.remove(subscribeId);
    }

    if (wrapper != null) {
      wrapper.close();
    }
    return 0;
  }

  @Override
  public boolean wifiPafSend(int id, int peerId, String peerAddr, byte[] data) {
    Log.d(TAG, "wifiPafSend to " + peerAddr + ", len=" + data.length);
    UsdSessionWrapper wrapper;
    synchronized (activeSessions) {
      wrapper = activeSessions.get(id);
    }

    if (wrapper == null) {
      Log.e(TAG, "No active USD session for ID: " + id);
      return false;
    }

    return wrapper.sendMessage(peerId, peerAddr, data);
  }

  @Override
  public void setAndroidChipPlatform(AndroidChipPlatform platform) {
    this.platform = platform;
  }

  private byte[] getSubscribeSsi(int discriminator) {
    ByteBuffer buf = ByteBuffer.allocate(7);
    buf.order(ByteOrder.LITTLE_ENDIAN);
    buf.put((byte) 0); // DevOpCode
    buf.putShort((short) discriminator); // DevInfo
    buf.putShort((short) 0); // ProductId
    buf.putShort((short) 0); // VendorId
    return buf.array();
  }

  private class UsdSessionWrapper {
    private final int subscribeId;
    private UsdSubscribeSession session;
    private final MySessionCallback callback;
    private int peerId = 0;
    private String peerAddr = null;

    UsdSessionWrapper(int subscribeId) {
      this.subscribeId = subscribeId;
      this.callback = new MySessionCallback();
    }

    UsdSubscribeSessionCallback getCallback() {
      return callback;
    }

    void close() {
      if (session != null) {
        session.close();
        session = null;
      }
    }

    boolean sendMessage(int peerId, String peerAddr, byte[] data) {
      if (session == null) {
        Log.e(TAG, "Session not started yet for ID: " + subscribeId);
        return false;
      }
      this.peerId = peerId;
      this.peerAddr = peerAddr;
      int messageId = nextMessageId++;
      try {
        session.sendMessage(MacAddress.fromString(peerAddr), messageId, data);
        return true;
      } catch (Exception e) {
        Log.e(TAG, "Failed to send USD message", e);
        return false;
      }
    }

    private class MySessionCallback implements UsdSubscribeSessionCallback {
      @Override
      public void onSessionStarted(UsdSubscribeSession usdSession) {
        Log.d(TAG, "USD session started for ID: " + subscribeId);
        session = usdSession;
      }

      @Override
      public void onSessionFailed(int reason) {
        Log.e(TAG, "USD session failed for ID: " + subscribeId + ", reason: " + reason);
        synchronized (activeSessions) {
          activeSessions.remove(subscribeId);
        }
        if (platform != null) {
          platform.handleSubscribeTerminated(subscribeId, "Session failed: " + reason);
        }
      }

      @Override
      public void onServiceDiscovered(MacAddress peerMac, byte[] ssi) {
        Log.d(TAG, "USD service discovered from: " + peerMac);
        if (ssi == null || ssi.length < 7) {
          Log.w(TAG, "Received USD response with invalid SSI length");
          return;
        }

        ByteBuffer buf = ByteBuffer.wrap(ssi);
        buf.order(ByteOrder.LITTLE_ENDIAN);
        byte opCode = buf.get();
        int discriminator = buf.getShort() & 0xFFFF;
        int productId = buf.getShort() & 0xFFFF;
        int vendorId = buf.getShort() & 0xFFFF;

        // Store peer info for sending messages later.
        peerAddr = peerMac.toString();
        // peerId is not directly available, we use 0 or extract if possible.
        peerId = 0; 

        if (platform != null) {
          platform.handleDiscoveryResult(
              subscribeId,
              peerId,
              peerAddr,
              discriminator,
              productId,
              vendorId);
        }
      }

      @Override
      public void onMessageReceived(MacAddress peerMac, byte[] message) {
        Log.d(TAG, "USD message received from: " + peerMac + ", len=" + message.length);
        if (platform != null) {
          platform.handleReceive(subscribeId, peerId, peerMac.toString(), message);
        }
      }

      @Override
      public void onMessageSendSucceeded(int messageId) {
        Log.d(TAG, "USD message send succeeded: " + messageId);
        if (platform != null && peerAddr != null) {
          platform.handleSendWriteDone(subscribeId, peerId, peerAddr, true);
        }
      }

      @Override
      public void onMessageSendFailed(int messageId, int reason) {
        Log.e(TAG, "USD message send failed: " + messageId + ", reason: " + reason);
        if (platform != null && peerAddr != null) {
          platform.handleSendWriteDone(subscribeId, peerId, peerAddr, false);
        }
      }
    }
  }
}
