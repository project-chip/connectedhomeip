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

/**
 * The CastingPlayer interface defines a Matter commissioner that is able to play media to a
 * physical output or to a display screen which is part of the device (e.g. TV). It is discovered on
 * the local network using Matter Commissioner discovery over DNS. It contains all the information
 * about the service discovered/resolved.
 */
public interface CastingPlayer {
  boolean isConnected();

  String getDeviceId();

  String getHostName();

  String getDeviceName();

  String getInstanceName();

  List<InetAddress> getIpAddresses();

  int getPort();

  int getVendorId();

  int getProductId();

  long getDeviceType();

  @Override
  String toString();

  @Override
  boolean equals(Object o);

  @Override
  int hashCode();

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
