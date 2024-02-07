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
import java.net.InetAddress;
import java.util.List;
import java.util.concurrent.CompletableFuture;

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
   * @return A CompletableFuture that completes when the VerifyOrEstablishConnection is completed.
   *     The CompletableFuture will be completed with a Void value if the
   *     VerifyOrEstablishConnection is successful. Otherwise, the CompletableFuture will be
   *     completed with an Exception. The Exception will be of type
   *     com.matter.casting.core.CastingException. If the VerifyOrEstablishConnection fails, the
   *     CastingException will contain the error code and message from the CastingApp.
   */
  CompletableFuture<Void> VerifyOrEstablishConnection(
      long commissioningWindowTimeoutSec, EndpointFilter desiredEndpointFilter);

  /**
   * Verifies that a connection exists with this CastingPlayer, or triggers a new session request.
   * If the CastingApp does not have the nodeId and fabricIndex of this CastingPlayer cached on
   * disk, this will execute the user directed commissioning process.
   *
   * @return A CompletableFuture that completes when the VerifyOrEstablishConnection is completed.
   *     The CompletableFuture will be completed with a Void value if the
   *     VerifyOrEstablishConnection is successful. Otherwise, the CompletableFuture will be
   *     completed with an Exception. The Exception will be of type
   *     com.matter.casting.core.CastingException. If the VerifyOrEstablishConnection fails, the
   *     CastingException will contain the error code and message from the CastingApp.
   */
  CompletableFuture<Void> VerifyOrEstablishConnection();
}
