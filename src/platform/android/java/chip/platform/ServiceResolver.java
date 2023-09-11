/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
