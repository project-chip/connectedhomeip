/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.appserver;

import java.util.List;

/** read fabric count and list */
public class ChipFabricProvider {
  public native int getFabricCount();

  public native List<Fabric> getFabricList();

  // todo support to remove fabric
}
