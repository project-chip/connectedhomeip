/*
 *   Copyright (c) 2021 Project CHIP Authors
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

/** Interface for resolving network services. */
public interface ServiceResolver {
  /**
   * Resolve an address for the given instance name and service type. The implementation of this
   * function should call {@link ChipMdnsCallback#handleServiceResolve} on chipMdnsCallback, passing
   * through the callbackHandle and contextHandle.
   */
  void resolve(
      String instanceName,
      String serviceType,
      long callbackHandle,
      long contextHandle,
      ChipMdnsCallback chipMdnsCallback);

  /**
   * Publishes a service via DNS-SD.
   *
   * <p>Calling the function again with the same service name, type, protocol, interface and port
   * but different text will update the text published.
   */
  void publish(
      String serviceName,
      String hostName,
      String type,
      int port,
      String[] textEntriesKeys,
      byte[][] textEntriesDatas,
      String[] subTypes);

  /** Removes or marks all services being advertised for removal. */
  void removeServices();
}
