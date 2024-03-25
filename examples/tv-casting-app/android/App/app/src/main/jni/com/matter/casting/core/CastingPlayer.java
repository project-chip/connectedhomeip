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

import com.matter.casting.support.EndpointFilter;
import com.matter.casting.support.MatterCallback;
import com.matter.casting.support.MatterError;
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

  List<Endpoint> getEndpoints();

  @Override
  String toString();

  @Override
  boolean equals(Object o);

  @Override
  int hashCode();

  /**
   * Verifies that a connection exists with this CastingPlayer, or triggers a new session request.
   * If the CastingApp does not have the nodeId and fabricIndex of this CastingPlayer cached on
   * disk, this will execute the user directed commissioning process.
   *
   * @param commissioningWindowTimeoutSec (Optional) time (in sec) to keep the commissioning window
   *     open, if commissioning is required. Needs to be >= MIN_CONNECTION_TIMEOUT_SEC.
   * @param desiredEndpointFilter (Optional) Attributes (such as VendorId) describing an Endpoint
   *     that the client wants to interact with after commissioning. If this value is passed in, the
   *     VerifyOrEstablishConnection will force User Directed Commissioning, in case the desired
   *     Endpoint is not found in the on device CastingStore.
   * @param successCallback called when the connection is established successfully
   * @param failureCallback called with MatterError when the connection is fails to establish
   * @return MatterError - Matter.NO_ERROR if request submitted successfully, otherwise a
   *     MatterError object corresponding to the error
   */
  MatterError verifyOrEstablishConnection(
      long commissioningWindowTimeoutSec,
      EndpointFilter desiredEndpointFilter,
      MatterCallback<Void> successCallback,
      MatterCallback<MatterError> failureCallback);

  /**
   * Verifies that a connection exists with this CastingPlayer, or triggers a new session request.
   * If the CastingApp does not have the nodeId and fabricIndex of this CastingPlayer cached on
   * disk, this will execute the user directed commissioning process.
   *
   * @param successCallback called when the connection is established successfully
   * @param failureCallback called with MatterError when the connection is fails to establish
   * @return MatterError - Matter.NO_ERROR if request submitted successfully, otherwise a
   *     MatterError object corresponding to the error
   */
  MatterError verifyOrEstablishConnection(
      MatterCallback<Void> successCallback, MatterCallback<MatterError> failureCallback);

  /** @brief Sets the internal connection state of this CastingPlayer to "disconnected" */
  void disconnect();
}
