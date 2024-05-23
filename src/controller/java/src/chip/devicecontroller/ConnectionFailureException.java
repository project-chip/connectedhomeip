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
  public enum ConnectionState {
    UNKNOWN, // Unknown state
    NOT_IN_KEY_EXCHANGE, // Not currently in key exchange process
    SENT_SIGMA1, // Sigma1 message sent
    RECEIVED_SIGMA1, // Sigma1 message received
    SENT_SIGMA2, // Sigma2 message sent
    RECEIVED_SIGMA2, // Sigma2 message received
    SENT_SIGMA3, // Sigma3 message sent
    RECEIVED_SIGMA3 // Sigma3 message received
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
        return ConnectionState.UNKNOWN;
      case 1:
        return ConnectionState.NOT_IN_KEY_EXCHANGE;
      case 2:
        return ConnectionState.SENT_SIGMA1;
      case 3:
        return ConnectionState.RECEIVED_SIGMA1;
      case 4:
        return ConnectionState.SENT_SIGMA2;
      case 5:
        return ConnectionState.RECEIVED_SIGMA2;
      case 6:
        return ConnectionState.SENT_SIGMA3;
      case 7:
        return ConnectionState.RECEIVED_SIGMA3;
      default:
        throw new IllegalArgumentException("Invalid connection state value: " + value);
    }
  }
}
