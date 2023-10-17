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
package matter.controller

import kotlinx.coroutines.flow.Flow

interface InteractionClient {
  /**
   * Subscribes to periodic updates of all attributes and events from a device.
   *
   * @param request The Subscribe command's path and arguments.
   * @return A Flow of SubscriptionState representing the subscription's state updates.
   */
  fun subscribe(request: SubscribeRequest): Flow<SubscriptionState>

  /**
   * Issues a read request to a target device for specified attributes and events.
   *
   * @param request Read command's path and arguments.
   * @return A response to the read request.
   * @throws Generic Exception if an error occurs during the read operation.
   */
  suspend fun read(request: ReadRequest): ReadResponse

  /**
   * Issues attribute write requests to a target device.
   *
   * @param writeRequests A list of attribute WriteRequest.
   * @return A response to the write request.
   * @throws Generic Exception or MatterControllerException if an error occurs during the write
   *   operation.
   */
  suspend fun write(writeRequests: WriteRequests): WriteResponse

  /**
   * Invokes a command on a target device.
   *
   * @param request Invoke command's path and arguments.
   * @return A response to the invoke request.
   * @throws Generic Exception or MatterControllerException if an error occurs during the invoke
   *   operation.
   */
  suspend fun invoke(request: InvokeRequest): InvokeResponse
}
