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
 *
 */
package chip.devicecontroller;

@SuppressWarnings("serial")
public class ConnectionFailureException extends ChipDeviceControllerException {
  enum ConnectionState {
    UNINITIALIZED, // Error state: OperationalSessionSetup is useless
    NEEDS_ADDRESS, // No address known, lookup not started yet.
    RESOLVING_ADDRESS, // Address lookup in progress.
    HAS_ADDRESS, // Have an address, CASE handshake not started yet.
    CONNECTING, // CASE handshake in progress.
    SECURE_CONNECTED, // CASE session established.
    WAITING_FOR_RETRY, // No address known, but a retry is pending.
  }

  private ConnectionState connectionState;

  public ConnectionFailureException() {
    super();
  }

  public ConnectionFailureException(long errorCode, int connectionState, String message) {
    super(errorCode, message);
    this.connectionState = mapIntToConnectionState(connectionState);
  }

  public ConnectionState getConnectionState() {
    return connectionState;
  }

  // Helper method to map an int to ConnectionState enum
  private ConnectionState mapIntToConnectionState(int value) {
    switch (value) {
      case 0:
        return ConnectionState.UNINITIALIZED;
      case 1:
        return ConnectionState.NEEDS_ADDRESS;
      case 2:
        return ConnectionState.RESOLVING_ADDRESS;
      case 3:
        return ConnectionState.HAS_ADDRESS;
      case 4:
        return ConnectionState.CONNECTING;
      case 5:
        return ConnectionState.SECURE_CONNECTED;
      case 6:
        return ConnectionState.WAITING_FOR_RETRY;
      default:
        throw new IllegalArgumentException("Invalid connection state value: " + value);
    }
  }
}
