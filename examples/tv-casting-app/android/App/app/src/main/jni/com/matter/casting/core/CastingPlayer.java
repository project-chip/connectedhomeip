/*
 *   Copyright (c) 2023 Project CHIP Authors
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
 */
package com.matter.casting.core;

import java.net.InetAddress;
import java.util.List;

public interface CastingPlayer {
  boolean isConnected();

  String getDeviceId();

  String getHostName();

  String getDeviceName();

  String getInstanceName();

  int getNumberIPs();

  List<InetAddress> getIpAddresses();

  int getPort();

  int getVendorId();

  int getProductId();

  int getDeviceType();

  boolean discoveredCastingPlayerHasSameSource(Object o);

  // TODO: Implement in following PRs. Related to player connection implementation.
  //    List<Endpoint> getEndpoints();
  //
  //    ConnectionState getConnectionState();
  //
  //    CompletableFuture<Void> connect(long timeout);
  //
  //    static class ConnectionState extends Observable {
  //        private boolean connected;
  //
  //        void setConnected(boolean connected) {
  //            this.connected = connected;
  //            setChanged();
  //            notifyObservers(this.connected);
  //        }
  //
  //        boolean isConnected() {
  //            return connected;
  //        }
  //    }
}
